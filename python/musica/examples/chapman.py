import musica
from musica.constants import GAS_CONSTANT
from musica.micm.solver_result import SolverState
from musica.mechanism_configuration import Parser
from musica.utils import find_config_path
import xarray as xr
import matplotlib.pyplot as plt
from musica.tuvx import vTS1
import json
import ussa1976
import pvlib
import numpy as np
from datetime import datetime, time, timedelta
from zoneinfo import ZoneInfo

SECONDS_PER_HOUR = 3600


# The location this example is set up for
boulder = (40.01879858223568, -105.27492413846649)
boulder_tz = ZoneInfo("America/Denver")

# Load TS1
tuvx = vTS1.get_tuvx_calculator()

# Also load its config file path so that we can get the alias mappings
tuv_path = find_config_path("tuvx", "ts1_tsmlt.json")
with open(tuv_path, 'r') as f:
    data = json.load(f)
alias_mappings = data.get('__CAM options', {}).get('aliasing', {}).get('pairs', {})

def get_tuv_rates(utc_time, num_grid_cells, start=1):
  """Calculate photolysis rate constants from TUV-x.

  Args:
    utc_time: The UTC time for which to calculate the TUV-x rates.
    num_grid_cells: Number of grid cells to retrieve rates for.
    start: Starting grid cell index (default is 1 to skip ground level).
  """
  lat, lon = boulder
  # get solar position
  # https://pvlib-python.readthedocs.io/en/stable/reference/generated/pvlib.solarposition.get_solarposition.html
  # https://pvlib-python.readthedocs.io/en/stable/reference/generated/pvlib.solarposition.spa_python.html#pvlib.solarposition.spa_python
  solpos = pvlib.solarposition.get_solarposition(time=utc_time, latitude=lat, longitude=lon)
  sza = solpos['zenith'].item()

  tuv_rates = tuvx.run(sza=np.deg2rad(sza), earth_sun_distance=1.0)
  end = start + num_grid_cells

  # map TUV-x rates to MICM user-defined photolysis rates
  photolysis_rate_constants = {}
  for mapping in alias_mappings:
      label = mapping['to']
      scale = mapping.get("scale by", 1)
      tuv_label = mapping['from']
      rate = tuv_rates.sel(reaction=tuv_label).photolysis_rate_constants.values * scale
      photolysis_rate_constants[f'PHOTO.{label}'] = rate[start:end]  # skip the first grid cell which is at 0 km
  
  return photolysis_rate_constants, tuv_rates

def get_solver(num_grid_cells, config_path=find_config_path("v1", "chapman", "config.json")):
  """Create a solver and state for the Chapman mechanism.

  Args:
    num_grid_cells: Number of grid cells in the solver state.
    config_path: Mechanism configuration file path.
  """
  path = config_path
  parser = Parser()
  mechanism = parser.parse(path)

  solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)
  state = solver.create_state(num_grid_cells)

  return solver, state

def get_conditions(vertical_edges):
  """Compute temperature and pressure from altitude for the grid cells."""
  # multiply by 1000 to convert from km to m
  environmental_conditions = ussa1976.compute(z=vertical_edges * 1000, variables=["t", "p"])
  temperature = environmental_conditions['t'].values
  pressure = environmental_conditions['p'].values

  return temperature, pressure

def check_missing_photo_rates(photolysis_rate_constants, state):
  """Print any photolysis rates required by the solver but not provided."""
  found_rates = sorted(photolysis_rate_constants.keys())
  needed_rates = sorted([i for i in state.get_user_defined_rate_parameters() if 'USER.j' in i])

  missing_rates = set(needed_rates) - set(found_rates)
  if missing_rates:
    print(f"Missing photolysis rates: {missing_rates}")

def set_photolysis_rates(state, photolysis_rate_constants):
  """Set all user-defined photolysis rate parameters on the solver state."""
  user_defined_dict = state.get_user_defined_rate_parameters()

  for key in user_defined_dict.keys():
    user_defined_dict[key] = photolysis_rate_constants[key]

  state.set_user_defined_rate_parameters(user_defined_dict)

def create_dataset(sim_times, concentrations, photo_rate_history, state, altitudes):
  """Build an xarray Dataset with concentrations, conditions, and rates."""
  conditions = state.get_conditions()

  data_vars = {
      "temperature": (["height"], conditions["temperature"], {'units': 'K'}),
      "pressure": (["height"], conditions["pressure"], {'units': 'Pa'}),
      "air_density": (["height"], conditions["air_density"], {'units': 'mol m-3'}),
  }

  photolysis_rate_names = list(photo_rate_history[0].keys())
  user_param_values = [
    [params[name] for name in photolysis_rate_names]
    for params in photo_rate_history
  ]
  data_vars["photolysis_rates"] = (["time", "photolysis_rate_names", "height"], user_param_values, {'units': 's-1'})

  for species in concentrations[0].keys():
    data = np.array([c[species] for c in concentrations], dtype=np.float64)  # shape: (time, height)
    data_vars[species] = (["time", "height"], data, {"units": "mol m-3"})

  coords = {
    "time": np.array(sim_times, dtype="datetime64[s]"),
    "height": altitudes,
    "photolysis_rate_names": np.array(photolysis_rate_names, dtype="S")
  }

  return xr.Dataset(data_vars, coords=coords)

