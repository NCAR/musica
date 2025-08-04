import pytest
import musica
import xarray as xr

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")


def test_carma_version():
    version = musica.carma.version
    assert version is not None
    assert isinstance(version, str)
    print(f"CARMA version: {version}")


def test_carma_instance():
    # Test CARMA instance creation
    test_params = musica.CARMAParameters.create_aluminum_test_config()

    # Add a gas to the parameters
    test_params.gases.append(
        musica.carma.CARMAGasConfig(
            name="Test Gas",
            shortname="TG",
            wtmol=0.018,  # kg mol-1
            ivaprtn=musica.carma.VaporizationAlgorithm.H2O_BUCK_1981,
            icomposition=musica.carma.GasComposition.H2O,
            dgc_threshold=1.0e-8,
            ds_threshold=1.0e-6
        )
    )

    carma = musica.CARMA(test_params)
    assert carma is not None
    assert isinstance(carma, musica.CARMA)

    state = carma.create_state(
        longitude=0.0,
        latitude=0.0,
        temperature=[300.0, 280.0],
        pressure=[101335.0, 90000.0],
        pressure_levels=[101325.0, 90050.0, 80000.0],
        coordinates=musica.carma.CarmaCoordinates.CARTESIAN
    )

    assert state is not None
    assert isinstance(state, musica.CARMAState)

    state.set_bin(1, 1, 1.0)
    state.set_detrain(1, 1, 1.0)
    carma.run()
    state.set_gas(1, 1.4e-3)
    state.set_temperature(300.0)
    state.set_air_density(1.2)
    state.step(land=musica.carma.CARMASurfaceProperties(surface_friction_velocity=0.42, area_fraction=0.3),
               ocean=musica.carma.CARMASurfaceProperties(aerodynamic_resistance=0.1),
               ice=musica.carma.CARMASurfaceProperties(area_fraction=0.2))
    print(state.get_step_statistics())
    print(state.get_bin(1, 1))
    print(state.get_detrain(1, 1))
    print(state.get_environmental_values())
    print(state.get_gas(1))
    print(carma.get_group_properties(1))
    print(carma.get_element_properties(1))
    print(carma.get_gas_properties(1))


def test_carma_with_default_parameters():
    """Test CARMA with default parameters - mimics RunCarmaWithDefaultParameters C++ test"""
    default_params = musica.CARMAParameters()
    carma = musica.CARMA(default_params)

    # Test that we can run CARMA with default parameters without throwing
    output = carma.run()
    assert output is not None


