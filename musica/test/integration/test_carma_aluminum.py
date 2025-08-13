import xarray as xr
import numpy as np
import pytest
import musica
import ussa1976

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")


def test_carma_aluminum():
    # Test CARMA instance creation
    params = musica.CARMAParameters.create_aluminum_test_config()
    n_levels = params.nz
    deltaz = 1000.0
    zmin = 16500.0

    vertical_center = zmin + (np.arange(n_levels) + 0.5) * deltaz
    vertical_levels = zmin + np.arange(n_levels + 1) * deltaz

    centered_variables = ussa1976.compute(z=vertical_center, variables=["t", "p", "rho"])
    edge_variables = ussa1976.compute(z=vertical_levels, variables=["p"])

    temperature = centered_variables.t.values
    pressure = centered_variables.p.values
    pressure_levels = edge_variables.p.values
    density = centered_variables.rho.values

    carma = musica.CARMA(params)

    mmr_initial = 5e9 / (deltaz * 2.57474699e14) / density[0]

    state = carma.create_state(
        time_step=params.dtime,
        temperature=temperature,
        pressure=pressure,
        pressure_levels=pressure_levels,
        vertical_center=vertical_center,
        vertical_levels=vertical_levels,
        longitude=0.0,
        latitude=-105.0,
        coordinates=musica.carma.CarmaCoordinates.CARTESIAN,
    )

    for i in range(params.nbin):
        for j in range(len(params.elements)):
            state.set_bin(i + 1, j + 1, mmr_initial)

    bin_data = state.get_bins()
    bin_data = bin_data.expand_dims({"time": [0]})
    env = state.get_environmental_values()
    env = env.expand_dims({"time": [0]})
    time_array = [0.0]  # Start with time 0

    # Run the simulation for the specified number of steps
    for step in range(1, int(params.nstep)):
        state.step()
        bin_data = xr.concat([bin_data, state.get_bins().expand_dims({"time": [step * params.dtime]})], dim="time")
        env = xr.concat([env, state.get_environmental_values().expand_dims({"time": [step * params.dtime]})], dim="time")
        time_array.append(step * params.dtime)

    print(xr.merge([bin_data, env]))


if __name__ == '__main__':
    import sys
    pytest.main([__file__] + sys.argv[1:])
