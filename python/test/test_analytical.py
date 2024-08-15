import unittest
import numpy as np
import musica
import random


def TestSingleGridCell(self, solver):
    num_grid_cells = 1
    time_step = 200.0
    temperature = 272.5
    pressure = 101253.3
    GAS_CONSTANT = 8.31446261815324
    air_density = pressure / (GAS_CONSTANT * temperature)

    rates = musica.user_defined_reaction_rates(solver)
    species_ordering = musica.species_ordering(solver)

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

    ordered_rate_constants = len(rate_constants.keys()) * [0.0]

    for key, value in rate_constants.items():
        ordered_rate_constants[rates[key]] = value

    ordered_concentrations = len(concentrations.keys()) * [0.0]

    for key, value in concentrations.items():
        ordered_concentrations[species_ordering[key]] = value

    initial_concentrations = ordered_concentrations

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
            time_step,
            temperature,
            pressure,
            air_density,
            ordered_concentrations,
            ordered_rate_constants)

        k1 = ordered_rate_constants[rates["USER.reaction 1"]]
        k2 = ordered_rate_constants[rates["USER.reaction 2"]]
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

        self.assertAlmostEqual(
            ordered_concentrations[species_ordering["A"]], A_conc, places=5)
        self.assertAlmostEqual(
            ordered_concentrations[species_ordering["B"]], B_conc, places=5)
        self.assertAlmostEqual(
            ordered_concentrations[species_ordering["C"]], C_conc, places=5)
        self.assertAlmostEqual(
            ordered_concentrations[species_ordering["D"]], D_conc, places=5)
        self.assertAlmostEqual(
            ordered_concentrations[species_ordering["E"]], E_conc, places=5)
        self.assertAlmostEqual(
            ordered_concentrations[species_ordering["F"]], F_conc, places=5)

        curr_time += time_step


class TestAnalyticalRosenbrock(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock,
            1)
        TestSingleGridCell(self, solver)


class TestAnalyicalStandardRosenbrock(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock_standard_order,
            1)
        TestSingleGridCell(self, solver)


def TestMultipleGridCell(self, solver, num_grid_cells):
    time_step = 200.0
    temperatures = []
    pressures = []
    air_densities = []
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
    for i in range(num_grid_cells):
        temperatures.append(275.0 + random.uniform(-10.0, 10.0))
        pressures.append(101253.3 + random.uniform(-500.0, 500.0))
        air_densities.append(
            pressures[i] / (8.31446261815324 * temperatures[i]))
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

    rates_ordering = musica.user_defined_reaction_rates(solver)
    species_ordering = musica.species_ordering(solver)

    ordered_rate_constants = (
        len(rate_constants.keys()) * num_grid_cells) * [0.0]
    for i in range(num_grid_cells):
        for key, value in rate_constants.items():
            ordered_rate_constants[i *
                                   len(rate_constants.keys()) +
                                   rates_ordering[key]] = value[i]

    ordered_concentrations = (
        len(concentrations.keys()) * num_grid_cells) * [0.0]
    for i in range(num_grid_cells):
        for key, value in concentrations.items():
            ordered_concentrations[i *
                                   len(concentrations.keys()) +
                                   species_ordering[key]] = value[i]

    initial_concentrations = ordered_concentrations

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
        k1[i] = ordered_rate_constants[i *
                                       len(rate_constants.keys()) +
                                       rates_ordering["USER.reaction 1"]]
        k2[i] = ordered_rate_constants[i *
                                       len(rate_constants.keys()) +
                                       rates_ordering["USER.reaction 2"]]
        k3[i] = 0.004 * np.exp(50.0 / temperatures[i])
        k4[i] = 0.012 * np.exp(75.0 / temperatures[i]) * \
            (temperatures[i] / 50.0)**(-2) * (1.0 + 1.0e-6 * pressures[i])

    while curr_time <= sim_length:
        musica.micm_solve(
            solver,
            time_step,
            temperatures,
            pressures,
            air_densities,
            ordered_concentrations,
            ordered_rate_constants)

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
                ordered_concentrations[i * len(concentrations.keys()) + species_ordering["A"]], A_conc, places=5)
            self.assertAlmostEqual(
                ordered_concentrations[i * len(concentrations.keys()) + species_ordering["B"]], B_conc, places=5)
            self.assertAlmostEqual(
                ordered_concentrations[i * len(concentrations.keys()) + species_ordering["C"]], C_conc, places=5)
            self.assertAlmostEqual(
                ordered_concentrations[i * len(concentrations.keys()) + species_ordering["D"]], D_conc, places=5)
            self.assertAlmostEqual(
                ordered_concentrations[i * len(concentrations.keys()) + species_ordering["E"]], E_conc, places=5)
            self.assertAlmostEqual(
                ordered_concentrations[i * len(concentrations.keys()) + species_ordering["F"]], F_conc, places=5)

        curr_time += time_step


class TestAnalyticalRosenbrockMultipleGridCells(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock,
            3)
        TestMultipleGridCell(self, solver, 3)


class TestAnalyicalStandardRosenbrockMultipleGridCells(unittest.TestCase):
    def test_simulation(self):
        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock_standard_order,
            3)
        TestMultipleGridCell(self, solver, 3)


if __name__ == '__main__':
    unittest.main()
