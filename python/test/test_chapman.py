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

        solver = musica.create_solver(
            "configs/chapman",
            musica.micmsolver.rosenbrock,
            num_grid_cells)

        rate_constant_ordering = musica.user_defined_reaction_rates(solver)
        species_ordering = musica.species_ordering(solver)

        rate_constants = {
            "PHOTO.jO2": 2.42e-17,
            "PHOTO.jO3->O": 1.15e-5,
            "PHOTO.jO3->O1D": 6.61e-9
        }

        concentrations = {
            "O2": 0.75,
            "O": 0.0,
            "O1D": 0.0,
            "O3": 0.0000081
        }

        ordered_rate_constants = len(rate_constants.keys()) * [0.0]

        for key, value in rate_constants.items():
            ordered_rate_constants[rate_constant_ordering[key]] = value

        ordered_concentrations = len(concentrations.keys()) * [0.0]

        for key, value in concentrations.items():
            ordered_concentrations[species_ordering[key]] = value

        musica.micm_solve(
            solver,
            time_step,
            temperature,
            pressure,
            air_density,
            ordered_concentrations,
            ordered_rate_constants)

        self.assertAlmostEqual(
            ordered_concentrations[species_ordering["O2"]], 0.75, places=5)
        self.assertGreater(ordered_concentrations[species_ordering["O"]], 0.0)
        self.assertGreater(
            ordered_concentrations[species_ordering["O1D"]], 0.0)
        self.assertNotEqual(
            ordered_concentrations[species_ordering["O3"]], 0.0000081)


if __name__ == '__main__':
    unittest.main()
