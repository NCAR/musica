import unittest
import numpy as np
import musica
import random


def TestSingleGridCell(self, solver, state, time_step, places=5):
    num_grid_cells = 1

    conditions = state.conditions
    condition = conditions[0]
    condition.temperature = 272.5
    condition.pressure = 101253.3
    GAS_CONSTANT = 8.31446261815324
    condition.air_density = condition.pressure / (GAS_CONSTANT * condition.temperature)

    rates = musica.user_defined_reaction_rates(solver, state)
    species_ordering = musica.species_ordering(solver, state)
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
    updated_ordered_rate_constants = len(rate_constants.keys()) * [0.0]
    for key, value in rate_constants.items():
        updated_ordered_rate_constants[rates[key]] = value
    state.ordered_rate_constants = updated_ordered_rate_constants

    update_ordered_concentrations = len(concentrations.keys()) * [0.0]
    for key, value in concentrations.items():
        update_ordered_concentrations[species_ordering[key]] = value
    state.ordered_concentrations = update_ordered_concentrations

    initial_concentrations = state.ordered_concentrations

    time_step = 1
    sim_length = 100

    curr_time = time_step
    initial_A = initial_concentrations[species_ordering["A"]]
    initial_C = initial_concentrations[species_ordering["C"]]
    initial_D = initial_concentrations[species_ordering["D"]]
    initial_F = initial_concentrations[species_ordering["F"]]
    # Gets analytical concentrations
    while curr_time <= sim_length:
        musica.micm_solve(
            solver,
            state,
            time_step)
        k1 = state.ordered_rate_constants[rates["USER.reaction 1"]]
        k2 = state.ordered_rate_constants[rates["USER.reaction 2"]]
        k3 = 0.004 * np.exp(50.0 / condition.temperature)
        k4 = 0.012 * np.exp(75.0 / condition.temperature) * \
            (condition.temperature / 50.0)**(-2) * (1.0 + 1.0e-6 * condition.pressure)
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

        self.assertAlmostEqual(
            state.ordered_concentrations[species_ordering["A"]], A_conc, places=places)
        self.assertAlmostEqual(
            state.ordered_concentrations[species_ordering["B"]], B_conc, places=places)
        self.assertAlmostEqual(
            state.ordered_concentrations[species_ordering["C"]], C_conc, places=places)
        self.assertAlmostEqual(
            state.ordered_concentrations[species_ordering["D"]], D_conc, places=places)
        self.assertAlmostEqual(
            state.ordered_concentrations[species_ordering["E"]], E_conc, places=places)
        self.assertAlmostEqual(
            state.ordered_concentrations[species_ordering["F"]], F_conc, places=places)

        curr_time += time_step


class TestAnalyticalStandardRosenbrock(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock_standard_order,
            1)
        state = musica.create_state(solver)
        TestSingleGridCell(self, solver, state, 200.0, 5)


class TestAnalyticalStandardBackwardEuler(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.backward_euler_standard_order,
            1)
        state = musica.create_state(solver)
        TestSingleGridCell(self, solver, state, 10.0, places=2)


