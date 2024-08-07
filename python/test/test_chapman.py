import unittest
import musica


class TestChapman(unittest.TestCase):
    def test_micm_solve(self):
        num_grid_cells = 1
        time_step = 200.0
        temperature = 272.5
        pressure = 101253.3
        GAS_CONSTANT = 8.31446261815324
        air_density = pressure / (GAS_CONSTANT * temperature)
        concentrations = [0.4, 0.8, 0.01, 0.02]

        solver = musica.create_solver(
            "configs/chapman",
            musica.micmsolver.rosenbrock,
            num_grid_cells)
        rate_constant_ordering = musica.user_defined_reaction_rates(solver)
        ordering = musica.species_ordering(solver)

        rate_constants = {
            "PHOTO.R1": 2.42e-17,
            "PHOTO.R3": 1.15e-5,
            "PHOTO.R5": 6.61e-9
        }

        ordered_rate_constants = len(rate_constants.keys()) * [0.0]

        for key, value in rate_constants.items():
            ordered_rate_constants[rate_constant_ordering[key]] = value

        musica.micm_solve(
            solver,
            time_step,
            temperature,
            pressure,
            air_density,
            concentrations,
            ordered_rate_constants)

        self.assertEqual(
            ordering, {
                'O': 1, 'O1D': 0, 'O2': 2, 'O3': 3})
        self.assertEqual(
            rate_constant_ordering, {
                'PHOTO.R1': 0, 'PHOTO.R3': 1, 'PHOTO.R5': 2})

        self.assertNotEqual(concentrations[0], 0.4)
        self.assertNotEqual(concentrations[1], 0.8)
        self.assertNotEqual(concentrations[2], 0.01)
        self.assertNotEqual(concentrations[3], 0.02)


if __name__ == '__main__':
    unittest.main()
