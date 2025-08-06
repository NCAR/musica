import pytest
import musica
from musica.constants import GAS_CONSTANT
import musica.mechanism_configuration as mc
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
pd.set_option('display.float_format', str)
np.set_printoptions(suppress=True)

MOLEC_CM3_TO_MOLE_M3 = 1.0e6 / 6.022e23  # Convert from molecules/cm³ to moles/m³
NUMBER_OF_GRID_CELLS = 120  # Number of grid cells for the simulation
NUMBER_OF_AEROSOL_SECTIONS = 38  # Number of aerosol sections for the simulation
DENSITY_SULFATE = 1923.0  # Density of sulfate in kg/m³
MOLECULAR_MASS_H2O = 0.01801528  # kg/mol
MOLECULAR_MASS_H2SO4 = 0.098078479  # kg/mol
MOLECULAR_MASS_AIR = 0.029  # kg/mol (average molecular mass of air)


def create_mechanism():
    """
    Creates a chemical mechanism for the gas-phase reactions involved in sulfate formation.
    """
    HO2 = mc.Species(name="HO2")
    H2O2 = mc.Species(name="H2O2")
    OH = mc.Species(name="OH", constant_mixing_ratio_mol_mol=0.8e-12)
    SO2 = mc.Species(name="SO2")
    SO3 = mc.Species(name="SO3")
    H2SO4 = mc.Species(name="H2SO4")
    H2O = mc.Species(name="H2O")
    M = mc.Species(name="M", is_third_body=True)

    gas = mc.Phase(name="gas", species=[HO2, H2O2, OH, SO2, SO3, H2SO4, H2O, M])

    rxn_2HO2_H2O2 = mc.Arrhenius(
        name="2HO2 -> H2O2",
        reactants=[HO2, HO2],
        products=[H2O2],
        gas_phase=gas,
        A=3.0e-13/MOLEC_CM3_TO_MOLE_M3,
        Ea=-6.35099e-21)

    rxn_2HO2_M_H2O2 = mc.Arrhenius(
        name="2HO2 + M -> H2O2",
        reactants=[HO2, HO2, M],
        products=[H2O2],
        gas_phase=gas,
        A=2.1e-33/(MOLEC_CM3_TO_MOLE_M3**2),
        Ea=-1.2702e-20)

    rxn_2HO2_H2O_H2O2 = mc.Arrhenius(
        name="2H2O2 + H2O -> H2O2 + H2O",
        reactants=[HO2, HO2, H2O],
        products=[H2O2, H2O],
        gas_phase=gas,
        A=4.2e-34/(MOLEC_CM3_TO_MOLE_M3**2),
        Ea=-3.67253e-20)

    rxn_2HO2_H2O_M_H2O2 = mc.Arrhenius(
        name="2HO2 + H2O + M -> H2O2 + H2O",
        reactants=[HO2, HO2, H2O, M],
        products=[H2O2, H2O],
        gas_phase=gas,
        A=2.94e-54/(MOLEC_CM3_TO_MOLE_M3**3),
        Ea=-4.30762e-20)

    rxn_H2O2_OH_HO2_H2O = mc.Arrhenius(
        name="H2O2 + OH -> HO2 + H2O",
        reactants=[H2O2, OH],
        products=[HO2, H2O],
        gas_phase=gas,
        A=1.8e-12/MOLEC_CM3_TO_MOLE_M3)

    rxn_SO2_OH_SO3_HO2 = mc.Troe(
        name="SO2 + OH -> SO3 + HO2",
        reactants=[SO2, OH],
        products=[SO3, HO2],
        gas_phase=gas,
        k0_A=2.9e-31/MOLEC_CM3_TO_MOLE_M3,
        k0_B=-4.1,
        kinf_A=1.7e-12/MOLEC_CM3_TO_MOLE_M3,
        kinf_B=-0.2)

    rxn_SO3_2H2O_H2SO4_H2O = mc.Arrhenius(
        name="SO3 + 2H2O -> H2SO4 + H2O",
        reactants=[SO3, H2O, H2O],
        products=[H2SO4, H2O],
        gas_phase=gas,
        A=8.5e-41/(MOLEC_CM3_TO_MOLE_M3**2),
        C=6540.0)

    return mc.Mechanism(
        name="Sulfate Box Model",
        phases=[gas],
        reactions=[
            rxn_2HO2_H2O2,
            rxn_2HO2_M_H2O2,
            rxn_2HO2_H2O_H2O2,
            rxn_2HO2_H2O_M_H2O2,
            rxn_H2O2_OH_HO2_H2O,
            rxn_SO2_OH_SO3_HO2,
            rxn_SO3_2H2O_H2SO4_H2O
        ],
        species=[HO2, H2O2, OH, SO2, SO3, H2SO4, H2O, M]
    )


