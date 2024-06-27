import unittest
import numpy as np
import musica


class TestAnalyticalSimulation(unittest.TestCase):
    def test_simulation(self):
        time_step = 200.0
        temperature = 272.5
        pressure = 101253.3
        GAS_CONSTANT = 8.31446261815324
        air_density = pressure / (GAS_CONSTANT * temperature)

        solver = musica.create_solver("configs/analytical")

        rates = musica.user_defined_reaction_rates(solver)
        ordering = musica.species_ordering(solver)

        self.assertEqual(rates, {})
        self.assertEqual(ordering['A'], 0)
        self.assertEqual(ordering['B'], 1)
        self.assertEqual(ordering['C'], 2)

        # Initalizes concentrations
        initial_concentrations = [1, 0, 0]

        model_concentrations = [initial_concentrations[:]]
        analytical_concentrations = [initial_concentrations[:]]

        k1 = 4.0e-3 * np.exp(50 / temperature)
        k2 = 1.2e-4 * np.exp(75 / temperature) * \
            (temperature / 50)**7 * (1.0 + 0.5 * pressure)

        time_step = 1
        sim_length = 100

        curr_time = time_step

        idx_A = 0
        idx_B = 1
        idx_C = 2

        concentrations = initial_concentrations

        # Gets analytical concentrations
        while curr_time <= sim_length:
            musica.micm_solve(
                solver,
                time_step,
                temperature,
                pressure,
                air_density,
                concentrations,
                None)
            model_concentrations.append(concentrations[:])

            initial_A = analytical_concentrations[0][idx_A]
            A_conc = initial_A * np.exp(-(k1) * curr_time)
            B_conc = initial_A * \
                (k1 / (k2 - k1)) * (np.exp(-k1 * curr_time) - np.exp(-k2 * curr_time))
            C_conc = initial_A * \
                (1.0 + (k1 * np.exp(-k2 * curr_time) - k2 * np.exp(-k1 * curr_time)) / (k2 - k1))

            analytical_concentrations.append([A_conc, B_conc, C_conc])
            curr_time += time_step

        for i in range(len(model_concentrations)):
            self.assertAlmostEqual(
                model_concentrations[i][idx_A],
                analytical_concentrations[i][idx_A],
                places=8)
            self.assertAlmostEqual(
                model_concentrations[i][idx_B],
                analytical_concentrations[i][idx_B],
                places=8)
            self.assertAlmostEqual(
                model_concentrations[i][idx_C],
                analytical_concentrations[i][idx_C],
                places=8)


if __name__ == '__main__':
    unittest.main()
