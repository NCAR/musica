import musica
import musica.mechanism_configuration as mc
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
pd.set_option('display.float_format', str)
np.set_printoptions(suppress=True)

MOLEC_CM3_TO_MOLE_M3 = 1.0e6 / 6.022e23  # Convert from molecules/cm³ to moles/m³
NUMBER_OF_GRID_CELLS = 120  # Number of grid cells for the simulation


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
    
    environmental_conditions = {
      "temperature": np.full(NUMBER_OF_GRID_CELLS, 279.3),
      "pressure": np.full(NUMBER_OF_GRID_CELLS, 101300.0)
    }
    initial_concentrations = {
      "HO2": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 80.0e-12 * MOLEC_CM3_TO_MOLE_M3),
      "H2O2": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 1.0e-9 * MOLEC_CM3_TO_MOLE_M3),
      "SO2": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 0.15e-9 * MOLEC_CM3_TO_MOLE_M3),
      "SO3": np.zeros(NUMBER_OF_GRID_CELLS),
      "H2SO4": np.zeros(NUMBER_OF_GRID_CELLS),
      "H2O": np.full(NUMBER_OF_GRID_CELLS, 2.5e19 * 0.004 * MOLEC_CM3_TO_MOLE_M3)
    }
    return environmental_conditions, initial_concentrations


def run_box_model():
    # Create the mechanism and MICM solver
    mechanism = create_mechanism()
    solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)
    state = solver.create_state(number_of_grid_cells=NUMBER_OF_GRID_CELLS)

    # Set initial conditions
    environmental_conditions, initial_concentrations = get_initial_conditions()
    state.set_conditions(environmental_conditions["temperature"], environmental_conditions["pressure"])
    state.set_concentrations(initial_concentrations)

    # Run the simulation for 6 hours with a timestep of 30 seconds
    time_hours = 6.0
    time_seconds = time_hours * 3600.0
    dt = 30.0
    num_steps = int(time_seconds / dt)
    concentrations = [state.get_concentrations()]
    for _ in range(num_steps):
        solver.solve(state, dt)
        concentrations.append(state.get_concentrations())

    # Plot H2SO4 concentration over time
    concentrations = pd.DataFrame(concentrations)
    time = np.linspace(0.0, time_hours, num_steps + 1)
    plt.plot(time, [conc[0] for conc in concentrations["H2SO4"]])
    plt.xlabel("Time (hours)")
    plt.ylabel("H2SO4 Concentration (moles/m³)")
    plt.title("Sulfate Box Model: H2SO4 Concentration Over Time")
    plt.grid()
    plt.show()


if __name__ == "__main__":
    run_box_model()