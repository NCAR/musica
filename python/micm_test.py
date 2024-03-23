import sys
sys.path.append('/Users/alexgarza/Documents/CSCE482/musica/build')

import musica

#tests python micm package
time_step = 200.0
temperature = 272.5
pressure = 101253.3
num_concentrations = 5
concentrations = [0.75, 0.4, 0.8, 0.01, 0.02]


solver = musica.create_micm("/Users/alexgarza/Documents/CSCE482/musica/src/micm/configs/chapman")  
musica.micm_solve(solver, time_step, temperature, pressure, concentrations)

assert concentrations[0] == 0.75
assert concentrations[1] != 0.4
assert concentrations[2] != 0.8
assert concentrations[3] != 0.01
assert concentrations[4] != 0.02




