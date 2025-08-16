"""
Python example for CARMA aluminum aerosols.

This script creates one grid box with an initial concentration of aluminum particles
and allows them to coagulate.
"""
import xarray as xr
import numpy as np
import musica
import ussa1976

available = musica.backend.carma_available()

def run_carma_aluminum_example():
    group = musica.carma.CARMAGroupConfig(
        name="aluminum",
        shortname="PRALUM",
        rmrat=2.0,
        rmin=21.5e-6,
        rmon=21.5e-6,
        ishape=musica.carma.ParticleShape.SPHERE,
        eshape=1.0,
        mie_calculation_algorithm=musica.carma.MieCalculationAlgorithm.TOON_1981,
        is_ice=False,
        is_fractal=True,
        do_wetdep=False,
        do_drydep=True,
        do_vtran=True,
        solfac=0.0,
        scavcoef=0.0,
        df=[1.6] * 5,  # 5 bins with fractal dimension 1.6
        falpha=1.0
    )

    # Create aluminum element
    element = musica.carma.CARMAElementConfig(
        igroup=1,
        isolute=0,
        name="Aluminum",
        shortname="ALUM",
        itype=musica.carma.ParticleType.INVOLATILE,
        icomposition=musica.carma.ParticleComposition.ALUMINUM,
        rho=0.00395,  # kg/m3
        arat=[1.0] * 5,  # 5 bins with area ratio 1.0
        kappa=0.0,
    )

    # Create coagulation
    coagulation = musica.carma.CARMACoagulationConfig(
        igroup1=1,
        igroup2=1,
        igroup3=1,
        algorithm=musica.carma.ParticleCollectionAlgorithm.FUCHS)

    params = musica.carma.CARMAParameters(
        nbin=5,
        nz=1,
        dtime=1800.0,
        groups=[group],
        elements=[element],
        coagulations=[coagulation]
    )

    FIVE_DAYS_IN_SECONDS = 432000
    params.nstep = FIVE_DAYS_IN_SECONDS // params.dtime
    params.initialization.do_vtran = False

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

    # Run the simulation for the specified number of steps
    for step in range(1, int(params.nstep)):
        state.step()
        bin_data = xr.concat([bin_data, state.get_bins().expand_dims({"time": [step * params.dtime]})], dim="time")
        env = xr.concat([env, state.get_environmental_values().expand_dims(
            {"time": [step * params.dtime]})], dim="time")

    return xr.merge([bin_data, env])


if __name__ == '__main__':
    if not available:
        print("CARMA backend is not available.")
    else:
        state = run_carma_aluminum_example()
        print(state)