def create_carma_solver():
    """
    Creates a CARMA solver with the sulfate box model configuration.
    """
    params = musica.CARMAParameters()
    params.nz = NUMBER_OF_GRID_CELLS
    params.zmin = 0.0
    params.delta_z = 100.0  # Vertical grid spacing in meters
    params.nbin = NUMBER_OF_AEROSOL_SECTIONS

    # Set up a group for sulfate particles
    sulfate_group = musica.carma.CARMAGroupConfig(
        name="sulfate",
        shortname="SULF",
        rmin=2.0e-10,  # Minimum radius in meters
        rmrat=2.0,  # Radius ratio for the group
        swelling_approach={
            "algorithm": musica.carma.ParticleSwellingAlgorithm.WEIGHT_PERCENT_H2SO4,
            "composition": musica.carma.ParticleSwellingComposition.NONE
        },
        is_sulfate=True,
        do_drydep=True,
    )
    params.groups.append(sulfate_group)

    # Set up an element for sulfate
    sulfate_element = musica.carma.CARMAElementConfig(
        name="Sulfate",
        shortname="SULF",
        rho=DENSITY_SULFATE,  # Density in kg/m³
        itype=musica.carma.ParticleType.VOLATILE,
        icomposition=musica.carma.ParticleComposition.SULFURIC_ACID,
        igroup=1,  # Group index for sulfate
    )
    params.elements.append(sulfate_element)

    # Set up gases for water and sulfuric acid
    water = musica.carma.CARMAGasConfig(
        name="Water Vapor",
        shortname="Q",
        wtmol=MOLECULAR_MASS_H2O,  # Molar mass of water in kg/mol
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_MURPHY_2005,
        icomposition=musica.carma.GasComposition.H2O,
        dgc_threshold=0.1,
        ds_threshold=0.1,
    )
    params.gases.append(water)

    h2so4 = musica.carma.CARMAGasConfig(
        name="Sulfuric Acid",
        shortname="H2SO4",
        wtmol=MOLECULAR_MASS_H2SO4,  # Molar mass of sulfuric acid in kg/mol
        ivaprtn=musica.carma.VaporizationAlgorithm.H2SO4_AYERS_1980,
        icomposition=musica.carma.GasComposition.H2SO4,
        dgc_threshold=0.1,
        ds_threshold=0.1,
    )
    params.gases.append(h2so4)

    # Add microphysical processes
    h2so4_uptake = musica.carma.CARMAGrowthConfig(
        ielem=1,  # Element index for sulfate
        igas=2,  # Gas index for sulfuric acid
    )
    params.growths.append(h2so4_uptake)
    
    nucleation = musica.carma.CARMANucleationConfig(
        ielemfrom=1,  # Element index for sulfate
        ielemto=1,  # Element index for sulfuric acid
        igas=2,  # Gas index for sulfuric acid
        algorithm=musica.carma.ParticleNucleationAlgorithm.HOMOGENEOUS_NUCLEATION,
    )
    params.nucleations.append(nucleation)

    coagulation = musica.carma.CARMACoagulationConfig(
        igroup1=1,  # Group index for sulfate (from)
        igroup2=1,  # Group index for sulfate (from)
        igroup3=1,  # Group index for sulfate (to)
        algorithm=musica.carma.ParticleCollectionAlgorithm.FUCHS,
    )
    params.coagulations.append(coagulation)

    # Set initialization options
    params.initialization.do_substep=True
    params.initialization.do_thermo=True
    params.initialization.maxretries=16
    params.initialization.maxsubsteps=32
    params.initialization.dt_threshold=1.0
    params.initialization.sulfnucl_method = musica.carma.SulfateNucleationMethod.ZHAO_TURCO

    return musica.CARMA(params)


