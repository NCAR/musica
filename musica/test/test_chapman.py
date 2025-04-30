import unittest
import musica


class TestChapman(unittest.TestCase):
    def test_micm_solve(self):
        num_grid_cells = 1

        solver = musica.MICM(
            config_path = "configs/chapman",
            solver_type = musica.SolverType.rosenbrock_standard_order,
            number_of_grid_cells = num_grid_cells)
        state = solver.create_state()

        time_step = 200.0
        temperature = 272.5
        pressure = 101253.3
        
        rate_constants = {
            "PHOTO.jO2": 2.42e-17,
            "PHOTO.jO3->O": 1.15e-5,
            "PHOTO.jO3->O1D": 6.61e-9
        }

        initial_concentrations = {
            "O2": 0.75,
            "O": 0.0,
            "O1D": 0.0,
            "O3": 0.0000081
        }

        state.set_conditions(temperatures = temperature, pressures = pressure)
        state.set_concentrations(initial_concentrations)
        state.set_user_defined_rate_parameters(rate_constants)

        solver.solve(state, time_step)
        concentrations = state.get_concentrations()

        self.assertAlmostEqual(concentrations["O2"][0], 0.75, places=5)
        self.assertGreater(concentrations["O"][0], 0.0)
        self.assertGreater(concentrations["O1D"][0], 0.0)
        self.assertNotEqual(concentrations["O3"][0], 0.0000081)


if __name__ == '__main__':
    unittest.main()
