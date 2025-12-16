import pytest
import numpy as np
import musica
import random
import musica.mechanism_configuration as mc
from musica.cuda import is_cuda_available
from musica.constants import GAS_CONSTANT


def oregonator():
    """Helper function to create the Oregonator mechanism."""
    # Create species
    X = mc.Species(name="X")
    Y = mc.Species(name="Y")
    Z = mc.Species(name="Z")
    P = mc.Species(name="P")
    Q = mc.Species(name="Q")

    # Create gas phase
    gas_phase = mc.Phase(name="gas", species=[X, Y, Z, P, Q])

    # Create processes
    r1 = mc.UserDefined(
        name="r1",
        reactants=[Y],
        products=[X]
    )

    r2 = mc.UserDefined(
        name="r2",
        reactants=[X, Y],
        products=[P]
    )

    r3 = mc.UserDefined(
        name="r3",
        reactants=[X],
        products=[Z, X, X]
    )

    r4 = mc.UserDefined(
        name="r4",
        reactants=[X, X],
        products=[Q]
    )

    r5 = mc.UserDefined(
        name="r5",
        reactants=[Z],
        products=[Y]
    )

    # Create mechanism
    mechanism = mc.Mechanism(
        name="Oregonator",
        species=[X, Y, Z, P, Q],
        phases=[gas_phase],
        reactions=[r1, r2, r3, r4, r5]
    )

    return mechanism


def robertson():
    """Helper function to create the Robertson mechanism."""
    # Create species
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    C = mc.Species(name="C")

    # Create gas phase
    gas_phase = mc.Phase(name="gas", species=[A, B, C])

    # Create processes
    r1 = mc.UserDefined(
        name="r1",
        reactants=[A],
        products=[B]
    )

    r2 = mc.UserDefined(
        name="r2",
        reactants=[B, B],
        products=[B, C]
    )

    r3 = mc.UserDefined(
        name="r3",
        reactants=[B, C],
        products=[A, C]
    )

    # Create mechanism
    mechanism = mc.Mechanism(
        name="Robertson",
        species=[A, B, C],
        phases=[gas_phase],
        reactions=[r1, r2, r3]
    )

    return mechanism


def test_oregonator():
    """Test setting and getting environmental conditions."""
    # Use the test mechanism
    mech = oregonator()
    solver = musica.MICM(mechanism=mech)

    # Test single grid cell
    state = solver.create_state(number_of_grid_cells=1)
    state.set_concentrations({"X": 1.0, "Y": 2.0, "Z": 3.0})

    state.set_user_defined_rate_parameters({
        "USER.r1": 1.34 * 0.06,
        "USER.r2": 1.6e9,
        "USER.r3": 8e3 * 0.06,
        "USER.r4": 4e7,
        "USER.r5": 1.0,
    })

    tau = 0.1610
    time_step = 30 * tau
    for i in range(12):
        actual_solve = 0
        while actual_solve < time_step:
            result = solver.solve(state, time_step=time_step - actual_solve)
            actual_solve += result.stats.final_time
            if (actual_solve < time_step):
                print(f"  Partial solve to {actual_solve:.6f} seconds, solver state: {result.state}")


def test_robertson():
    """Test setting and getting environmental conditions."""
    # Use the test mechanism
    mech = robertson()
    solver = musica.MICM(mechanism=mech)

    # Test single grid cell
    state = solver.create_state(number_of_grid_cells=1)
    state.set_concentrations({"A": 1.0, "B": 0.0, "C": 0.0})

    state.set_user_defined_rate_parameters({
        "USER.r1": 0.04,
        "USER.r2": 3e7,
        "USER.r3": 1e4,
    })

    temperature = 272.5
    pressure = 101253.3
    air_density = 1e6

    state.set_conditions(
        temperatures=temperature,
        pressures=pressure,
        air_densities=air_density
    )

    time_step = 1
    for i in range(12):
        actual_solve = 0
        while actual_solve < time_step:
            result = solver.solve(state, time_step=time_step - actual_solve)
            actual_solve += result.stats.final_time
            if (actual_solve < time_step):
                print(f"  Partial solve to {actual_solve:.6f} seconds, solver state: {result.state}")
        time_step *= 10


