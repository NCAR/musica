import unittest
import numpy as np
import musica
import math


class TestAnalyticalSimulation(unittest.TestCase):
    def test_simulation(self):
        num_grid_cells = 1
        time_step = 200.0
        temperature = 272.5
        pressure = 101253.3
        GAS_CONSTANT = 8.31446261815324
        air_density = pressure / (GAS_CONSTANT * temperature)

        solver = musica.create_solver(
            "configs/analytical",
            musica.micmsolver.rosenbrock,
            num_grid_cells)
        
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
            k4 = 0.012 * np.exp(75.0 / temperature) * (temperature / 50.0)**(-2) * (1.0 + 1.0e-6 * pressure)
            A_conc = initial_A * np.exp(-(k3) * curr_time)
            B_conc = initial_A * \
                (k3 / (k4 - k3)) * (np.exp(-k3 * curr_time) - np.exp(-k4 * curr_time))
            C_conc = initial_C + initial_A * \
                (1.0 + (k3 * np.exp(-k4 * curr_time) - k4 * np.exp(-k3 * curr_time)) / (k4 - k3))
            D_conc = initial_D * np.exp(-(k1) * curr_time)
            E_conc = initial_D * \
                (k1 / (k2 - k1)) * (np.exp(-k1 * curr_time) - np.exp(-k2 * curr_time))
            F_conc = initial_F + initial_D * \
                (1.0 + (k1 * np.exp(-k2 * curr_time) - k2 * np.exp(-k1 * curr_time)) / (k2 - k1))

            self.assertAlmostEqual(ordered_concentrations[species_ordering["A"]], A_conc, places=5)
            self.assertAlmostEqual(ordered_concentrations[species_ordering["B"]], B_conc, places=5)
            self.assertAlmostEqual(ordered_concentrations[species_ordering["C"]], C_conc, places=5)
            self.assertAlmostEqual(ordered_concentrations[species_ordering["D"]], D_conc, places=5)
            self.assertAlmostEqual(ordered_concentrations[species_ordering["E"]], E_conc, places=5)
            self.assertAlmostEqual(ordered_concentrations[species_ordering["F"]], F_conc, places=5)

            curr_time += time_step


if __name__ == '__main__':
    unittest.main()