def TestStandardMultipleGridCell(self, solver, state, num_grid_cells, time_step, places=5):
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
    conditions = state.conditions

    for i in range(num_grid_cells):
        conditions[i].temperature = 275.0 + random.uniform(-10.0, 10.0)
        conditions[i].pressure = 101253.3 + random.uniform(-500.0, 500.0)
        conditions[i].air_density = conditions[i].pressure / (8.31446261815324 * conditions[i].temperature)
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

    rates_ordering = musica.user_defined_reaction_rates(solver, state)
    species_ordering = musica.species_ordering(solver, state)

    updated_ordered_rate_constants = (
        len(rate_constants.keys()) * num_grid_cells) * [0.0]
    for i in range(num_grid_cells):
        for key, value in rate_constants.items():
            updated_ordered_rate_constants[i *
                                           len(rate_constants.keys()) +
                                           rates_ordering[key]] = value[i]
    state.ordered_rate_constants = updated_ordered_rate_constants

    update_ordered_concentrations = (
        len(concentrations.keys()) * num_grid_cells) * [0.0]
    for i in range(num_grid_cells):
        for key, value in concentrations.items():
            update_ordered_concentrations[i *
                                          len(concentrations.keys()) +
                                          species_ordering[key]] = value[i]
    state.ordered_concentrations = update_ordered_concentrations

    initial_concentrations = state.ordered_concentrations

    time_step = 1
    sim_length = 100

    curr_time = time_step
    initial_A = num_grid_cells * [0.0]
    initial_C = num_grid_cells * [0.0]
    initial_D = num_grid_cells * [0.0]
    initial_F = num_grid_cells * [0.0]
    for i in range(num_grid_cells):
        initial_A[i] = initial_concentrations[i *
                                              len(concentrations.keys()) + species_ordering["A"]]
        initial_C[i] = initial_concentrations[i *
                                              len(concentrations.keys()) + species_ordering["C"]]
        initial_D[i] = initial_concentrations[i *
                                              len(concentrations.keys()) + species_ordering["D"]]
        initial_F[i] = initial_concentrations[i *
                                              len(concentrations.keys()) + species_ordering["F"]]

    k1 = num_grid_cells * [0.0]
    k2 = num_grid_cells * [0.0]
    k3 = num_grid_cells * [0.0]
    k4 = num_grid_cells * [0.0]
    for i in range(num_grid_cells):
        k1[i] = state.ordered_rate_constants[i *
                                             len(rate_constants.keys()) +
                                             rates_ordering["USER.reaction 1"]]
        k2[i] = state.ordered_rate_constants[i *
                                             len(rate_constants.keys()) +
                                             rates_ordering["USER.reaction 2"]]
        k3[i] = 0.004 * np.exp(50.0 / conditions[i].temperature)
        k4[i] = 0.012 * np.exp(75.0 / conditions[i].temperature) * \
            (conditions[i].temperature / 50.0)**(-2) * (1.0 + 1.0e-6 * conditions[i].pressure)

    while curr_time <= sim_length:
        musica.micm_solve(
            solver,
            state,
            time_step)

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

            self.assertAlmostEqual(state.ordered_concentrations[i *
                                                                len(concentrations.keys()) +
                                                                species_ordering["A"]], A_conc, places=places)
            self.assertAlmostEqual(state.ordered_concentrations[i *
                                                                len(concentrations.keys()) +
                                                                species_ordering["B"]], B_conc, places=places)
            self.assertAlmostEqual(state.ordered_concentrations[i *
                                                                len(concentrations.keys()) +
                                                                species_ordering["C"]], C_conc, places=places)
            self.assertAlmostEqual(state.ordered_concentrations[i *
                                                                len(concentrations.keys()) +
                                                                species_ordering["D"]], D_conc, places=places)
            self.assertAlmostEqual(state.ordered_concentrations[i *
                                                                len(concentrations.keys()) +
                                                                species_ordering["E"]], E_conc, places=places)
            self.assertAlmostEqual(state.ordered_concentrations[i *
                                                                len(concentrations.keys()) +
                                                                species_ordering["F"]], F_conc, places=places)

        curr_time += time_step


