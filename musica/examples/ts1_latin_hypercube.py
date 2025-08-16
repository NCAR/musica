import musica
from musica.mechanism_configuration import Parser
import pandas as pd
from scipy.stats import qmc
import xarray as xr
import matplotlib.pyplot as plt

path = 'configs/v1/ts1/ts1.json'

parser = Parser()
mechanism = parser.parse(path)

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
initial_concentrations = conditions[conditions['parameter'].str.contains(
    'CONC')]
# remove CONC. from the parameter names
initial_concentrations.loc[:, 'parameter'] = initial_concentrations.loc[:,
                                                                        'parameter'].str.replace('CONC.', '')

# grab the environmental conditions, anything prefixed with ENV.
environmental_conditions = conditions[conditions['parameter'].str.contains(
    'ENV')]

# grab the user defined conditions, anything prefixed with USER.
user_defined_conditions = conditions[conditions['parameter'].str.contains(
    'USER')]

# make sure the length of all the subsets matches the total length of conditions
assert len(surface_reactions) + len(initial_concentrations) + \
    len(environmental_conditions) + \
    len(user_defined_conditions) == len(conditions)

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
    lower_bounds.append(float(concentration) *
                        (1 - concentration_perturbation))
    upper_bounds.append(float(concentration) *
                        (1 + concentration_perturbation))

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
    lower_bounds.append(float(environmental) *
                        (1 - environmental_perturbation))
    upper_bounds.append(float(environmental) *
                        (1 + environmental_perturbation))

# Now we can create the Latin Hypercube samples
sampler = qmc.LatinHypercube(d=len(lower_bounds))
# we will make one sample for each grid cell and solve all of the parameter combinations at once
sample = sampler.random(n=num_grid_cells)
scaled_sample = qmc.scale(sample, lower_bounds, upper_bounds)

# Now we need to extract the sampled values and set them on the state
# The order matches how we built the bounds arrays above

# Extract samples for each parameter type
current_index = 0

# 1. Extract concentration samples
concentration_samples = {}
species_ordering = state.get_species_ordering()
for _, concentration_name in enumerate(initial_concentrations['parameter']):
    concentration_samples[concentration_name] = scaled_sample[:, current_index]
    current_index += 1

# 2. Extract user-defined rate samples
user_defined_samples = {}
user_defined_ordering = state.get_user_defined_rate_parameters_ordering()
for _, user_param in enumerate(user_defined_conditions['parameter']):
    # Remove USER. prefix to match the expected parameter name
    user_defined_samples[user_param] = scaled_sample[:, current_index]
    current_index += 1

# 3. Extract surface reaction samples (skip for now as they need special handling)
for _, row in surface_reactions.iterrows():
    # Skip effective radius and particle concentration for now
    user_defined_samples[f"{row['parameter']}.effective radius [m]"] = scaled_sample[:, current_index]
    current_index += 1
    user_defined_samples[f"{row['parameter']}.particle number concentration [# m-3]"] = scaled_sample[:, current_index]
    current_index += 1

# 4. Extract environmental samples
temperature_samples = scaled_sample[:, current_index]
current_index += 1
pressure_samples = scaled_sample[:, current_index]
current_index += 1

# Now set all of the samples across all grid cells
state.set_conditions(temperature_samples, pressure_samples)
state.set_concentrations(concentration_samples)
state.set_user_defined_rate_parameters(user_defined_samples)

# setup and run the box model simulation
concentrations = [state.get_concentrations()]
times = []
time_step = 30  # seconds
simulation_length = 1 * 60 * 60  # 1 hour in seconds
current_time = 0

while current_time < simulation_length:
    times.append(current_time)
    solver.solve(state, time_step)
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
ds.to_netcdf('ts1_latin_hypercube.nc')

# plot 5 grid cells for O3, OH, NO, NO2
fig, ax = plt.subplots(2, 2, figsize=(12, 8))

# Convert time from seconds to hours for better readability
time_hours = ds['time'] / 3600

# Plot each species with 5 lines (one for each grid cell)
for i in range(5):
    ax[0, 0].plot(time_hours, ds['O3'].isel(
        grid_cell=i), label=f'Grid Cell {i}')
    ax[0, 1].plot(time_hours, ds['OH'].isel(
        grid_cell=i), label=f'Grid Cell {i}')
    ax[1, 0].plot(time_hours, ds['NO'].isel(
        grid_cell=i), label=f'Grid Cell {i}')
    ax[1, 1].plot(time_hours, ds['NO2'].isel(
        grid_cell=i), label=f'Grid Cell {i}')

for _ax in ax.flat:
    _ax.grid(True, alpha=0.5)
    _ax.spines[:].set_visible(False)
    _ax.tick_params(width=0)
    _ax.set_xlim(0, simulation_length / 3600)  # Set x-axis limit to simulation length in hours
    _ax.set_ylim(0, None)
    _ax.set_ylabel('Concentration [mol m-3]')
    _ax.set_xlabel('Time [hours]')
    _ax.legend()

ax[0, 0].set_title('O3')
ax[0, 1].set_title('OH')
ax[1, 0].set_title('NO')
ax[1, 1].set_title('NO2')

fig.tight_layout()
fig.savefig('ts1_latin_hypercube_OH_O3_NO_NO2.png', dpi=300)


# Plot minimum and maximum values for O3, OH, NO, NO2 across all grid cells
fig, ax = plt.subplots(2, 2, figsize=(12, 8))
for i, species in enumerate(['O3', 'OH', 'NO', 'NO2']):
    ax.flat[i].plot(time_hours, ds[species].min(dim='grid_cell'), label='Min')
    ax.flat[i].plot(time_hours, ds[species].max(dim='grid_cell'), label='Max')
    ax.flat[i].set_title(species)
    ax.flat[i].set_ylabel('Concentration [mol m-3]')
    ax.flat[i].set_xlabel('Time [hours]')
    ax.flat[i].legend()
    ax.flat[i].grid(True, alpha=0.5)
    ax.flat[i].spines[:].set_visible(False)
    ax.flat[i].tick_params(width=0)
    # ax.flat[i].set_yscale('log')

fig.tight_layout()
fig.savefig('ts1_latin_hypercube_OH_O3_NO_NO2_min_max.png', dpi=300)

fig, ax = plt.subplots(3, 1, figsize=(10, 8))

# Plot temperature, pressure, and air density
ax[0].plot(ds['grid_cell'], ds['temperature'])
ax[0].set_title('Temperature')
ax[0].set_ylabel('Temperature [K]')
ax[0].set_xlabel('Grid Cell')
ax[1].plot(ds['grid_cell'], ds['pressure'])
ax[1].set_title('Pressure')
ax[1].set_ylabel('Pressure [Pa]')
ax[1].set_xlabel('Grid Cell')
ax[2].plot(ds['grid_cell'], ds['air density'])
ax[2].set_title('Air Density')
ax[2].set_ylabel('Air Density [kg m-3]')
ax[2].set_xlabel('Grid Cell')

fig.tight_layout()
fig.savefig('ts1_latin_hypercube_conditions.png', dpi=300)