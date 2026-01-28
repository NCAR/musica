import musica
from musica.micm.solver_result import SolverState
from musica.mechanism_configuration import Parser
import pandas as pd
import xarray as xr
import matplotlib.pyplot as plt

path = 'configs/v1/ts1/ts1.json'

parser = Parser()
mechanism = parser.parse(path)

solver = musica.MICM(mechanism=mechanism,
                     solver_type=musica.SolverType.rosenbrock_standard_order)
num_grid_cells = 1
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
                         sep=',', names=['parameter', 'value1', 'value2'],
                         dtype={'parameter': str, 'value1': float, 'value2': float})

# grab the surface reactions, anything prefixed with SURF.
surface_reactions = conditions[conditions['parameter'].str.contains('SURF')]

# grab the initial concentrations, anything prefixed with CONC.
initial_concentrations = conditions[conditions['parameter'].str.contains(
    'CONC')]
# remove CONC. from the parameter names
initial_concentrations.loc[:, 'parameter'] = initial_concentrations.loc[:, 'parameter'].str.replace('CONC.', '')

# grab the environmental conditions, anything prefixed with ENV.
environmental_conditions = conditions[conditions['parameter'].str.contains('ENV')]

# grab the user defined conditions, anything prefixed with USER.
user_defined_conditions = conditions[conditions['parameter'].str.contains('USER')]

# make sure the length of all the subsets matches the total length of conditions
assert len(surface_reactions) + len(initial_concentrations) + \
    len(environmental_conditions) + \
    len(user_defined_conditions) == len(conditions)

# Set initial concentrations
concentration_dict = {}
for _, row in initial_concentrations.iterrows():
    concentration_dict[row['parameter']] = [row['value1']]

# Set user-defined rate parameters
user_defined_dict = {}
for _, row in user_defined_conditions.iterrows():
    user_defined_dict[row['parameter']] = [row['value1']]

# Set surface reaction parameters
for _, row in surface_reactions.iterrows():
    user_defined_dict[f"{row['parameter']}.effective radius [m]"] = [row['value1']]
    user_defined_dict[f"{row['parameter']}.particle number concentration [# m-3]"] = [row['value2']]

# Set environmental conditions (temperature and pressure)
temperature = environmental_conditions[environmental_conditions['parameter'] == 'ENV.temperature'].iloc[0]['value1']
pressure = environmental_conditions[environmental_conditions['parameter'] == 'ENV.pressure'].iloc[0]['value1']

# Set all conditions on the state
state.set_conditions([temperature], [pressure])
state.set_concentrations(concentration_dict)
state.set_user_defined_rate_parameters(user_defined_dict)

# setup and run the box model simulation
times = [0]
concentrations = [state.get_concentrations()]
time_step = 30  # seconds
simulation_length = 1 * 60 * 60 # 1 hour in seconds
current_time = 0
last_printed_percent = -5  # Track last printed percentage

while current_time < simulation_length:
    elapsed = 0
    while elapsed < time_step:
      result = solver.solve(state, time_step)
      elapsed += result.stats.final_time
      current_time += result.stats.final_time
      if result.state != SolverState.Converged:
        print(f"Solver state: {result.state}, time: {current_time}")
    
    # Print progress every 5%
    current_percent = (current_time / simulation_length) * 100
    if int(current_percent // 5) * 5 > last_printed_percent:
        last_printed_percent = int(current_percent // 5) * 5
        print(f"Simulation progress: {last_printed_percent}%")
      
    times.append(current_time)
    concentrations.append(state.get_concentrations())

conditions = state.get_conditions()

data_vars = {
    "temperature": (["grid_cell"], conditions["temperature"]),
    "pressure": (["grid_cell"], conditions["pressure"]),
    "air density": (["grid_cell"], conditions["air_density"]),
}

# Collect user-defined rate parameters as a single array
user_params = state.get_user_defined_rate_parameters()
user_param_names = list(user_params.keys())
user_param_values = [user_params[name] for name in user_param_names]

if user_param_names:
    data_vars["user_defined_rate_parameters"] = (["user_parameter", "grid_cell"], user_param_values)

species_ordering = state.get_species_ordering()
for species, _ in species_ordering.items():
    data = [concentrations[time_idx][species]
            for time_idx in range(len(concentrations))]
    data_vars[species] = (["time", "grid_cell"], data)

coords = {
    "time": times,
    "grid_cell": range(num_grid_cells),
}

if user_param_names:
    coords["user_parameter"] = user_param_names

ds = xr.Dataset(data_vars, coords=coords)
ds.to_netcdf('ts1_box_model.nc')

fig, ax = plt.subplots(2, 2, figsize=(12, 8))

# Convert time from seconds to hours for better readability
time_hours = ds['time'] / 3600

# Plot each species
ax[0, 0].plot(time_hours, ds['BEPOMUC'].isel(grid_cell=0))
ax[0, 1].plot(time_hours, ds['C6H5OOH'].isel(grid_cell=0))
ax[1, 0].plot(time_hours, ds['BR'].isel(grid_cell=0))
ax[1, 1].plot(time_hours, ds['CL'].isel(grid_cell=0))

for _ax in ax.flat:
    _ax.grid(True, alpha=0.5)
    _ax.spines[:].set_visible(False)
    _ax.tick_params(width=0)
    _ax.set_xlim(0, simulation_length / 3600)
    _ax.set_ylim(0, None)
    _ax.set_ylabel('Concentration [mol m-3]')
    _ax.set_xlabel('Time [hours]')

ax[0, 0].set_title('BEPOMUC')
ax[0, 1].set_title('C6H5OOH')
ax[1, 0].set_title('BR')
ax[1, 1].set_title('CL')

fig.tight_layout()
fig.savefig('ts1_box_model.png', dpi=300)