def TestSingleGridCell(solver, state, time_step, places=5):
    temperature = 272.5
    pressure = 101253.3
    air_density = pressure / (GAS_CONSTANT * temperature)

    rate_constants = {
        "USER.reaction 1": 0.001,
        "USER.reaction 2": 0.002
    }

    concentrations = {
        "A": 0.75,
        "B": 0,
        "C": 0.4,
        "D": 0.8,
        "E": 0,
        "F": 0.1
    }
    state.set_conditions(temperature, pressure, air_density)
    state.set_concentrations(concentrations)
    state.set_user_defined_rate_parameters(rate_constants)

    # test to make sure a second call to set_conditions with an empty dictionary does not change the values
    state.set_concentrations({})
    state.set_user_defined_rate_parameters({})

    initial_concentrations = state.get_concentrations()
    initial_rate_parameters = state.get_user_defined_rate_parameters()
    initial_temperatures = state.get_conditions()["temperature"]
    initial_pressures = state.get_conditions()["pressure"]
    initial_air_density = state.get_conditions()["air_density"]
    assert np.isclose(initial_concentrations["A"][0], concentrations["A"], atol=1e-13)
    assert np.isclose(initial_concentrations["B"][0], concentrations["B"], atol=1e-13)
    assert np.isclose(initial_concentrations["C"][0], concentrations["C"], atol=1e-13)
    assert np.isclose(initial_concentrations["D"][0], concentrations["D"], atol=1e-13)
    assert np.isclose(initial_concentrations["E"][0], concentrations["E"], atol=1e-13)
    assert np.isclose(initial_concentrations["F"][0], concentrations["F"], atol=1e-13)
    assert np.isclose(initial_rate_parameters["USER.reaction 1"][0], rate_constants["USER.reaction 1"], atol=1e-13)
    assert np.isclose(initial_rate_parameters["USER.reaction 2"][0], rate_constants["USER.reaction 2"], atol=1e-13)
    assert np.isclose(initial_temperatures[0], temperature, atol=1e-13)
    assert np.isclose(initial_pressures[0], pressure, atol=1e-13)
    assert np.isclose(initial_air_density[0], air_density, atol=1e-13)

    time_step = 1
    sim_length = 100

    curr_time = time_step
    initial_A = initial_concentrations["A"][0]
    initial_C = initial_concentrations["C"][0]
    initial_D = initial_concentrations["D"][0]
    initial_F = initial_concentrations["F"][0]
    # Gets analytical concentrations
    while curr_time <= sim_length:
        solver.solve(state, time_step)
        concentrations = state.get_concentrations()
        k1 = rate_constants["USER.reaction 1"]
        k2 = rate_constants["USER.reaction 2"]
        k3 = 0.004 * np.exp(50.0 / temperature)
        k4 = 0.012 * np.exp(75.0 / temperature) * \
            (temperature / 50.0)**(-2) * (1.0 + 1.0e-6 * pressure)
        A_conc = initial_A * np.exp(-(k3) * curr_time)
        B_conc = initial_A * (k3 / (k4 - k3)) * \
            (np.exp(-k3 * curr_time) - np.exp(-k4 * curr_time))
        C_conc = initial_C + initial_A * \
            (1.0 + (k3 * np.exp(-k4 * curr_time) - k4 * np.exp(-k3 * curr_time)) / (k4 - k3))
        D_conc = initial_D * np.exp(-(k1) * curr_time)
        E_conc = initial_D * (k1 / (k2 - k1)) * \
            (np.exp(-k1 * curr_time) - np.exp(-k2 * curr_time))
        F_conc = initial_F + initial_D * \
            (1.0 + (k1 * np.exp(-k2 * curr_time) - k2 * np.exp(-k1 * curr_time)) / (k2 - k1))

        assert np.isclose(concentrations["A"][0], A_conc, atol=10**-places)
        assert np.isclose(concentrations["B"][0], B_conc, atol=10**-places)
        assert np.isclose(concentrations["C"][0], C_conc, atol=10**-places)
        assert np.isclose(concentrations["D"][0], D_conc, atol=10**-places)
        assert np.isclose(concentrations["E"][0], E_conc, atol=10**-places)
        assert np.isclose(concentrations["F"][0], F_conc, atol=10**-places)

        curr_time += time_step