def plot(ds, simulation_length):
  fig = plt.figure(figsize=(12, 8))
  gs = fig.add_gridspec(2, 2)

  ax_o = fig.add_subplot(gs[0, 0])
  ax_o1d = fig.add_subplot(gs[0, 1])
  ax_o3 = fig.add_subplot(gs[1, :])

  # Convert time from seconds to hours for better readability
  elapsed_hours = (ds['time'] - ds['time'].isel(time=0)) / np.timedelta64(1, 'h')
  time_hours = elapsed_hours

  # Plot each species
  ax_o.plot(time_hours, ds['O'])
  ax_o1d.plot(time_hours, ds['O1D'])
  ax_o3.plot(time_hours, ds['O3'])
  ax_o3.legend(title='Height', ncol=3, labels=[f'{h:.1f} km' for h in ds['height'].values])

  for _ax in (ax_o, ax_o1d, ax_o3):
      _ax.grid(True, alpha=0.5)
      _ax.spines[:].set_visible(False)
      _ax.tick_params(width=0)
      _ax.set_xlim(0, simulation_length / SECONDS_PER_HOUR)
      _ax.set_ylim(0, None)
      _ax.set_ylabel('Concentration [mol m-3]')
      _ax.set_xlabel('Time [hours]')

  ax_o.set_title('O')
  ax_o1d.set_title('O1D')
  ax_o3.set_title('O3')

  fig.tight_layout()
  fig.savefig('chapman.png', dpi=300)

def mixing_ratio_to_mol_m3(mixing_ratio, pressure, temperature):
  """Convert volume mixing ratio to concentration in mol m-3."""
  return (mixing_ratio * pressure) / (GAS_CONSTANT * temperature)

def main():
  start = 20
  num_grid_cells = 10
  today_local = datetime.now(boulder_tz).date()
  
  noon_local = datetime.combine(today_local, time(7, 30), tzinfo=boulder_tz)
  sim_time = (noon_local - timedelta(hours=1)).astimezone(ZoneInfo("UTC"))

  photolysis_rate_constants, tuv_rates = get_tuv_rates(sim_time, num_grid_cells, start)
  vertical_edges = tuv_rates.vertical_edge[start:start+num_grid_cells].data
  solver, state = get_solver(num_grid_cells)
  temperature, pressure = get_conditions(vertical_edges)

  # do this once
  check_missing_photo_rates(photolysis_rate_constants, state)

  initial_concentrations = {
      "N2": mixing_ratio_to_mol_m3(0.78084, pressure, temperature),
      "O2": mixing_ratio_to_mol_m3(0.20946, pressure, temperature),
      "O": [0] * num_grid_cells,
      "O1D": [0] * num_grid_cells,
      "O3": mixing_ratio_to_mol_m3(40e-9, pressure, temperature),
  }

  # Set all conditions on the state
  state.set_conditions(temperature, pressure)
  state.set_concentrations(initial_concentrations)
  set_photolysis_rates(state, photolysis_rate_constants)

  # setup and run the box model simulation
  sim_times = [sim_time]
  concentrations = [state.get_concentrations()]
  user_param_history = [{k: v.copy() for k, v in state.get_user_defined_rate_parameters().items()}]
  time_step = 15 * 60  # minutes in seconds
  simulation_length = 12 * SECONDS_PER_HOUR  # hours in seconds
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
      rounded_percent = int(current_percent // 5) * 5
      if rounded_percent > last_printed_percent:
        last_printed_percent = rounded_percent
        local_time = sim_time.astimezone(boulder_tz).strftime('%Y-%m-%d %H:%M:%S %p %Z')
        print(f"Simulation progress: {last_printed_percent}% // sim time: {local_time}")

      sim_time += timedelta(seconds=time_step)
      sim_times.append(sim_time)
      concentrations.append(state.get_concentrations())
      photolysis_rate_constants, _ = get_tuv_rates(sim_time, num_grid_cells, start)
      set_photolysis_rates(state, photolysis_rate_constants)
      user_param_history.append({k: v.copy() for k, v in state.get_user_defined_rate_parameters().items()})
    
  ds = create_dataset(sim_times, concentrations, user_param_history, state, vertical_edges)
  # for some reason, scipy must be used so that when the wheel is tested on linux, it works
  ds.to_netcdf("chapman.nc", engine="scipy")
  print(ds)
  plot(ds, simulation_length)

if __name__ == "__main__":
  main()