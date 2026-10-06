import numpy as np
import pytest
import musica

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")


def test_carma_version():
    version = musica.carma.__version__
    assert version is not None
    assert isinstance(version, str)
    print(f"CARMA version: {version}")


def test_carma_with_all_components():
    """Test CARMA with multiple groups, elements, solutes, and gases"""
    params = musica.carma.CARMAParameters()
    params.nbin = 3

    # Set up wavelength bins
    params.wavelength_bins = [
        musica.carma.CARMAWavelengthBin(
            center=550e-9, width=50e-9, do_emission=True),   # 550 nm ± 25 nm
        musica.carma.CARMAWavelengthBin(
            center=850e-9, width=100e-9, do_emission=True)  # 850 nm ± 50 nm
    ]

    # Group 1: Aluminum particles (sphere)
    aluminum_group = musica.carma.CARMAGroupConfig(
        short_name="ALUM",
        name="aluminum",
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
        short_name="SULF",
        name="sulfate",
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
        short_name="ICE",
        name="ice",
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
        short_name="AL",
        group="ALUM",
        name="Aluminum",
        rho=2.70,  # g/cm³
        itype=musica.carma.ParticleType.INVOLATILE,
        icomposition=musica.carma.ParticleComposition.ALUMINUM,
        kappa=0.0,
        is_shell=False  # core
    )
    params.elements.append(aluminum_element)

    # Element 2: Sulfate (Group 2)
    sulfate_element = musica.carma.CARMAElementConfig(
        short_name="SO4",
        group="SULF",
        solute="SO4",
        name="Sulfate",
        rho=1.84,  # g/cm³
        itype=musica.carma.ParticleType.VOLATILE,
        icomposition=musica.carma.ParticleComposition.SULFURIC_ACID,
        kappa=0.61,  # hygroscopicity
        is_shell=True
    )
    params.elements.append(sulfate_element)

    # Element 3: Water on sulfate (Group 2)
    water_element = musica.carma.CARMAElementConfig(
        short_name="H2O",
        group="SULF",
        name="Water",
        rho=1.0,  # g/cm³
        itype=musica.carma.ParticleType.CORE_MASS,
        icomposition=musica.carma.ParticleComposition.WATER,
        kappa=0.0,
        is_shell=True
    )
    params.elements.append(water_element)

    # Element 4: Ice (Group 3)
    ice_element = musica.carma.CARMAElementConfig(
        short_name="ICE",
        group="ICE",
        name="Ice",
        rho=0.92,  # g/cm³
        itype=musica.carma.ParticleType.INVOLATILE,
        icomposition=musica.carma.ParticleComposition.ICE,
        kappa=0.0,
        is_shell=False
    )
    params.elements.append(ice_element)

    # Solute: Sulfate
    sulfate_solute = musica.carma.CARMASoluteConfig(
        short_name="SO4",
        name="Sulfate",
        ions=2,
        wtmol=0.1324,  # kg mol-1
        rho=1840.0  # kg m-3
    )
    params.solutes.append(sulfate_solute)

    # Gas: Water vapor
    water_gas = musica.carma.CARMAGasConfig(
        short_name="H2O",
        name="Water Vapor",
        wtmol=0.01801528,  # kg mol-1
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_MURPHY_2005,
        icomposition=musica.carma.GasComposition.H2O,
        dgc_threshold=1.0e-6,
        ds_threshold=1.0e-4
    )
    params.gases.append(water_gas)

    # Gas: Sulfuric acid
    h2so4_gas = musica.carma.CARMAGasConfig(
        short_name="H2SO4",
        name="Sulfuric Acid",
        wtmol=0.098079,  # kg mol-1
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_BUCK_1981,
        icomposition=musica.carma.GasComposition.H2SO4,
        dgc_threshold=0.05,
        ds_threshold=0.1
    )
    params.gases.append(h2so4_gas)

    # Gas: Sulfur dioxide
    so2_gas = musica.carma.CARMAGasConfig(
        short_name="SO2",
        name="Sulfur Dioxide",
        wtmol=0.064066,  # kg mol-1
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_BUCK_1981,
        icomposition=musica.carma.GasComposition.SO2,
        dgc_threshold=0.05,
        ds_threshold=0.1
    )
    params.gases.append(so2_gas)

    # Create CARMA instance and run
    carma = musica.carma.CARMA(params)

    assert carma is not None
    assert isinstance(carma, musica.carma.CARMA)

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
    assert isinstance(state, musica.carma.CARMAState)

    state.set_bin(1, "AL", 1.0)
    state.set_detrain(1, "AL", 1.0)
    state.set_gas("H2O", 1.4e-3)
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