def TestMultipleGridCell(solver, state, num_grid_cells, time_step, places=5):
    concentrations = {
        "A": [],
        "B": [],
        "C": [],
        "D": [],
        "E": [],
        "F": []
    }
    rate_constants = {
        "USER.reaction 1": [],
        "USER.reaction 2": []
    }
    temperatures = []
    pressures = []

    for i in range(num_grid_cells):
        temperatures.append(275.0 + random.uniform(-50.0, 50.0))
        pressures.append(101253.3 + random.uniform(-500.0, 500.0))
        concentrations["A"].append(0.75 + random.uniform(-0.05, 0.05))
        concentrations["B"].append(0)
        concentrations["C"].append(0.4 + random.uniform(-0.05, 0.05))
        concentrations["D"].append(0.8 + random.uniform(-0.05, 0.05))
        concentrations["E"].append(0)
        concentrations["F"].append(0.1 + random.uniform(-0.05, 0.05))
        rate_constants["USER.reaction 1"].append(
            0.001 + random.uniform(-0.0001, 0.0001))
        rate_constants["USER.reaction 2"].append(
            0.002 + random.uniform(-0.0001, 0.0001))

    state.set_conditions(temperatures, pressures)  # Air density should be calculated in the state
    state.set_concentrations(concentrations)
    state.set_user_defined_rate_parameters(rate_constants)

    initial_concentrations = state.get_concentrations()
    initial_rate_parameters = state.get_user_defined_rate_parameters()
    initial_temperatures = state.get_conditions()["temperature"]
    initial_pressures = state.get_conditions()["pressure"]
    initial_air_density = state.get_conditions()["air_density"]

    for i in range(num_grid_cells):
        assert np.isclose(initial_concentrations["A"][i], concentrations["A"][i], atol=1e-13)
        assert np.isclose(initial_concentrations["B"][i], concentrations["B"][i], atol=1e-13)
        assert np.isclose(initial_concentrations["C"][i], concentrations["C"][i], atol=1e-13)
        assert np.isclose(initial_concentrations["D"][i], concentrations["D"][i], atol=1e-13)
        assert np.isclose(initial_concentrations["E"][i], concentrations["E"][i], atol=1e-13)
        assert np.isclose(initial_concentrations["F"][i], concentrations["F"][i], atol=1e-13)
        assert np.isclose(
            initial_rate_parameters["USER.reaction 1"][i],
            rate_constants["USER.reaction 1"][i],
            atol=1e-13)
        assert np.isclose(
            initial_rate_parameters["USER.reaction 2"][i],
            rate_constants["USER.reaction 2"][i],
            atol=1e-13)
        assert np.isclose(initial_temperatures[i], temperatures[i], atol=1e-13)
        assert np.isclose(initial_pressures[i], pressures[i], atol=1e-13)
        assert np.isclose(initial_air_density[i], pressures[i] / (8.31446261815324 * temperatures[i]), atol=1e-13)

    time_step = 1
    sim_length = 100

    curr_time = time_step
    initial_A = num_grid_cells * [0.0]
    initial_C = num_grid_cells * [0.0]
    initial_D = num_grid_cells * [0.0]
    initial_F = num_grid_cells * [0.0]
    for i in range(num_grid_cells):
        initial_A[i] = initial_concentrations["A"][i]
        initial_C[i] = initial_concentrations["C"][i]
        initial_D[i] = initial_concentrations["D"][i]
        initial_F[i] = initial_concentrations["F"][i]

    k1 = num_grid_cells * [0.0]
    k2 = num_grid_cells * [0.0]
    k3 = num_grid_cells * [0.0]
    k4 = num_grid_cells * [0.0]
    for i in range(num_grid_cells):
        k1[i] = rate_constants["USER.reaction 1"][i]
        k2[i] = rate_constants["USER.reaction 2"][i]
        k3[i] = 0.004 * np.exp(50.0 / temperatures[i])
        k4[i] = 0.012 * np.exp(75.0 / temperatures[i]) * \
            (temperatures[i] / 50.0)**(-2) * (1.0 + 1.0e-6 * pressures[i])

    while curr_time <= sim_length:
        solver.solve(state, time_step)
        concentrations = state.get_concentrations()

        for i in range(num_grid_cells):
            A_conc = initial_A[i] * np.exp(-(k3[i]) * curr_time)
            B_conc = initial_A[i] * (k3[i] / (k4[i] - k3[i])) * \
                (np.exp(-k3[i] * curr_time) - np.exp(-k4[i] * curr_time))
            C_conc = initial_C[i] + initial_A[i] * (1.0 + (
                k3[i] * np.exp(-k4[i] * curr_time) - k4[i] * np.exp(-k3[i] * curr_time)) / (k4[i] - k3[i]))
            D_conc = initial_D[i] * np.exp(-(k1[i]) * curr_time)
            E_conc = initial_D[i] * (k1[i] / (k2[i] - k1[i])) * \
                (np.exp(-k1[i] * curr_time) - np.exp(-k2[i] * curr_time))
            F_conc = initial_F[i] + initial_D[i] * (1.0 + (
                k1[i] * np.exp(-k2[i] * curr_time) - k2[i] * np.exp(-k1[i] * curr_time)) / (k2[i] - k1[i]))

            assert np.isclose(
                concentrations["A"][i],
                A_conc,
                atol=10**-places), f"Grid cell {i} of {num_grid_cells}: A concentration mismatch. Initial A: {initial_concentrations['A'][i]}"
            assert np.isclose(
                concentrations["B"][i],
                B_conc,
                atol=10**-places), f"Grid cell {i} of {num_grid_cells}: B concentration mismatch. Initial B: {initial_concentrations['B'][i]}"
            assert np.isclose(
                concentrations["C"][i],
                C_conc,
                atol=10**-places), f"Grid cell {i} of {num_grid_cells}: C concentration mismatch. Initial C: {initial_concentrations['C'][i]}"
            assert np.isclose(
                concentrations["D"][i],
                D_conc,
                atol=10**-places), f"Grid cell {i} of {num_grid_cells}: D concentration mismatch. Initial D: {initial_concentrations['D'][i]}"
            assert np.isclose(
                concentrations["E"][i],
                E_conc,
                atol=10**-places), f"Grid cell {i} of {num_grid_cells}: E concentration mismatch. Initial E: {initial_concentrations['E'][i]}"
            assert np.isclose(
                concentrations["F"][i],
                F_conc,
                atol=10**-places), f"Grid cell {i} of {num_grid_cells}: F concentration mismatch. Initial F: {initial_concentrations['F'][i]}"

        curr_time += time_step


