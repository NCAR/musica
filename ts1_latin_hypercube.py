import musica
import os
from musica.mechanism_configuration import Parser, MechanismSerializer
import pandas as pd
from scipy.stats import qmc

# The path to the TS1 configuration file
path = 'configs/v1/ts1/ts1.json'

parser = Parser()
if os.path.exists(path):
    mechanism = parser.parse(path)
else:
    mechanism = parser.parse_and_convert_v0(
        '/Users/kshores/Documents/musica/configs/v0/TS1/config.json')
    MechanismSerializer.serialize(mechanism, path)

solver = musica.MICM(mechanism=mechanism,
                     solver_type=musica.SolverType.rosenbrock_standard_order)
num_grid_cells = 100
state = solver.create_state(num_grid_cells)

# The initial conditions file contains various parameters with their values.
# It has three columns: parameter, value1, and value2.
# The meaning of value1 and value2 depends on the parameter type:
#
# | Parameter Prefix | value1 Meaning                      | value2 Meaning                      |
# |------------------|-------------------------------------|-------------------------------------|
# | SURF             | Effective radius (meters)           | Particle number concentration (#/m3)|
# | CONC             | Initial concentration (mol m-3)     | Unused                              |
# | ENV              | Temperature (K) or Pressure (Pa)    | Unused                              |
# | USER             | User-defined parameter value        | Unused                              |
conditions = pd.read_csv('configs/v1/ts1/initial_conditions.csv',
             sep=',', names=['parameter', 'value1', 'value2'])

# grab the surface reactions, anything prefixed with SURF.
surface_reactions = conditions[conditions['parameter'].str.contains('SURF')]

# grab the initial concentrations, anything prefixed with CONC.
initial_concentrations = conditions[conditions['parameter'].str.contains('CONC')]
# remove CONC. from the parameter names
initial_concentrations.loc[:, 'parameter'] = initial_concentrations.loc[:, 'parameter'].str.replace('CONC.', '')

# grab the environmental conditions, anything prefixed with ENV.
environmental_conditions = conditions[conditions['parameter'].str.contains('ENV')]

# grab the user defined conditions, anything prefixed with USER.
user_defined_conditions = conditions[conditions['parameter'].str.contains('USER')]

# make sure the length of all the subsets matches the total length of conditions
assert len(surface_reactions) + len(initial_concentrations) + len(environmental_conditions) + len(user_defined_conditions) == len(conditions)

# To create a Latin Hypercube sample, we have to define the lower and upper bounds for each parameter
# We will use the initial conditions from the file and add a small perturbation to create a range
lower_bounds = []
upper_bounds = []

concentration_perturbation = 0.1
user_defined_perturbation = 0.05
surface_perturbation = 0.02
environmental_perturbation = 0.2

# Lets loop through each of our initial conditions arrays and create bounds
for concentration in initial_concentrations['value1']:
    lower_bounds.append(float(concentration) * (1 - concentration_perturbation))
    upper_bounds.append(float(concentration) * (1 + concentration_perturbation))

for user in user_defined_conditions['value1']:
    lower_bounds.append(float(user) * (1 - user_defined_perturbation))
    upper_bounds.append(float(user) * (1 + user_defined_perturbation))

for _, row in surface_reactions.iterrows():
  # effective radius
  lower_bounds.append(float(row['value1']) * (1 - surface_perturbation))
  upper_bounds.append(float(row['value1']) * (1 + surface_perturbation))
  # particle number concentration
  lower_bounds.append(float(row['value2']) * (1 - surface_perturbation))
  upper_bounds.append(float(row['value2']) * (1 + surface_perturbation))
  
for environmental in environmental_conditions['value1']:
    lower_bounds.append(float(environmental) * (1 - environmental_perturbation))
    upper_bounds.append(float(environmental) * (1 + environmental_perturbation))

# Now we can create the Latin Hypercube samples
sampler = qmc.LatinHypercube(d=len(lower_bounds))
# we will make one sample for each grid cell and solve all of the parameter combinations at once
sample = sampler.random(n=num_grid_cells)
scaled_sample = qmc.scale(sample, lower_bounds, upper_bounds)

# print(surface_reactions)
# print(initial_concentrations)
# print(environmental_conditions)
# print(photolysis_rates)
# print(user_defined_conditions)
# print(state.get_species_ordering())
print(state.get_user_defined_rate_parameters_ordering())