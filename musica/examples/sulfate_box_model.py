"""
Simple box model with MICM and CARMA

This script creates one column and solves gas-phase sulfuric acid formation with
MICM and nucleation, growth, and coagulation of aerosols with CARMA.

UNDER DEVELOPMENT
"""
import musica
from musica.constants import GAS_CONSTANT
import musica.mechanism_configuration as mc
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import ussa1976
import xarray as xr

available = musica.backend.carma_available()

MOLEC_CM3_TO_MOLE_M3 = np.float64(1.0e6) / np.float64(6.022e23)  # Convert from molecules/cm³ to moles/m³
NUMBER_OF_GRID_CELLS = 120  # Number of grid cells for the simulation
NUMBER_OF_AEROSOL_SECTIONS = 38  # Number of aerosol sections for the simulation
DENSITY_SULFATE = np.float64(1923.0)  # Density of sulfate in kg/m³
MOLECULAR_MASS_H2O = np.float64(0.01801528)  # kg/mol
MOLECULAR_MASS_H2SO4 = np.float64(0.098078479)  # kg/mol
MOLECULAR_MASS_AIR = np.float64(0.029)  # kg/mol (average molecular mass of air)


def create_mechanism():
    """
    Creates a chemical mechanism for the gas-phase reactions involved in sulfate formation.
    """
    HO2 = mc.Species(name="HO2")
    H2O2 = mc.Species(name="H2O2")
    OH = mc.Species(name="OH", constant_mixing_ratio_mol_mol=0.8e-11)
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
        A=3.0e-13 / MOLEC_CM3_TO_MOLE_M3,
        Ea=-6.35099e-21)

    rxn_2HO2_M_H2O2 = mc.Arrhenius(
        name="2HO2 + M -> H2O2",
        reactants=[HO2, HO2, M],
        products=[H2O2],
        gas_phase=gas,
        A=2.1e-33 / (MOLEC_CM3_TO_MOLE_M3**2),
        Ea=-1.2702e-20)

    rxn_2HO2_H2O_H2O2 = mc.Arrhenius(
        name="2H2O2 + H2O -> H2O2 + H2O",
        reactants=[HO2, HO2, H2O],
        products=[H2O2, H2O],
        gas_phase=gas,
        A=4.2e-34 / (MOLEC_CM3_TO_MOLE_M3**2),
        Ea=-3.67253e-20)

    rxn_2HO2_H2O_M_H2O2 = mc.Arrhenius(
        name="2HO2 + H2O + M -> H2O2 + H2O",
        reactants=[HO2, HO2, H2O, M],
        products=[H2O2, H2O],
        gas_phase=gas,
        A=2.94e-54 / (MOLEC_CM3_TO_MOLE_M3**3),
        Ea=-4.30762e-20)

    rxn_H2O2_OH_HO2_H2O = mc.Arrhenius(
        name="H2O2 + OH -> HO2 + H2O",
        reactants=[H2O2, OH],
        products=[HO2, H2O],
        gas_phase=gas,
        A=1.8e-12 / MOLEC_CM3_TO_MOLE_M3)

    rxn_SO2_OH_SO3_HO2 = mc.Troe(
        name="SO2 + OH -> SO3 + HO2",
        reactants=[SO2, OH],
        products=[SO3, HO2],
        gas_phase=gas,
        k0_A=2.9e-31 / MOLEC_CM3_TO_MOLE_M3,
        k0_B=-4.1,
        kinf_A=1.7e-12 / MOLEC_CM3_TO_MOLE_M3,
        kinf_B=-0.2)

    rxn_SO3_2H2O_H2SO4_H2O = mc.Arrhenius(
        name="SO3 + 2H2O -> H2SO4 + H2O",
        reactants=[SO3, H2O, H2O],
        products=[H2SO4, H2O],
        gas_phase=gas,
        A=8.5e-41 / (MOLEC_CM3_TO_MOLE_M3**2),
        C=6540.0)

    rxn_SO2 = mc.Emission(
        name="SO2",
        products=[SO2],
        gas_phase=gas
    )

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
            rxn_SO3_2H2O_H2SO4_H2O,
            rxn_SO2
        ],
        species=[HO2, H2O2, OH, SO2, SO3, H2SO4, H2O, M]
    )


