import musica
import os
from musica.mechanism_configuration import Parser, MechanismSerializer
import pandas as pd

# The path to the TS1 configuration file
path = 'configs/v1/ts1/ts1.json'

parser = Parser()
if os.path.exists(path):
  mechanism = parser.parse(path)
else:
  mechanism = parser.parse_and_convert_v0('/Users/kshores/Documents/musica/configs/v0/TS1/config.json')
  MechanismSerializer.serialize(mechanism, path)

solver = musica.MICM(mechanism = mechanism, solver_type = musica.SolverType.rosenbrock_standard_order)
num_grid_cells = 100
state = solver.create_state(num_grid_cells)

print(state.get_species_ordering())
conditions = pd.read_csv('configs/v1/ts1/intial_conditions.csv')
# remove CONC. prefix from first column
conditions.columns = [col.replace('CONC.', '') for col in conditions.columns]
# remove ENV. prefix from first column
conditions.columns = [col.replace('ENV.', '') for col in conditions.columns]

print(conditions)