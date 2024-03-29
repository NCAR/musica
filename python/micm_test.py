import sys

import musica

#tests python micm package
time_step = 200.0
temperature = 272.5
pressure = 101253.3
num_concentrations = 5
concentrations = [0.75, 0.4, 0.8, 0.01, 0.02]

print(concentrations)

solver = musica.create_micm("configs/chapman")  
musica.micm_solve(solver, time_step, temperature, pressure, concentrations)

print(concentrations)

assert concentrations[0] == 0.75
assert concentrations[1] != 0.4
assert concentrations[2] != 0.8
assert concentrations[3] != 0.01
assert concentrations[4] != 0.02
