import xarray as xr
import numpy as np
import pytest
import musica


def extract_data(params, state, env, data):
    nz = params.nz
    nbin = params.nbin
    nelem = len(params.elements)
    ngroup = len(params.groups)
    ngas = len(params.gases)
    nstep = params.nstep

    # Create coordinates
    coords = {}

    # Spatial coordinates
    vertical_center = state.vertical_center
    vertical_levels = state.vertical_levels

    coords['lat'] = ('y', [state.latitude])
    coords['lon'] = ('x', [state.longitude])
    coords['z'] = ('z', vertical_center)
    coords['z_levels'] = ('z_levels', vertical_levels)
    coords['time'] = ('time', list(range(int(params.nstep))))

    # Bin coordinates (1-indexed like Fortran)
    coords['bin'] = ('bin', list(range(1, nbin + 1)))
    coords['group'] = ('group', list(range(1, ngroup + 1)))
    coords['elem'] = ('elem', list(range(1, nelem + 1)))
    coords['nwave'] = ('nwave', list(
        range(1, len(params.wavelength_bins) + 1)))

    # Create z_interface coordinate for variables defined at interfaces (nz+1 levels)
    coords['z_interface'] = ('z_interface', list(range(1, nz + 2)))

    data_vars = {}

    # Atmospheric state variables
    pressure = [env[i]['pressure'] for i in range(len(env))]
    temperature = [env[i]['temperature'] for i in range(len(env))]
    air_density = [env[i]['air_density'] for i in range(len(env))]

    data_vars['pressure'] = (
        ('time', 'z'), pressure, {'units': 'Pa', 'long_name': 'Pressure'})
    data_vars['temperature'] = (
        ('time', 'z'), temperature, {'units': 'K', 'long_name': 'Temperature'})
    data_vars['air_density'] = (
        ('time', 'z'), air_density, {'units': 'kg m-3', 'long_name': 'Air density'})

    # data is time * bin * elem, each element contains data for each vertical level
    # so data's shape is: time * bin * elem * z

    print(f"shape: {np.array(data).shape}")

    def _extract_data(data, key):
      result = []
      for data_step in data:
          step_data = []
          for i in range(nbin):
              bin_data = []
              for j in range(nelem):
                  bin_data.append(data_step[i][j][key])
              step_data.append(bin_data)
          result.append(step_data)
      return np.transpose(result, (0, 3, 1, 2))


    data_vars['mass_mixing_ratio'] = (
        ('time', 'z', 'bin', 'elem'), _extract_data(data, 'mass_mixing_ratio'), {
            'units': 'kg kg-1', 'long_name': 'Mass mixing ratio'})
    data_vars['number_mixing_ratio'] = (
        ('time', 'z', 'bin', 'elem'), _extract_data(data, 'number_mixing_ratio'), {
            'units': '# kg-1', 'long_name': 'Number mixing ratio'})
    data_vars['number_density'] = (
        ('time', 'z', 'bin', 'elem'), _extract_data(data, 'number_density'), {
            'units': '# cm-3', 'long_name': 'Number density'})

    # Create the dataset
    return xr.Dataset(
        data_vars=data_vars,
        coords=coords,
        attrs={
            'title': 'CARMA aerosol model output',
            'description': 'Output from CARMA aerosol simulation',
            'nz': nz,
            'nbin': nbin,
            'nelem': nelem,
            'ngroup': ngroup,
            'ngas': ngas,
            'nstep': nstep
        }
    )


def extract_bin_data_for_timestep(params, state):
    nbin = params.nbin
    nelem = len(params.elements)
    bin_data = [[] for _ in range(nbin)]

    for i in range(nbin):
        for j in range(nelem):
            bin_data[i].append(state.get_bin(i + 1, j + 1))

    return bin_data


def test_carma_aluminum():
    # Test CARMA instance creation
    params = musica.CARMAParameters.create_aluminum_test_config()
    params.nz = 1

    carma = musica.CARMA(params)
    state = carma.create_state(
        longitude=0.0,
        latitude=-105.0,
        coordinates=musica.carma.CarmaCoordinates.CARTESIAN
    )
    env = state.get_environmental_values()
    mmr_initial = 5e9 / (params.deltaz * 2.57474699e14) / env["air_density"][0]

    for i in range(params.nbin):
        for j in range(len(params.elements)):
            state.set_bin(i + 1, j + 1, mmr_initial)

    data = [extract_bin_data_for_timestep(params, state)]
    env = [state.get_environmental_values()]

    # Run the simulation for the specified number of steps
    for step in range(1, int(params.nstep)):
        state.step(params.dtime)
        data.append(extract_bin_data_for_timestep(params, state))
        env.append(state.get_environmental_values())

    ds = extract_data(params, state, env, data)
    print(ds)


if __name__ == '__main__':
    # pytest.main([__file__])
    test_carma_aluminum()
