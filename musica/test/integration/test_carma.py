import pytest
import musica

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")


def test_carma_version():
    version = musica.carma.version
    assert version is not None
    assert isinstance(version, str)
    print(f"CARMA version: {version}")


def test_carma_with_all_components():
    """Test CARMA with multiple groups, elements, solutes, and gases"""
    params = musica.CARMAParameters()
    params.nz = 1
    params.ny = 1
    params.nx = 1
    params.nbin = 3
    params.dtime = 900.0
    params.deltaz = 500.0
    params.zmin = 1000.0

    # Set up wavelength bins
    params.wavelength_bins = [
        musica.carma.CARMAWavelengthBin(
            center=550e-9, width=50e-9, do_emission=True),   # 550 nm ± 25 nm
        musica.carma.CARMAWavelengthBin(
            center=850e-9, width=100e-9, do_emission=True)  # 850 nm ± 50 nm
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

    assert carma is not None
    assert isinstance(carma, musica.CARMA)

    state = carma.create_state(
        vertical_center=[16500.0],
        vertical_levels=[16500.0, 17000.0],
        pressure=[90000.0],
        pressure_levels=[101325.0, 90050.0],
        temperature=[280.0],
        time=0.0,
        time_step=900.0,  # 15 minutes
        longitude=0.0,
        latitude=0.0,
        coordinates=musica.carma.CarmaCoordinates.CARTESIAN
    )

    assert state is not None
    assert isinstance(state, musica.CARMAState)

    state.set_bin(1, 1, 1.0)
    state.set_detrain(1, 1, 1.0)
    state.set_gas(1, 1.4e-3)
    state.set_temperature(300.0)
    state.set_air_density(1.2)
    state.step(land=musica.carma.CARMASurfaceProperties(surface_friction_velocity=0.42, area_fraction=0.3),
               ocean=musica.carma.CARMASurfaceProperties(
                   aerodynamic_resistance=0.1),
               ice=musica.carma.CARMASurfaceProperties(area_fraction=0.2))
    print(state.get_step_statistics())
    print(state.get_bins())
    print(state.get_detrained_masses())
    print(state.get_environmental_values())
    print(state.get_gases())
    print(carma.get_group_properties())
    print(carma.get_element_properties())
    print(carma.get_gas_properties())
    print(carma.get_solute_properties())


if __name__ == '__main__':
    pytest.main([__file__])