def _two_group_parameters():
    params = musica.carma.CARMAParameters(nbin=3)
    params.add_group(musica.carma.CARMAGroupConfig(short_name="DUST", do_vtran=False))
    params.add_group(musica.carma.CARMAGroupConfig(short_name="SULF", is_sulfate=True))
    params.add_element(musica.carma.CARMAElementConfig(short_name="DST", group="DUST"))
    params.add_element(musica.carma.CARMAElementConfig(
        short_name="SO4", group="SULF", solute="SALT",
        itype=musica.carma.ParticleType.VOLATILE,
        icomposition=musica.carma.ParticleComposition.SULFURIC_ACID))
    params.add_element(musica.carma.CARMAElementConfig(
        short_name="CORE", group="SULF", itype=musica.carma.ParticleType.CORE_MASS))
    params.add_solute(musica.carma.CARMASoluteConfig(short_name="SALT", ions=2, wtmol=0.058, rho=2160.0))
    params.add_gas(musica.carma.CARMAGasConfig(
        short_name="H2O", wtmol=0.018015,
        ivaprtn=musica.carma.VaporizationAlgorithm.H2O_MURPHY_2005,
        icomposition=musica.carma.GasComposition.H2O))
    params.add_gas(musica.carma.CARMAGasConfig(
        short_name="H2SO4", wtmol=0.098079,
        ivaprtn=musica.carma.VaporizationAlgorithm.H2SO4_AYERS_1980,
        icomposition=musica.carma.GasComposition.H2SO4))
    params.add_coagulation(musica.carma.CARMACoagulationConfig(
        group1="SULF", group2="DUST", group3="SULF",
        algorithm=musica.carma.ParticleCollectionAlgorithm.FUCHS))
    params.add_growth(musica.carma.CARMAGrowthConfig(element="SO4", gas="H2SO4"))
    params.add_nucleation(musica.carma.CARMANucleationConfig(
        element_from="SO4", element_to="SO4", gas="H2SO4",
        algorithm=musica.carma.ParticleNucleationAlgorithm.HOMOGENEOUS_NUCLEATION))
    return params


def _create_state(carma, n_levels=1):
    return carma.create_state(
        vertical_center=[16500.0 + 1000.0 * i for i in range(n_levels)],
        vertical_levels=[16000.0 + 1000.0 * i for i in range(n_levels + 1)],
        pressure=[9000.0 - 1000.0 * i for i in range(n_levels)],
        pressure_levels=[9500.0 - 1000.0 * i for i in range(n_levels + 1)],
        temperature=[250.0] * n_levels,
        time_step=1800.0)


def test_short_names_resolve_to_indices():
    cpp = _two_group_parameters()._to_cpp()
    assert [element.igroup for element in cpp.elements] == [1, 2, 2]
    assert [element.isolute for element in cpp.elements] == [0, 1, 0]
    assert [element.shortname for element in cpp.elements] == ["DST", "SO4", "CORE"]
    coagulation = cpp.coagulations[0]
    assert (coagulation.igroup1, coagulation.igroup2, coagulation.igroup3) == (2, 1, 2)
    assert (cpp.growths[0].ielem, cpp.growths[0].igas) == (2, 2)
    nucleation = cpp.nucleations[0]
    assert (nucleation.ielemfrom, nucleation.ielemto, nucleation.igas, nucleation.ievp2elem) == (2, 2, 2, 0)


def test_unknown_short_name_raises():
    params = _two_group_parameters()
    params.add_growth(musica.carma.CARMAGrowthConfig(element="SO4", gas="SO2"))
    with pytest.raises(ValueError, match="No gas has short_name 'SO2'"):
        musica.carma.CARMA(params)


def test_integer_reference_raises():
    params = _two_group_parameters()
    params.add_growth(musica.carma.CARMAGrowthConfig(element=2, gas="H2SO4"))
    with pytest.raises(TypeError, match="short_name"):
        musica.carma.CARMA(params)


def test_duplicate_short_name_raises():
    params = _two_group_parameters()
    params.add_gas(musica.carma.CARMAGasConfig(short_name="H2O"))
    with pytest.raises(ValueError, match="unique short_name"):
        musica.carma.CARMA(params)