def TestVectorMultipleGridCell(self, solver, state, num_grid_cells, time_step, places=5):
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
    conditions = state.conditions
    for i in range(num_grid_cells):
        conditions[i].temperature = 275.0 + random.uniform(-10.0, 10.0)
        conditions[i].pressure = 101253.3 + random.uniform(-500.0, 500.0)
        conditions[i].air_density = conditions[i].pressure / (8.31446261815324 * conditions[i].temperature)
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

    rates_ordering = musica.user_defined_reaction_rates(solver, state)
    species_ordering = musica.species_ordering(solver, state)

    updated_ordered_rate_constants = (
        len(rate_constants.keys()) * num_grid_cells) * [0.0]
    for i in range(num_grid_cells):
        for key, value in rate_constants.items():
            updated_ordered_rate_constants[i +
                                           rates_ordering[key] * num_grid_cells] = value[i]
    state.ordered_rate_constants = updated_ordered_rate_constants

    update_ordered_concentrations = (
        len(concentrations.keys()) * num_grid_cells) * [0.0]
    for i in range(num_grid_cells):
        for key, value in concentrations.items():
            update_ordered_concentrations[i +
                                          species_ordering[key] * num_grid_cells] = value[i]
    state.ordered_concentrations = update_ordered_concentrations

    initial_concentrations = state.ordered_concentrations

    time_step = 1
    sim_length = 100

    curr_time = time_step
    initial_A = num_grid_cells * [0.0]
    initial_C = num_grid_cells * [0.0]
    initial_D = num_grid_cells * [0.0]
    initial_F = num_grid_cells * [0.0]
    for i in range(num_grid_cells):
        initial_A[i] = initial_concentrations[i + species_ordering["A"] * num_grid_cells]
        initial_C[i] = initial_concentrations[i + species_ordering["C"] * num_grid_cells]
        initial_D[i] = initial_concentrations[i + species_ordering["D"] * num_grid_cells]
        initial_F[i] = initial_concentrations[i + species_ordering["F"] * num_grid_cells]

    k1 = num_grid_cells * [0.0]
    k2 = num_grid_cells * [0.0]
    k3 = num_grid_cells * [0.0]
    k4 = num_grid_cells * [0.0]
    for i in range(num_grid_cells):
        k1[i] = state.ordered_rate_constants[i + rates_ordering["USER.reaction 1"] * num_grid_cells]
        k2[i] = state.ordered_rate_constants[i + rates_ordering["USER.reaction 2"] * num_grid_cells]
        k3[i] = 0.004 * np.exp(50.0 / conditions[i].temperature)
        k4[i] = 0.012 * np.exp(75.0 / conditions[i].temperature) * \
            (conditions[i].temperature / 50.0)**(-2) * (1.0 + 1.0e-6 * conditions[i].pressure)

    while curr_time <= sim_length:
        musica.micm_solve(
            solver,
            state,
            time_step)

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

            self.assertAlmostEqual(
                state.ordered_concentrations[i + species_ordering["A"] * num_grid_cells], A_conc, places=places)
            self.assertAlmostEqual(
                state.ordered_concentrations[i + species_ordering["B"] * num_grid_cells], B_conc, places=places)
            self.assertAlmostEqual(
                state.ordered_concentrations[i + species_ordering["C"] * num_grid_cells], C_conc, places=places)
            self.assertAlmostEqual(
                state.ordered_concentrations[i + species_ordering["D"] * num_grid_cells], D_conc, places=places)
            self.assertAlmostEqual(
                state.ordered_concentrations[i + species_ordering["E"] * num_grid_cells], E_conc, places=places)
            self.assertAlmostEqual(
                state.ordered_concentrations[i + species_ordering["F"] * num_grid_cells], F_conc, places=places)

        curr_time += time_step


class TestAnalyticalRosenbrockMultipleGridCells(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock,
            4)
        state = musica.create_state(solver)
        # The number of grid cells must equal the MICM matrix vector dimension
        TestVectorMultipleGridCell(self, solver, state, 4, 200.0, 5)


class TestAnalyticalStandardRosenbrockMultipleGridCells(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock_standard_order,
            3)
        state = musica.create_state(solver)
        TestStandardMultipleGridCell(self, solver, state, 3, 200.0, 5)


class TestAnalyticalBackwardEulerMultipleGridCells(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.backward_euler,
            4)
        state = musica.create_state(solver)
        # The number of grid cells must equal the MICM matrix vector dimension
        TestVectorMultipleGridCell(self, solver, state, 4, 10.0, places=2)


class TestAnalyticalStandardBackwardEulerMultipleGridCells(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.backward_euler_standard_order,
            3)
        state = musica.create_state(solver)
        TestStandardMultipleGridCell(self, solver, state, 3, 10.0, places=2)


if __name__ == '__main__':
    unittest.main()