def test_carma_with_all_components():
    """Test CARMA with multiple groups, elements, solutes, and gases - mimics RunCarmaWithAllComponents C++ test"""
    params = musica.CARMAParameters()
    params.nz = 2
    params.ny = 1
    params.nx = 1
    params.nbin = 3
    params.dtime = 900.0
    params.deltaz = 500.0
    params.zmin = 1000.0

    # Set up wavelength bins
    params.wavelength_bins = [
        musica.carma.CARMAWavelengthBin(center=550e-9, width=50e-9, do_emission=True),   # 550 nm ± 25 nm
        musica.carma.CARMAWavelengthBin(center=850e-9, width=100e-9, do_emission=True)  # 850 nm ± 50 nm
    ]

    # Group 1: Aluminum particles (sphere)
    aluminum_group = musica.carma.CARMAGroupConfig(
        name="aluminum",
        shortname="ALUM",
        rmin=1e-8,
        rmrat=2.0,
        ishape=musica.carma.ParticleShape.SPHERE,
        eshape=1.0,
        is_fractal=False,
        do_vtran=True,
        do_drydep=True,
        df=[1.8, 1.8, 1.8]  # fractal dimension per bin
    )
    params.groups.append(aluminum_group)

    # Group 2: Sulfate particles (sphere, with swelling)
    sulfate_group = musica.carma.CARMAGroupConfig(
        name="sulfate",
        shortname="SULF",
        rmin=5e-9,
        rmrat=2.5,
        ishape=musica.carma.ParticleShape.SPHERE,
        eshape=1.0,
        swelling_approach={
            "algorithm": musica.carma.ParticleSwellingAlgorithm.FITZGERALD,
            "composition": musica.carma.ParticleSwellingComposition.AMMONIUM_SULFATE
        },
        is_sulfate=True,
        do_wetdep=True,
        do_vtran=True,
        solfac=0.8,
        df=[2.0, 2.0, 2.0]
    )
    params.groups.append(sulfate_group)

    # Group 3: Ice particles (hexagon)
    ice_group = musica.carma.CARMAGroupConfig(
        name="ice",
        shortname="ICE",
        rmin=2e-8,
        rmrat=3.0,
        ishape=musica.carma.ParticleShape.HEXAGON,
        eshape=2.0,  # aspect ratio
        is_ice=True,
        is_cloud=True,
        do_vtran=True,
        df=[1.5, 1.5, 1.5]
    )
    params.groups.append(ice_group)

    # Element 1: Aluminum core (Group 1)
    aluminum_element = musica.carma.CARMAElementConfig(
        igroup=1,
        name="Aluminum",
        shortname="AL",
        rho=2.70,  # g/cm³
        itype=musica.carma.ParticleType.INVOLATILE,
        icomposition=musica.carma.ParticleComposition.ALUMINUM,
        kappa=0.0,
        is_shell=False  # core
    )
    params.elements.append(aluminum_element)

    # Element 2: Sulfate (Group 2)
    sulfate_element = musica.carma.CARMAElementConfig(
        igroup=2,
        isolute=1,  # linked to first solute
        name="Sulfate",
        shortname="SO4",
        rho=1.84,  # g/cm³
        itype=musica.carma.ParticleType.VOLATILE,
        icomposition=musica.carma.ParticleComposition.SULFURIC_ACID,
        kappa=0.61,  # hygroscopicity
        is_shell=True
    )
    params.elements.append(sulfate_element)

    # Element 3: Water on sulfate (Group 2)
    water_element = musica.carma.CARMAElementConfig(
        igroup=2,
        name="Water",
        shortname="H2O",
        rho=1.0,  # g/cm³
        itype=musica.carma.ParticleType.CORE_MASS,
        icomposition=musica.carma.ParticleComposition.WATER,
        kappa=0.0,
        is_shell=True
    )
    params.elements.append(water_element)

    # Element 4: Ice (Group 3)
    ice_element = musica.carma.CARMAElementConfig(
        igroup=3,
        name="Ice",
        shortname="ICE",
        rho=0.92,  # g/cm³
        itype=musica.carma.ParticleType.INVOLATILE,
        icomposition=musica.carma.ParticleComposition.ICE,
        kappa=0.0,
        is_shell=False
    )
    params.elements.append(ice_element)

    # Solute: Sulfate
    sulfate_solute = musica.carma.CARMASoluteConfig(
        name="Sulfate",
        shortname="SO4",
        ions=2,
        wtmol=0.1324,  # kg mol-1
        rho=1840.0  # kg m-3
    )
    params.solutes.append(sulfate_solute)

    # Gas: Water vapor
    water_gas = musica.carma.CARMAGasConfig(
        name="Water Vapor",
        shortname="H2O",
        wtmol=0.01801528,  # kg mol-1
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_MURPHY_2005,
        icomposition=musica.carma.GasComposition.H2O,
        dgc_threshold=1.0e-6,
        ds_threshold=1.0e-4
    )
    params.gases.append(water_gas)

    # Gas: Sulfuric acid
    h2so4_gas = musica.carma.CARMAGasConfig(
        name="Sulfuric Acid",
        shortname="H2SO4",
        wtmol=0.098079,  # kg mol-1
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_BUCK_1981,
        icomposition=musica.carma.GasComposition.H2SO4,
        dgc_threshold=0.05,
        ds_threshold=0.1
    )
    params.gases.append(h2so4_gas)

    # Gas: Sulfur dioxide
    so2_gas = musica.carma.CARMAGasConfig(
        name="Sulfur Dioxide",
        shortname="SO2",
        wtmol=0.064066,  # kg mol-1
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_BUCK_1981,
        icomposition=musica.carma.GasComposition.SO2,
        dgc_threshold=0.05,
        ds_threshold=0.1
    )
    params.gases.append(so2_gas)

    # Create CARMA instance and run
    carma = musica.CARMA(params)
    output = carma.run()

    # Verify basic output structure exists
    assert output is not None
    assert hasattr(output, 'lat')
    assert hasattr(output, 'lon')
    assert hasattr(output, 'z')
    assert hasattr(output, 'pressure')
    assert hasattr(output, 'temperature')
    assert hasattr(output, 'air_density')

    # Verify dimensions match parameters
    assert len(output.lat) == params.ny
    assert len(output.lon) == params.nx
    assert len(output.z) == params.nz
    assert len(output.pressure) == params.nz
    assert len(output.temperature) == params.nz
    assert len(output.air_density) == params.nz

    # Verify 3D particle arrays exist and have reasonable structure
    assert hasattr(output, 'particle_concentration')
    assert hasattr(output, 'mass_mixing_ratio')
    assert len(output.particle_concentration) == params.nz
    assert len(output.mass_mixing_ratio) == params.nz

    # Verify that the output contains data for the configured number of elements
    assert len(output.particle_concentration[0]) == params.nbin
    # Should have data for all configured elements
    assert len(output.particle_concentration[0][0]) == len(params.elements)

    print(f"Successfully ran CARMA with {len(params.groups)} groups and {len(params.elements)} elements")