def test_state_size_comes_from_vertical_grid():
    carma = musica.carma.CARMA(_two_group_parameters())
    for n_levels in (1, 4):
        state = _create_state(carma, n_levels)
        state.set_bin(1, "SO4", 1.0e-10)
        state.set_gas("H2SO4", 1.0e-10)
        state.step()
        bins = state.get_bins()
        assert bins.sizes["vertical_center"] == n_levels
        assert bins.sizes["vertical_level"] == n_levels + 1


def test_state_setters_reject_unknown_short_names():
    state = _create_state(musica.carma.CARMA(_two_group_parameters()))
    with pytest.raises(ValueError, match="No element has short_name 'XX'"):
        state.set_bin(1, "XX", 1.0)
    with pytest.raises(ValueError, match="No gas has short_name 'XX'"):
        state.set_gas("XX", 1.0)


def test_outputs_use_short_name_coordinates():
    carma = musica.carma.CARMA(_two_group_parameters())
    state = _create_state(carma)
    assert list(state.get_bins().element.values) == ["DST", "SO4", "CORE"]
    assert list(state.get_detrained_masses().element.values) == ["DST", "SO4", "CORE"]
    assert list(state.get_gases()[0].gas.values) == ["H2O", "H2SO4"]
    group_properties, _ = carma.get_group_properties()
    assert list(group_properties.group.values) == ["DUST", "SULF"]
    element_properties, _ = carma.get_element_properties()
    assert list(element_properties.element.values) == ["DST", "SO4", "CORE"]


def test_disabled_physics_returns_nan():
    params = _two_group_parameters()
    params.initialization.do_vtran = True
    carma = musica.carma.CARMA(params)
    state = _create_state(carma)
    state.set_bin(1, "DST", 1.0e-10)
    state.set_bin(1, "SO4", 1.0e-10)
    state.set_gas("H2O", 1.0e-4)
    state.set_gas("H2SO4", 1.0e-10)
    state.step()
    bins = state.get_bins()

    assert np.isnan(bins.fall_velocity.sel(element="DST")).all()
    assert np.isnan(bins.sedimentation_flux.sel(element="DST")).all()
    assert not np.isnan(bins.fall_velocity.sel(element="SO4")).any()
    assert not np.isnan(bins.sedimentation_flux.sel(element="SO4")).any()

    assert np.isnan(bins.deposition_velocity).all()
    assert np.isnan(bins.particle_mass_on_surface).all()
    assert np.isnan(bins.delta_particle_temperature).all()

    assert np.isnan(bins.nucleation_rate.sel(element="DST")).all()
    assert not np.isnan(bins.nucleation_rate.sel(element="SO4")).any()

    assert np.isnan(bins.number_density.sel(element="CORE")).all()
    assert not np.isnan(bins.number_density.sel(element="SO4")).any()
    assert not np.isnan(bins.mass_mixing_ratio).any()
    assert bins.fall_velocity.attrs["units"] == "m s-1"

    assert np.isnan(state.get_environmental_values().latent_heat).all()


def test_gas_without_vaporization_raises():
    params = _two_group_parameters()
    params.add_gas(musica.carma.CARMAGasConfig(short_name="SO2", wtmol=0.064066))
    with pytest.raises(ValueError, match="Gas 'SO2' has no vaporization routine"):
        musica.carma.CARMA(params)


def test_calculated_values_are_zero_before_first_step():
    params = _two_group_parameters()
    params.initialization.do_vtran = True
    params.initialization.do_thermo = True
    carma = musica.carma.CARMA(params)
    for _ in range(2):
        state = _create_state(carma)
        state.set_bin(1, "SO4", 1.0e-10)
        state.set_gas("H2O", 1.0e-4)
        state.set_gas("H2SO4", 1.0e-10)
        so4 = state.get_bins().sel(element="SO4")
        for name in ("nucleation_rate", "wet_particle_radius", "wet_particle_density", "dry_particle_density",
                     "fall_velocity", "sedimentation_flux", "kappa"):
            assert (so4[name] == 0.0).all(), name
        assert so4.mass_mixing_ratio.sel(bin=1).item() == 1.0e-10
        assert (so4.mass_mixing_ratio.sel(bin=[2, 3]) == 0.0).all()
        gases, _ = state.get_gases()
        assert not np.isnan(gases.gas_saturation_wrt_liquid).any()
        assert (state.get_environmental_values().latent_heat == 0.0).all()
        state.step()


if __name__ == '__main__':
    pytest.main([__file__])
