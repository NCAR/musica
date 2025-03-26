import unittest
import musica


class TestChapman(unittest.TestCase):
    def test_micm_solve(self):
        num_grid_cells = 1

        solver = musica.create_solver(
            "configs/chapman",
            musica.micmsolver.rosenbrock_standard_order,
            num_grid_cells)
        state = musica.create_state(solver)

        time_step = 200.0
        conditions = state.conditions
        condition = conditions[0]
        condition.temperature = 272.5
        condition.pressure = 101253.3
        GAS_CONSTANT = 8.31446261815324
        condition.air_density = condition.pressure / (GAS_CONSTANT * condition.temperature)

        rate_constant_ordering = musica.user_defined_reaction_rates(solver, state)
        species_ordering = musica.species_ordering(solver, state)

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

        updated_ordered_rate_constants = len(rate_constants.keys()) * [0.0]
        for key, value in rate_constants.items():
            updated_ordered_rate_constants[rate_constant_ordering[key]] = value
        state.ordered_rate_constants = updated_ordered_rate_constants

        update_ordered_concentrations = len(concentrations.keys()) * [0.0]
        for key, value in concentrations.items():
            update_ordered_concentrations[species_ordering[key]] = value
        state.ordered_concentrations = update_ordered_concentrations

        musica.micm_solve(
            solver,
            state,
            time_step)

        self.assertAlmostEqual(
            state.ordered_concentrations[species_ordering["O2"]], 0.75, places=5)
        self.assertGreater(state.ordered_concentrations[species_ordering["O"]], 0.0)
        self.assertGreater(
            state.ordered_concentrations[species_ordering["O1D"]], 0.0)
        self.assertNotEqual(
            state.ordered_concentrations[species_ordering["O3"]], 0.0000081)


if __name__ == '__main__':
    unittest.main()