def create_carma_solver():
    """
    Creates a CARMA solver with the sulfate box model configuration.
    """
    params = musica.CARMAParameters()
    params.nz = NUMBER_OF_GRID_CELLS
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
        shortname="H2O",
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
    params.initialization.do_substep = True
    params.initialization.do_thermo = True
    params.initialization.maxretries = 16
    params.initialization.maxsubsteps = 32
    params.initialization.dt_threshold = 1.0
    params.initialization.sulfnucl_method = musica.carma.SulfateNucleationMethod.ZHAO_TURCO.value

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

    # Set up the vertical grid in CARMA
    zmin = np.float64(0.0)
    deltaz = np.float64(100.0)
    grid = {
        "vertical_center": zmin + (np.arange(NUMBER_OF_GRID_CELLS, dtype=np.float64) + 0.5) * deltaz,
        "vertical_levels": zmin + np.arange(NUMBER_OF_GRID_CELLS + 1, dtype=np.float64) * deltaz
    }

    centered_variables = ussa1976.compute(z=grid["vertical_center"], variables=["t", "p"])
    edge_variables = ussa1976.compute(z=grid["vertical_levels"], variables=["p"])
    centered_variables["rho"] = centered_variables["p"] / (GAS_CONSTANT * centered_variables["t"])
    environmental_conditions = {
        "temperature": np.array(centered_variables["t"], dtype=np.float64),
        "pressure": np.array(centered_variables["p"], dtype=np.float64),
        "pressure levels": np.array(edge_variables["p"], dtype=np.float64),
        "air density": np.array(centered_variables["rho"], dtype=np.float64)
    }

    initial_concentrations = {
        "HO2": environmental_conditions["air density"] * 80.0e-12,
        "H2O2": environmental_conditions["air density"] * 1.0e-9,
        "SO2": environmental_conditions["air density"] * 0.15e-9,
        "SO3": np.zeros(NUMBER_OF_GRID_CELLS, dtype=np.float64),
        "H2SO4": np.zeros(NUMBER_OF_GRID_CELLS, dtype=np.float64),
        "H2O": environmental_conditions["air density"] * 0.004,
    }
    return grid, environmental_conditions, initial_concentrations


def run_box_model():
    # Create the mechanism and solvers
    mechanism = create_mechanism()
    solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)
    state = solver.create_state(number_of_grid_cells=NUMBER_OF_GRID_CELLS)

    carma = create_carma_solver()

    # Set initial conditions
    grid, environmental_conditions, initial_concentrations = get_initial_conditions()
    state.set_conditions(environmental_conditions["temperature"], environmental_conditions["pressure"])
    state.set_concentrations(initial_concentrations)
    state.set_user_defined_rate_parameters({"EMIS.SO2": np.full(NUMBER_OF_GRID_CELLS, 1.0e-8, dtype=np.float64)})

    # Run the simulation for 6 hours with a timestep of 30 seconds
    time_hours = 2.0
    time_seconds = time_hours * np.float64(3600.0)
    dt = np.float64(30.0)
    num_steps = int(time_seconds / dt)

    # Initialize combined state arrays
    concentrations = [state.get_concentrations()]
    current_temperature = environmental_conditions["temperature"].copy()
    bin_state = None

    # Set up additional CARMA state arrays
    satliq_h2o = np.full(NUMBER_OF_GRID_CELLS, -1.0, dtype=np.float64)
    satice_h2o = np.full(NUMBER_OF_GRID_CELLS, -1.0, dtype=np.float64)
    satliq_h2so4 = np.full(NUMBER_OF_GRID_CELLS, -1.0, dtype=np.float64)
    satice_h2so4 = np.full(NUMBER_OF_GRID_CELLS, -1.0, dtype=np.float64)
    last_h2so4_mmr = np.zeros(NUMBER_OF_GRID_CELLS, dtype=np.float64)

    for i_time in range(num_steps):
        state.set_conditions(temperatures=current_temperature, pressures=environmental_conditions["pressure"])
        solver.solve(state, dt)
        micm_output = state.get_concentrations()

        h2so4_mmr = np.array(micm_output["H2SO4"], dtype=np.float64) * MOLECULAR_MASS_H2SO4 / \
            (environmental_conditions["air density"] * MOLECULAR_MASS_AIR)
        h2o_mmr = np.array(micm_output["H2O"], dtype=np.float64) * MOLECULAR_MASS_H2O / \
            (environmental_conditions["air density"] * MOLECULAR_MASS_AIR)

        carma_state = carma.create_state(
            vertical_center=grid["vertical_center"],
            vertical_levels=grid["vertical_levels"],
            temperature=current_temperature,
            pressure=environmental_conditions["pressure"],
            pressure_levels=environmental_conditions["pressure levels"],
            time=i_time * dt,
            time_step=dt,
            latitude=-40.0,
            longitude=-105.0,
            specific_humidity=h2o_mmr,
        )

        if bin_state is None:
            bin_state = carma_state.get_bins()
            bin_state = bin_state.expand_dims({"time": [0.0]}).copy(deep=True)
            bin_state["mass_mixing_ratio"].values[0, :, :, :] = 0.0

        for i_bin in range(NUMBER_OF_AEROSOL_SECTIONS):
            carma_state.set_bin(
                bin_index=i_bin + 1,
                element_index=1,  # Sulfate element index
                value=bin_state.isel(time=i_time, bin=i_bin, element=0)["mass_mixing_ratio"].values,
            )

        carma_state.set_gas(
            gas_index=1,  # Water vapor gas index
            value=h2o_mmr,
            old_mmr=h2o_mmr,
            gas_saturation_wrt_liquid=satliq_h2o,
            gas_saturation_wrt_ice=satice_h2o
        )
        carma_state.set_gas(
            gas_index=2,  # Sulfuric acid gas index
            value=h2so4_mmr,
            old_mmr=last_h2so4_mmr,
            gas_saturation_wrt_liquid=satliq_h2so4,
            gas_saturation_wrt_ice=satice_h2so4
        )
        last_h2so4_mmr = h2so4_mmr

        carma_state.step()

        # Get updated state data from CARMA
        bin_state = xr.concat([bin_state, carma_state.get_bins().expand_dims(
            {"time": [(i_time + 1) * dt]})], dim="time")

        gas_state, gas_index = carma_state.get_gases()
        micm_output["H2O"] = np.array(
            gas_state.isel(
                gas=gas_index["H2O"])["gas_mass_mixing_ratio"],
            dtype=np.float64) * environmental_conditions["air density"] * MOLECULAR_MASS_AIR / MOLECULAR_MASS_H2O
        micm_output["H2SO4"] = np.array(
            gas_state.isel(
                gas=gas_index["H2SO4"])["gas_mass_mixing_ratio"],
            dtype=np.float64) * environmental_conditions["air density"] * MOLECULAR_MASS_AIR / MOLECULAR_MASS_H2SO4

        # save concentrations for output
        concentrations.append(micm_output)

        # prepare for next time step
        current_temperature = carma_state.get_environmental_values()["temperature"]
        state.set_concentrations({
            "H2O": micm_output["H2O"],
            "H2SO4": micm_output["H2SO4"]
        })

    # Collect output data
    concentrations = pd.DataFrame(concentrations)
    times = np.arange(num_steps + 1) * dt / 3600.0  # Time in hours

    return concentrations, times, bin_state