def test_carma_aluminum():
    # Test CARMA instance creation
    params = musica.CARMAParameters.create_aluminum_test_config()

    carma = musica.CARMA(params)
    state = carma.create_state(
        longitude=0.0,
        latitude=-105.0,
        coordinates=musica.carma.CarmaCoordinates.CARTESIAN
    )
    env = state.get_environmental_values()
    mmr_initial = 5e9 / (params.deltaz * 2.57474699e14) / env["air_density"][0]

    # output arrays
    mmr = []
    env = [state.get_environmental_values()]

    # set initial conditions, intiiail environmental values are set in the state constructor
    mmr_step = [[] * len(params.elements) for _ in range(params.nbin)]
    for i in range(params.nbin):
        for j in range(len(params.elements)):
            state.set_bin(i + 1, j + 1, mmr_initial)
            mmr_step[i].append(state.get_bin(i + 1, j + 1))

    mmr.append(mmr_step)

    # Run the simulation for the specified number of steps
    for step in range(1, int(params.nstep)):
        state.step(params.dtime)
        mmr_step = [[] * len(params.elements) for _ in range(params.nbin)]
        for i in range(params.nbin):
            for j in range(len(params.elements)):
                mmr_step[i].append(state.get_bin(i + 1, j + 1))
        mmr.append(mmr_step)
        env.append(state.get_environmental_values())

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

    # # Particle state variables (3D: nz x nbin x nelem)
    # particle_concentration = output_dict.get('particle_concentration', [])
    # data_vars['particle_concentration'] = (
    #     ('z', 'bin', 'elem'), np.array(particle_concentration), {
    #         'units': '# cm-3', 'long_name': 'Particle concentration'})

    # mass_mixing_ratio = output_dict.get('mass_mixing_ratio', [])
    # data_vars['mass_mixing_ratio'] = (
    #     ('z', 'bin', 'elem'), np.array(mass_mixing_ratio), {
    #         'units': 'kg kg-1', 'long_name': 'Mass mixing ratio'})

    # wet_radius = output_dict.get('wet_radius', [])
    # data_vars['wet_radius'] = (
    #     ('z', 'bin', 'group'), np.array(wet_radius), {
    #         'units': 'cm', 'long_name': 'Wet radius of particles'})

    # wet_density = output_dict.get('wet_density', [])
    # data_vars['wet_density'] = (
    #     ('z', 'bin', 'group'), np.array(wet_density), {
    #         'units': 'g cm-3', 'long_name': 'Wet density of particles'})

    # fall_velocity = output_dict.get('fall_velocity', [])
    # data_vars['fall_velocity'] = (('z_interface', 'bin', 'group'), np.array(fall_velocity), {
    #                               'units': 'cm s-1', 'long_name': 'Fall velocity of particles'})

    # nucleation_rate = output_dict.get('nucleation_rate', [])
    # data_vars['nucleation_rate'] = (
    #     ('z', 'bin', 'group'), np.array(nucleation_rate), {
    #         'units': 'cm-3 s-1', 'long_name': 'Nucleation rate of particles'})

    # deposition_velocity = output_dict.get('deposition_velocity', [])
    # data_vars['deposition_velocity'] = (
    #     ('z', 'bin', 'group'), np.array(deposition_velocity), {
    #         'units': 'cm s-1', 'long_name': 'Deposition velocity of particles'})

    # dry_radius = output_dict.get('dry_radius', [])
    # data_vars['dry_radius'] = (
    #     ('bin', 'group'), np.array(dry_radius), {
    #         'units': 'cm', 'long_name': 'Dry radius of particles'})

    # mass_per_bin = output_dict.get('mass_per_bin', [])
    # data_vars['mass_per_bin'] = (
    #     ('bin', 'group'), np.array(mass_per_bin), {
    #         'units': 'g', 'long_name': 'Mass per bin of particles'})

    # radius_ratio = output_dict.get('radius_ratio', [])
    # data_vars['radius_ratio'] = (
    #     ('bin', 'group'), np.array(radius_ratio), {
    #         'units': '1', 'long_name': 'Radius ratio of particles'})

    # aspect_ratio = output_dict.get('aspect_ratio', [])
    # data_vars['aspect_ratio'] = (
    #     ('bin', 'group'), np.array(aspect_ratio), {
    #         'units': '1', 'long_name': 'Aspect ratio of particles'})

    # group_particle_number_concentration = output_dict.get(
    #     'group_particle_number_concentration', [])
    # data_vars['group_particle_number_concentration'] = (
    #     ('group'), np.array(group_particle_number_concentration), {
    #         'units': '# cm-3', 'long_name': 'Group particle number concentration'})

    # constituent_type = output_dict.get('constituent_type', [])
    # data_vars['constituent_type'] = (
    #     ('group'), np.array(constituent_type), {
    #         'units': '1', 'long_name': 'Constituent type of particle groups'})

    # max_prognostic_bin = output_dict.get('max_prognostic_bin', [])
    # data_vars['max_prognostic_bin'] = (
    #     ('group'), np.array(max_prognostic_bin), {
    #         'units': '1', 'long_name': 'Maximum prognostic bin for each group'})

    # Create the dataset
    ds = xr.Dataset(
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
    print(ds)


if __name__ == '__main__':
    pytest.main([__file__])
