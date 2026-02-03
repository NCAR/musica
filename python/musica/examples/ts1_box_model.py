import musica
from musica.micm.solver_result import SolverState
from musica.mechanism_configuration import Parser
from musica.utils import find_config_path
import pandas as pd
import xarray as xr
import matplotlib.pyplot as plt
from musica.tuvx import vTS1
import json
import ussa1976

# Load TS1
tuvx = vTS1.get_tuvx_calculator()
tuv_rates = tuvx.run(sza=0.0, earth_sun_distance=1.0)

# Also load its config file path so that we can get the alias mappings
tuv_path = find_config_path("tuvx", "ts1_tsmlt.json")
with open(tuv_path, 'r') as f:
    data = json.load(f)
alias_mappings = data.get('__CAM options', {}).get('aliasing', {}).get('pairs', {})

# index zero is 0 km, so we will skip that for box model runs
start = 1
# index 10 just happens to be 10 km
end = 10

photolysis_rate_constants = {}
for mapping in alias_mappings:
    label = mapping['to']
    scale = mapping.get("scale by", 1)
    tuv_label = mapping['from']
    rate = tuv_rates.sel(reaction=tuv_label).photolysis_rate_constants.values * scale
    photolysis_rate_constants[f'USER.{label}'] = rate[start:end]  # skip the first grid cell which is at 0 km

parser = Parser()
mechanism = parser.parse(find_config_path("v1", "ts1", "ts1.json"))

solver = musica.MICM(mechanism=mechanism,
                     solver_type=musica.SolverType.rosenbrock_standard_order)
num_grid_cells = tuv_rates.vertical_edge[start:end].size
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
conditions = pd.read_csv(find_config_path("v1", "ts1", "initial_conditions.csv"),
                         sep=',', names=['parameter', 'value1', 'value2'],
                         dtype={'parameter': str, 'value1': float, 'value2': float})

# grab the surface reactions, anything prefixed with SURF.
surface_reactions = conditions[conditions['parameter'].str.contains('SURF')]

# grab the initial concentrations, anything prefixed with CONC.
initial_concentrations = conditions[conditions['parameter'].str.contains('CONC')]
# remove CONC. from the parameter names
initial_concentrations.loc[:, 'parameter'] = initial_concentrations.loc[:,'parameter'].str.replace('CONC.', '', regex=False)

# grab the environmental conditions, anything prefixed with ENV.
environmental_conditions = conditions[conditions['parameter'].str.contains('ENV')]

# grab the user defined conditions, anything prefixed with USER.
# many of these will be overwritten by the TUV-x photolysis rates
user_defined_conditions = conditions[conditions['parameter'].str.contains('USER')]

# make sure the length of all the subsets matches the total length of conditions
assert len(surface_reactions) + len(initial_concentrations) + \
    len(environmental_conditions) + \
    len(user_defined_conditions) == len(conditions)

# Set initial concentrations
concentration_dict = {}
for _, row in initial_concentrations.iterrows():
    concentration_dict[row['parameter']] = [row['value1']] * num_grid_cells

# Set user-defined rate parameters
user_defined_dict = {}
for _, row in user_defined_conditions.iterrows():
    user_defined_dict[row['parameter']] = [row['value1']] * num_grid_cells

# Set surface reaction parameters
for _, row in surface_reactions.iterrows():
    user_defined_dict[f"{row['parameter']}.effective radius [m]"] = [row['value1']] * num_grid_cells
    user_defined_dict[f"{row['parameter']}.particle number concentration [# m-3]"] = [row['value2']] * num_grid_cells

# multiply by 1000 to convert from km to m
environmental_conditions = ussa1976.compute(z=tuv_rates.vertical_edge[start:end].data * 1000, variables=["t", "p"])
temperature = environmental_conditions['t'].values
pressure = environmental_conditions['p'].values

found_rates = sorted(photolysis_rate_constants.keys())
needed_rates = sorted([i for i in state.get_user_defined_rate_parameters() if 'USER.j' in i])

missing_rates = set(needed_rates) - set(found_rates)
print(f"Missing photolysis rates: {missing_rates}")

user_defined_dict.update(photolysis_rate_constants)

# Set all conditions on the state
state.set_conditions(temperature, pressure)
state.set_concentrations(concentration_dict)
state.set_user_defined_rate_parameters(user_defined_dict)

# setup and run the box model simulation
times = [0]
concentrations = [state.get_concentrations()]
time_step = 30  # seconds
simulation_length = 0.1 * 24 * 60 * 60  # 1/10 of a day in seconds
current_time = 0
last_printed_percent = -5  # Track last printed percentage


while current_time < simulation_length:
    elapsed = 0
    while elapsed < time_step:
        remaining_time = time_step - elapsed
        result = solver.solve(state, remaining_time)
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
ax[0, 0].plot(time_hours, ds['BEPOMUC'])
ax[0, 1].plot(time_hours, ds['C6H5OOH'])
ax[1, 0].plot(time_hours, ds['BR'])
ax[1, 1].plot(time_hours, ds['CL'])

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