def get_initial_conditions():
    """
    Initializes and returns the environmental conditions and chemical species concentrations
    for a box model simulation.
    Returns:
      tuple:
        - environmental_conditions (dict): Dictionary containing arrays for temperature (K)
          and pressure (Pa) for each grid cell.
        - initial_concentrations (dict): Dictionary containing arrays for the initial
          concentrations (in mol/m^3) of chemical species ('HO2', 'H2O2', 'OH', 'SO2', 'SO3',
          'H2SO4', 'H2O', 'M') for each grid cell.
    Notes:
      - The number of grid cells is determined by the constant NUMBER_OF_GRID_CELLS.
      - Concentrations are calculated using the conversion factor MOLEC_CM3_TO_MOLE_M3.
      - Some species are initialized to zero concentration.
    """
    
    temperature = np.full(NUMBER_OF_GRID_CELLS, 279.3)  # Temperature in Kelvin
    pressure = np.full(NUMBER_OF_GRID_CELLS, 101300.0)  # Pressure in Pascals
    air_density = pressure / (GAS_CONSTANT * temperature)  # Ideal gas law: rho = P / (R * T)

    environmental_conditions = {
      "temperature": temperature,
      "pressure": pressure,
      "pressure levels": np.full(NUMBER_OF_GRID_CELLS+1, 101300.0),  # Pressure in Pascals
      "air density": air_density
    }
    initial_concentrations = {
      "HO2": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 80.0e-12 * MOLEC_CM3_TO_MOLE_M3),
      "H2O2": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 1.0e-9 * MOLEC_CM3_TO_MOLE_M3),
      "SO2": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 0.15e-9 * MOLEC_CM3_TO_MOLE_M3),
      "SO3": np.zeros(NUMBER_OF_GRID_CELLS),
      "H2SO4": np.zeros(NUMBER_OF_GRID_CELLS),
      "H2O": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 0.004 * MOLEC_CM3_TO_MOLE_M3),
    }
    return environmental_conditions, initial_concentrations