def GetMechanism():
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    C = mc.Species(name="C")
    D = mc.Species(name="D")
    E = mc.Species(name="E")
    F = mc.Species(name="F")
    gas = mc.Phase(name="gas", species=[A, B, C, D, E, F])
    arr1 = mc.Arrhenius(name="A->B", A=0.004, C=50,
                        gas_phase=gas, reactants=[A], products=[B])
    arr2 = mc.Arrhenius(name="B->C", A=0.012, B=-2, C=75, D=50, E=1.0e-6,
                        gas_phase=gas, reactants=[B], products=[C])
    user1 = mc.UserDefined(name="reaction 1", gas_phase=gas,
                           reactants=[D], products=[E])
    user2 = mc.UserDefined(name="reaction 2", gas_phase=gas,
                           reactants=[E], products=[F])
    mechanism = mc.Mechanism(
        name="analytical test",
        species=[A, B, C, D, E, F],
        phases=[gas],
        reactions=[arr1, arr2, user1, user2],
    )
    return mechanism


def test_single_grid_cell_standard_rosenbrock():
    solver = musica.MICM(
        config_path="configs/v0/analytical",
        solver_type=musica.SolverType.rosenbrock_standard_order)
    state = solver.create_state()
    TestSingleGridCell(solver, state, 200.0, 5)


def test_multiple_grid_cells_standard_rosenbrock():
    for i in range(1, 11):
        solver = musica.MICM(
            config_path="configs/v0/analytical",
            solver_type=musica.SolverType.rosenbrock_standard_order)
        state = solver.create_state(i)
        TestMultipleGridCell(solver, state, i, 200.0, 5)


def test_cuda_rosenbrock():
    if is_cuda_available():
        solver = musica.MICM(
            config_path="configs/v0/analytical",
            solver_type=musica.SolverType.cuda_rosenbrock)
        state = solver.create_state()
        TestSingleGridCell(solver, state, 200.0, 5)
    else:
        pytest.skip("CUDA is not available.")


def test_single_grid_cell_backward_euler():
    solver = musica.MICM(
        config_path="configs/v0/analytical",
        solver_type=musica.SolverType.backward_euler_standard_order)
    state = solver.create_state()
    TestSingleGridCell(solver, state, 10.0, places=2)


def test_multiple_grid_cells_backward_euler():
    for i in range(1, 11):
        solver = musica.MICM(
            config_path="configs/v0/analytical",
            solver_type=musica.SolverType.backward_euler_standard_order)
        state = solver.create_state(i)
        TestMultipleGridCell(solver, state, i, 10.0, places=2)


def test_single_grid_cell_rosenbrock():
    solver = musica.MICM(
        config_path="configs/v0/analytical",
        solver_type=musica.SolverType.rosenbrock)
    state = solver.create_state()
    TestSingleGridCell(solver, state, 200.0, 5)


def test_multiple_grid_cells_rosenbrock():
    for i in range(1, 11):
        solver = musica.MICM(
            config_path="configs/v0/analytical",
            solver_type=musica.SolverType.rosenbrock)
        state = solver.create_state(i)
        TestMultipleGridCell(solver, state, i, 200.0, 5)


def test_single_grid_cell_backward_euler_standard_order():
    solver = musica.MICM(
        config_path="configs/v0/analytical",
        solver_type=musica.SolverType.backward_euler_standard_order)
    state = solver.create_state()
    TestSingleGridCell(solver, state, 10.0, places=2)


def test_multiple_grid_cells_backward_euler_standard_order():
    for i in range(1, 11):
        solver = musica.MICM(
            config_path="configs/v0/analytical",
            solver_type=musica.SolverType.backward_euler_standard_order)
        state = solver.create_state(i)
        TestMultipleGridCell(solver, state, i, 10.0, places=2)


if __name__ == '__main__':
    pytest.main()
