"""
Python example for CARMA sulfate that mimics the logic of the CARMA Fortran test.

This script creates one grid box with an initial concentration of sulfate particles
at the smallest size, then allows that to grow using H2SO4 gas. The total mass
of particles + gas should be conserved.

Based on carma_sulfatetest.F90
"""

import xarray as xr
import numpy as np
import musica

available = musica.backend.carma_available()

def run_carma_sulfate_example():
    """Test CARMA sulfate growth from gas condensation in a simple single grid box."""

    GRAVITY = 9.806  # Acceleration due to gravity in m/s^2

    # Simplified constants for debugging
    NZ = 1
    NELEM = 1
    NBIN = 38

    dtime = 1800.0  # Time step in seconds
    deltaz = 10000.0  # Grid box height in meters
    nsteps = int(180000 / dtime)

    # Sulfate density
    rho_sulfate_kg_m3 = 1923.0  # kg/m³

    # Grid setup
    latitude = -40.0
    longitude = -105.0
    vertical_center = [17000.0]
    vertical_levels = [vertical_center[0] - deltaz, vertical_center[0] + deltaz]

    # Standard atmosphere
    pressure_centers = np.array([9000.0])
    temperature_centers = np.array([250.0])
    # this is directly from the original CARMA test, but it seems like there are some unit conversion issues
    air_mass_density_centers = pressure_centers * 10.0 / (8.31430e07 / 28.966 * temperature_centers) * (1.0e-3 * 1.0e6)
    pressure_levels = np.zeros(2)
    pressure_levels[0] = pressure_centers[0] - \
        (vertical_levels[0] - vertical_center[0]) * air_mass_density_centers[0] * GRAVITY
    pressure_levels[1] = pressure_centers[0] - \
        (vertical_levels[1] - vertical_center[0]) * air_mass_density_centers[0] * GRAVITY

    # Initial mass mixing ratios
    mass_mixing_ratios = {
        "H2O": [1.0e-4],
        "H2SO4": [0.1e-9 * (98.0 / 29.0)],
        "SULFATE": [[0.0 for _ in range(NBIN)]]
    }
    satliq = {
        "H2O": [-1.0],
        "H2SO4": [-1.0]
    }
    satice = {
        "H2O": [-1.0],
        "H2SO4": [-1.0]
    }

    # Set up CARMA parameters
    params = musica.CARMAParameters()
    params.nz = NZ
    params.nbin = NBIN

    # Create sulfate group - simplified
    sulfate_group = musica.carma.CARMAGroupConfig(
        name="sulfate",
        shortname="SULF",
        rmin=2.e-10,  # Minimum radius in m
        rmrat=2.0,   # Mass ratio between bins
        swelling_approach={
            "algorithm": musica.carma.ParticleSwellingAlgorithm.WEIGHT_PERCENT_H2SO4,
            "composition": musica.carma.ParticleSwellingComposition.NONE
        },
        do_drydep=True,
        is_sulfate=True
    )
    params.add_group(sulfate_group)

    # Create sulfate element
    sulfate_element = musica.carma.CARMAElementConfig(
        igroup=1,
        name="Sulfate",
        shortname="SULF",
        rho=rho_sulfate_kg_m3,
        itype=musica.carma.ParticleType.VOLATILE,
        icomposition=musica.carma.ParticleComposition.SULFURIC_ACID
    )
    params.add_element(sulfate_element)

    # Create gases - simplified to match successful test_carma.py pattern
    water_gas = musica.carma.CARMAGasConfig(
        name="Water Vapor",
        shortname="H2O",
        wtmol=0.018015,  # kg/mol
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_MURPHY_2005,
        icomposition=musica.carma.GasComposition.H2O,
        dgc_threshold=0.1,
        ds_threshold=0.1
    )
    params.add_gas(water_gas)

    # Create H2SO4 gas
    h2so4_gas = musica.carma.CARMAGasConfig(
        name="Sulfuric Acid",
        shortname="H2SO4",
        wtmol=0.098079,  # kg/mol
        ivaprtn=musica.carma.VaporizationAlgorithm.H2SO4_AYERS_1980,
        icomposition=musica.carma.GasComposition.H2SO4,
        dgc_threshold=0.1,
        ds_threshold=0.1
    )
    params.add_gas(h2so4_gas)

    # Add growth process
    growth = musica.carma.CARMAGrowthConfig(
        ielem=1,  # Sulfate element
        igas=2    # H2SO4 gas
    )
    params.add_growth(growth)

    # Add nucleation process
    nucleation = musica.carma.CARMANucleationConfig(
        ielemfrom=1,
        ielemto=1,
        algorithm=musica.carma.ParticleNucleationAlgorithm.HOMOGENEOUS_NUCLEATION,
        rlh_nuc=0.0,
        igas=2  # H2SO4 gas
    )
    params.add_nucleation(nucleation)

    # Add coagulation
    coagulation = musica.carma.CARMACoagulationConfig(
        igroup1=1,
        igroup2=1,
        igroup3=1,
        algorithm=musica.carma.ParticleCollectionAlgorithm.FUCHS
    )
    params.add_coagulation(coagulation)

    # Initialization
    params.initialization.do_substep = True
    params.initialization.do_thermo = True
    params.initialization.maxretries = 16
    params.initialization.maxsubsteps = 32
    params.initialization.dt_threshold = 1.0
    params.initialization.sulfnucl_method = musica.carma.SulfateNucleationMethod.ZHAO_TURCO.value

    # Create CARMA instance
    carma = musica.CARMA(params)

    # Output group properties
    group_props, _ = carma.get_group_properties()
    # Print bin radius and bin mass from group_props xarray Dataset
    for i_bin in range(NBIN):
        print(
            f"Bin {i_bin + 1}: bin radius = {group_props.isel(bin=i_bin)['bin_radius']}; bin mass = {group_props.isel(bin=i_bin)['bin_mass']}"
        )

    bin_state = None
    gas_state = None
    env_state = None

    for i_step in range(nsteps):

        # Create state
        state = carma.create_state(
            time_step=dtime,
            temperature=temperature_centers,
            original_temperature=temperature_centers,
            pressure=pressure_centers,
            pressure_levels=pressure_levels,
            vertical_center=vertical_center,
            vertical_levels=vertical_levels,
            longitude=longitude,
            latitude=latitude,
            coordinates=musica.carma.CarmaCoordinates.CARTESIAN,
            specific_humidity=mass_mixing_ratios["H2O"],
        )

        # Initialize particle concentrations to zero
        for ibin in range(1, NBIN + 1):
            for ielem in range(1, NELEM + 1):
                state.set_bin(ibin, ielem, mass_mixing_ratios["SULFATE"][0][ibin - 1])

        # Set H2O concentration
        state.set_gas(
            gas_index=1,
            value=mass_mixing_ratios["H2O"],
            old_mmr=mass_mixing_ratios["H2O"],
            gas_saturation_wrt_ice=satice["H2O"],
            gas_saturation_wrt_liquid=satliq["H2O"],
        )

        # Set H2SO4 concentration
        state.set_gas(
            gas_index=2,
            value=[mmr * 1.05 for mmr in mass_mixing_ratios["H2SO4"]],
            old_mmr=mass_mixing_ratios["H2SO4"],
            gas_saturation_wrt_ice=satice["H2SO4"],
            gas_saturation_wrt_liquid=satliq["H2SO4"],
        )

        # Perform model step
        state.step()
        stats = state.get_step_statistics()
        print(f"Step {i_step + 1}/{nsteps}: {stats}")

        # Get updated state values
        if bin_state is None:
            bin_state = state.get_bins()
            bin_state = bin_state.expand_dims({"time": [i_step * dtime]})
        else:
            bin_state = xr.concat([bin_state, state.get_bins().expand_dims({"time": [i_step * dtime]})], dim="time")

        if gas_state is None:
            gas_state = state.get_gases()[0]
            gas_state = gas_state.expand_dims({"time": [i_step * dtime]})
        else:
            gas_state = xr.concat([gas_state, state.get_gases()[0].expand_dims({"time": [i_step * dtime]})], dim="time")

        if env_state is None:
            env_state = state.get_environmental_values()
            env_state = env_state.expand_dims({"time": [i_step * dtime]})
        else:
            env_state = xr.concat([env_state, state.get_environmental_values(
            ).expand_dims({"time": [i_step * dtime]})], dim="time")

    return env_state, gas_state, bin_state


if __name__ == '__main__':
    if not available:
        print("CARMA backend is not available.")
    else:
        env_state, gas_state, bin_state = run_carma_sulfate_example()
        print(env_state)
        print(gas_state)
        print(bin_state)
