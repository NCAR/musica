import musica
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
concentrations = [state.get_concentrations()]
times = []
time_step = 30  # seconds
simulation_length = 1 * 60 # seconds
current_time = 0

iteration = 0

while current_time < simulation_length:
    if iteration > 100:
        break
    times.append(current_time)
    elapsed = 0
    while elapsed < time_step:
      result = solver.solve(state, time_step)
      elapsed += result.stats.final_time
      print(f"Solver state: {result.state}")
      iteration += 1
      if iteration > 100:
          break

    concentrations.append(state.get_concentrations())
    current_time += time_step

conditions = state.get_conditions()

data_vars = {
    "temperature": (["grid_cell"], conditions["temperature"]),
    "pressure": (["grid_cell"], conditions["pressure"]),
    "air density": (["grid_cell"], conditions["air_density"]),
}

for user_param, values in state.get_user_defined_rate_parameters().items():
    data_vars[user_param] = (["grid_cell"], values)

species_ordering = state.get_species_ordering()
for species, _ in species_ordering.items():
    data = [concentrations[time_idx][species]
            for time_idx in range(len(concentrations))]
    data_vars[species] = (["time", "grid_cell"], data)

ds = xr.Dataset(
    data_vars,
    coords={
        "time": times + [current_time],
        "grid_cell": range(num_grid_cells),
    },
)

print(ds)
ds.to_netcdf('ts1_box_model.nc')

# # plot O3, OH, NO, NO2
# fig, ax = plt.subplots(2, 2, figsize=(12, 8))

# # Convert time from seconds to hours for better readability
# time_hours = ds['time'] / 3600

# # Plot each species
# ax[0, 0].plot(time_hours, ds['BEPOMUC'].isel(grid_cell=0))
# ax[0, 1].plot(time_hours, ds['C6H5OOH'].isel(grid_cell=0))
# ax[1, 0].plot(time_hours, ds['CH3CO3'].isel(grid_cell=0))
# ax[1, 1].plot(time_hours, ds['CH3COCH3'].isel(grid_cell=0))

# for _ax in ax.flat:
#     _ax.grid(True, alpha=0.5)
#     _ax.spines[:].set_visible(False)
#     _ax.tick_params(width=0)
#     _ax.set_xlim(0, simulation_length / 3600)
#     _ax.set_ylim(0, None)
#     _ax.set_ylabel('Concentration [mol m-3]')
#     _ax.set_xlabel('Time [hours]')

# ax[0, 0].set_title('BEPOMUC')
# ax[0, 1].set_title('C6H5OOH')
# ax[1, 0].set_title('CH3CO3')
# ax[1, 1].set_title('CH3COCH3')

# fig.tight_layout()
# fig.savefig('ts1_box_model.png', dpi=300)