def plot_results(concentrations, times, sulfate_data=None):
    """Plots the results of the sulfate box model simulation.
    Args:
        concentrations (pd.DataFrame): DataFrame containing the concentrations of chemical species over time.
        times (np.ndarray): Array of time values corresponding to the concentrations.
        sulfate_data (xr.Dataset): xarray Dataset containing CARMA sulfate data over time.
    """

    # Create a 4-panel plot for H2O, H2SO4, SO2, and HO2 concentrations over time at the first grid cell
    fig, axs = plt.subplots(2, 2, figsize=(14, 10), sharex=True)
    species = ["H2O", "H2SO4", "SO2", "HO2"]
    titles = [
        "H2O Concentration (mol/m³)",
        "H2SO4 Concentration (mol/m³)",
        "SO2 Concentration (mol/m³)",
        "HO2 Concentration (mol/m³)"
    ]
    for ax, specie, title in zip(axs.flat, species, titles):
        conc = np.array([row[0] for row in [c[specie] for _, c in concentrations.iterrows()]])
        ax.plot(times, conc, label=specie)
        ax.set_title(title)
        ax.set_ylabel("Concentration (mol/m³)")
        ax.legend()
        ax.grid()
    for ax in axs[1]:
        ax.set_xlabel("Time (hours)")
    plt.tight_layout()
    plt.show()

    # Plot total sulfate mass mixing ratio (sum over all bins) over time at the first vertical level
    if sulfate_data is not None and hasattr(sulfate_data, "mass_mixing_ratio"):
        total_mmr = sulfate_data["mass_mixing_ratio"].isel(vertical_center=0).sum(dim="bin")
        total_mmr.plot(x="time")
        plt.title("Total Sulfate Mass Mixing Ratio (First Vertical Level)")
        plt.ylabel("Mass Mixing Ratio (kg/kg)")
        plt.xlabel("Time (hours)")
        plt.grid()
        plt.show()
    else:
        print("SULFATE data structure:", sulfate_data)
        if sulfate_data is not None:
            print(
                "Available variables:",
                list(
                    sulfate_data.data_vars) if hasattr(
                    sulfate_data,
                    'data_vars') else "No data_vars available")
            print(
                "Available dimensions:",
                list(
                    sulfate_data.dims) if hasattr(
                    sulfate_data,
                    'dims') else "No dims available")
        else:
            print("No SULFATE data available")


if __name__ == "__main__":
    if not available:
        print("CARMA backend is not available.")
    else:
        concs, plot_times, sulfate_data = run_box_model()
        plot_results(concs, plot_times, sulfate_data)
