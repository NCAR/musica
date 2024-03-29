import unittest
import musica

class TestChapman(unittest.TestCase):
    def test_micm_solve(self):
        time_step = 200.0
        temperature = 272.5
        pressure = 101253.3
        concentrations = [0.75, 0.4, 0.8, 0.01, 0.02]

        solver = musica.create_micm("configs/chapman")  
        musica.micm_solve(solver, time_step, temperature, pressure, concentrations)

        self.assertEqual(concentrations[0], 0.75)
        self.assertNotEqual(concentrations[1], 0.4)
        self.assertNotEqual(concentrations[2], 0.8)
        self.assertNotEqual(concentrations[3], 0.01)
        self.assertNotEqual(concentrations[4], 0.02)

if __name__ == '__main__':
    unittest.main()