def run_box_model():
    # Create the mechanism and solvers
    mechanism = create_mechanism()
    solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)
    state = solver.create_state(number_of_grid_cells=NUMBER_OF_GRID_CELLS)

    try:
        carma = create_carma_solver()
    except Exception as e:
        print(f"Error creating CARMA solver: {e}")
        print(f'Error type: {type(e)}')
        import traceback
        traceback.print_exc()

    # Set initial conditions
    environmental_conditions, initial_concentrations = get_initial_conditions()
    state.set_conditions(environmental_conditions["temperature"], environmental_conditions["pressure"])
    state.set_concentrations(initial_concentrations)

    # Run the simulation for 6 hours with a timestep of 30 seconds
    time_hours = 6.0
    time_seconds = time_hours * 3600.0
    dt = 30.0
    num_steps = int(time_seconds / dt)

    # Set up the vertical grid in CARMA
    zmin = 0.0
    deltaz = 100.0
    vertical_center = zmin + (np.arange(NUMBER_OF_GRID_CELLS) + 0.5) * deltaz
    vertical_levels = zmin + np.arange(NUMBER_OF_GRID_CELLS + 1) * deltaz

    # Initialize state arrays
    sulfate_mmr = np.zeros((NUMBER_OF_AEROSOL_SECTIONS, NUMBER_OF_GRID_CELLS))
    carma_water = np.zeros(NUMBER_OF_GRID_CELLS)
    carma_sulfuric_acid = np.zeros(NUMBER_OF_GRID_CELLS)
    output_state = state.get_concentrations()
    output_state["SULFATE"] = sulfate_mmr
    concentrations = [output_state.copy()]
    temperatures = [environmental_conditions["temperature"]]
    pressures = [environmental_conditions["pressure"]]
    current_temperature = environmental_conditions["temperature"].copy()
    current_pressure = environmental_conditions["pressure"].copy()

    for i_time in range(num_steps):

        state.set_conditions(temperatures=current_temperature, pressures=current_pressure)
        solver.solve(state, dt)
        micm_output = state.get_concentrations()

        carma_state = carma.create_state(
            vertical_center=vertical_center,
            vertical_levels=vertical_levels,
            temperature=current_temperature,
            pressure=environmental_conditions["pressure"],
            pressure_levels=environmental_conditions["pressure levels"],
            time=i_time * dt,
            time_step=dt,
            latitude=-40.0,
            longitude=-105.0,
        )

        for i_bin in range(NUMBER_OF_AEROSOL_SECTIONS):
            carma_state.set_bin(
                bin_index=i_bin+1,
                element_index=1,  # Sulfate element index
                value=sulfate_mmr[i_bin, :]
            )

        carma_state.set_gas(
            gas_index=1,  # Water vapor gas index
            value=micm_output["H2O"] / environmental_conditions["air density"] * MOLECULAR_MASS_H2O / MOLECULAR_MASS_AIR,
            old_mmr=carma_water
        )
        carma_state.set_gas(
            gas_index=2,  # Sulfuric acid gas index
            value=micm_output["H2SO4"] / environmental_conditions["air density"] * MOLECULAR_MASS_H2SO4 / MOLECULAR_MASS_AIR,
            old_mmr=carma_sulfuric_acid
        )

        carma_state.step()

        carma_env = carma_state.get_environmental_values()
        current_temperature = carma_env["temperature"]
        current_pressure = carma_env["pressure"]

        for i_bin in range(NUMBER_OF_AEROSOL_SECTIONS):
            sulfate_mmr[i_bin, :] = carma_state.get_bin(
                bin_index=i_bin+1,
                element_index=1  # Sulfate element index
            )["mass_mixing_ratio"]
        micm_output["SULFATE"] = sulfate_mmr
        micm_output["H2O"] = carma_state.get_gas(
            gas_index=1  # Water vapor gas index
        )["mass_mixing_ratio"] * environmental_conditions["air density"] * MOLECULAR_MASS_AIR / MOLECULAR_MASS_H2O
        micm_output["H2SO4"] = carma_state.get_gas(
            gas_index=2  # Sulfuric acid gas index
        )["mass_mixing_ratio"] * environmental_conditions["air density"] * MOLECULAR_MASS_AIR / MOLECULAR_MASS_H2SO4

        temperatures.append(current_temperature)
        pressures.append(current_pressure)
        concentrations.append(micm_output)


    # Collect output data
    concentrations = pd.DataFrame(concentrations)
    times = np.arange(num_steps + 1) * dt / 3600.0  # Time in hours

    return concentrations, times


def test_sulfate_box_model():
    # Test the sulfate box model implementation
    run_box_model()


def plot_results(concentrations, times):
    """    Plots the results of the sulfate box model simulation.
    Args:
        concentrations (pd.DataFrame): DataFrame containing the concentrations of chemical species over time.
        times (np.ndarray): Array of time values corresponding to the concentrations.
    """
    # Plot H2O and H2SO4 concentrations over time at the first grid cell
    h2o_concentration = np.array([conc[0] for conc in [conc["H2O"] for _, conc in concentrations.iterrows()]])
    h2so4_concentration = np.array([conc[0] for conc in [conc["H2SO4"] for _, conc in concentrations.iterrows()]])
    plt.figure(figsize=(12, 6))
    plt.plot(times, h2o_concentration, label="H2O Concentration (moles/m³)")
    plt.plot(times, h2so4_concentration, label="H2SO4 Concentration (moles/m³)")
    plt.xlabel("Time (hours)")
    plt.ylabel("Concentration (moles/m³)")
    plt.title("Sulfate Box Model: H2O and H2SO4 Concentration Over Time")
    plt.legend()
    plt.grid()
    plt.show()

    # Plot sulfate mmr in each bin over time at the first grid cell
    sulfate_mmr = np.array([conc[0] for conc in [conc["SULFATE"] for _, conc in concentrations.iterrows()]])
    for i_bin in range(NUMBER_OF_AEROSOL_SECTIONS):
        plt.plot(times, sulfate_mmr[:, i_bin], label=f"Bin {i_bin + 1}")
    plt.xlabel("Time (hours)")
    plt.ylabel("Sulfate Molar Mass Mixing Ratio (kg/kg)")
    plt.title("Sulfate Box Model: Molar Mass Mixing Ratio Over Time")
    plt.legend()
    plt.grid()
    plt.show()


if __name__ == "__main__":
    concentrations, times = run_box_model()
    plot_results(concentrations, times)