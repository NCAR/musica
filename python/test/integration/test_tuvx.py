import pytest
import musica
import numpy as np

available = musica.backend.tuvx_available()
pytestmark = pytest.mark.skipif(not available, reason="TUV-x backend is not available")


def get_grid_map():
    heights = musica.Grid(name="height", units="km", num_sections=120)
    heights.edges = np.linspace(0, 120, 121)
    heights.midpoints = 0.5 * (heights.edges[:-1] + heights.edges[1:])
    wavelengths = musica.Grid(name="wavelength", units="nm", num_sections=156)
    wavelengths.edges = np.array(
        [
            120.0000,
            121.4000,
            121.9000,
            122.3000,
            123.1000,
            123.8000,
            124.6000,
            125.4000,
            126.2000,
            127.0000,
            128.6000,
            129.4000,
            130.3000,
            132.0000,
            135.0000,
            137.0000,
            145.0000,
            155.0000,
            165.0000,
            170.0000,
            175.4000,
            177.0000,
            178.6000,
            180.2000,
            181.8000,
            183.5000,
            185.2000,
            186.9000,
            188.7000,
            190.5000,
            192.3000,
            194.2000,
            196.1000,
            198.0000,
            200.0000,
            202.0000,
            204.1000,
            206.2000,
            208.3330,
            210.5260,
            212.7660,
            215.0540,
            217.3910,
            219.7800,
            222.2220,
            224.7190,
            227.2730,
            229.8850,
            232.5580,
            235.2940,
            238.0950,
            240.9640,
            243.9020,
            246.9140,
            250.0000,
            253.1650,
            256.4100,
            259.7400,
            263.1580,
            266.6670,
            270.2700,
            273.9730,
            277.7780,
            281.6900,
            285.7140,
            289.8550,
            294.1180,
            298.5000,
            302.5000,
            303.5000,
            304.5000,
            305.5000,
            306.5000,
            307.5000,
            308.5000,
            309.5000,
            310.5000,
            311.5000,
            312.5000,
            313.5000,
            314.5000,
            317.5000,
            322.5000,
            327.5000,
            332.5000,
            337.5000,
            342.5000,
            347.5000,
            352.5000,
            357.5000,
            362.5000,
            367.5000,
            372.5000,
            377.5000,
            382.5000,
            387.5000,
            392.5000,
            397.5000,
            402.5000,
            407.5000,
            412.5000,
            417.5000,
            422.5000,
            427.5000,
            432.5000,
            437.5000,
            442.5000,
            447.5000,
            452.5000,
            457.5000,
            462.5000,
            467.5000,
            472.5000,
            477.5000,
            482.5000,
            487.5000,
            492.5000,
            497.5000,
            502.5000,
            507.5000,
            512.5000,
            517.5000,
            522.5000,
            527.5000,
            532.5000,
            537.5000,
            542.5000,
            547.5000,
            552.5000,
            557.5000,
            562.5000,
            567.5000,
            572.5000,
            577.5000,
            582.5000,
            587.5000,
            592.5000,
            597.5000,
            602.5000,
            607.5000,
            612.5000,
            617.5000,
            622.5000,
            627.5000,
            632.5000,
            637.5000,
            642.5000,
            647.1000,
            655.0000,
            665.0000,
            675.0000,
            685.0000,
            695.0000,
            705.0000,
            715.0000,
            725.0000,
            735.0000,
        ]
    )
    wavelengths.midpoints = 0.5 * (wavelengths.edges[:-1] + wavelengths.edges[1:])
    grid_map = musica.GridMap()
    grid_map["height", "km"] = heights
    grid_map["wavelength", "nm"] = wavelengths
    return grid_map


def get_profile_map(grid_map):
    # Simple exponential profiles for testing
    midpoints = 1.0e-6 * 2.54e19 * np.exp(-(grid_map["height", "km"].midpoints - 120) / 7)
    ozone = musica.Profile(name="O3", units="molecule cm-3", grid=grid_map["height", "km"], midpoint_values=midpoints)
    midpoints = 2.54e19 * np.exp(-(grid_map["height", "km"].midpoints - 120) / 7)
    air = musica.Profile(name="air", units="molecule cm-3", grid=grid_map["height", "km"], midpoint_values=midpoints)
    midpoints = 0.21 * 2.54e19 * np.exp(-(grid_map["height", "km"].midpoints - 120) / 7)
    oxygen = musica.Profile(name="O2", units="molecule cm-3", grid=grid_map["height", "km"], midpoint_values=midpoints)
    midpoints = 298.0 * np.exp(-(grid_map["height", "km"].midpoints - 120) / 7)
    temperature = musica.Profile(name="temperature", units="K",
                                 grid=grid_map["height", "km"], midpoint_values=midpoints)
    midpoints = 0.1 * np.ones(grid_map["wavelength", "nm"].num_sections)
    surface_albedo = musica.Profile(name="surface albedo", units="none",
                                    grid=grid_map["wavelength", "nm"], midpoint_values=midpoints)
    midpoints = 1.0e18 * 1420.0 / 615.0 * 0.0001 * np.ones(grid_map["wavelength", "nm"].num_sections)
    et_flux = musica.Profile(name="extraterrestrial flux", units="photon cm-2 s-1",
                             grid=grid_map["wavelength", "nm"], midpoint_values=midpoints)
    profile_map = musica.ProfileMap()
    profile_map["O3", "molecule cm-3"] = ozone
    profile_map["air", "molecule cm-3"] = air
    profile_map["O2", "molecule cm-3"] = oxygen
    profile_map["temperature", "K"] = temperature
    profile_map["surface albedo", "none"] = surface_albedo
    profile_map["extraterrestrial flux", "photon cm-2 s-1"] = et_flux
    return profile_map


def get_radiator_map(grid_map):
    ssa = 0.99 * np.ones((grid_map["wavelength", "nm"].num_sections, grid_map["height", "km"].num_sections))
    asymmetry = 0.61 * np.ones((grid_map["wavelength", "nm"].num_sections, grid_map["height", "km"].num_sections))

    # clouds
    od = np.tile(1.0e-6 * np.exp(-(grid_map["height", "km"].midpoints - 120) / 7),
                 (grid_map["wavelength", "nm"].num_sections, 1))
    clouds = musica.Radiator(name="clouds",
                             height_grid=grid_map["height",
                                                  "km"],
                             wavelength_grid=grid_map["wavelength",
                                                      "nm"],
                             optical_depths=od,
                             single_scattering_albedos=ssa,
                             asymmetry_factors=asymmetry)
    # hot air balloons
    od = np.tile(1.8e-8 * np.exp(-(grid_map["height", "km"].midpoints - 120) / 7),
                 (grid_map["wavelength", "nm"].num_sections, 1))
    hot_air_balloons = musica.Radiator(name="hot air balloons",
                                       height_grid=grid_map["height",
                                                            "km"],
                                       wavelength_grid=grid_map["wavelength",
                                                                "nm"],
                                       optical_depths=od,
                                       single_scattering_albedos=ssa,
                                       asymmetry_factors=asymmetry)
    radiator_map = musica.RadiatorMap()
    radiator_map["clouds"] = clouds
    radiator_map["hot air balloons"] = hot_air_balloons
    return radiator_map


def test_tuvx_version():
    version = musica.tuvx.__version__
    assert version is not None
    assert isinstance(version, str)


def test_full_tuvx(monkeypatch):
    monkeypatch.chdir("configs/tuvx")
    file = "tuv_5_4.json"
    grid_map = get_grid_map()
    profile_map = get_profile_map(grid_map)
    radiator_map = get_radiator_map(grid_map)
    tuvx = musica.TUVX(grid_map, profile_map, radiator_map, config_path=file)
    assert tuvx is not None

    sza = 2.0  # Radians
    earth_sun_distance = 1.0  # AU

    photolysis_rates, heating_rates, dose_rates = tuvx.run(sza, earth_sun_distance)

    assert len(tuvx.heating_rate_names) == 0, "Heating rate names should be empty for this config"
    assert len(tuvx.photolysis_rate_names) > 0, "No photolysis rate names found"
    assert len(tuvx.dose_rate_names) > 0, "No dose rate names found"
    assert len(heating_rates) == 121, "Number of layers should be 121 for heating rates"
    assert len(photolysis_rates) == 121, "Number of layers should be 121 for photolysis rates"
    assert len(dose_rates) == 121, "Number of layers should be 121 for dose rates"
    assert heating_rates.shape[1] == 0, "Should be no heating rates for this config"
    assert photolysis_rates.shape[1] == len(tuvx.photolysis_rate_names), "Photolysis rates shape mismatch"
    assert dose_rates.shape[1] == len(tuvx.dose_rate_names), "Dose rates shape mismatch"


def get_fixed_grid_map():
    heights = musica.Grid(name="height", units="km", num_sections=3)
    heights.edges = np.array([0.0, 10.0, 20.0, 30.0])
    heights.midpoints = 0.5 * (heights.edges[:-1] + heights.edges[1:])
    wavelengths = musica.Grid(name="wavelength", units="nm", num_sections=5)
    wavelengths.edges = np.array([300.0, 400.0, 500.0, 600.0, 700.0, 800.0])
    wavelengths.midpoints = 0.5 * (wavelengths.edges[:-1] + wavelengths.edges[1:])
    grid_map = musica.GridMap()
    grid_map["height", "km"] = heights
    grid_map["wavelength", "nm"] = wavelengths
    return grid_map


def test_fixed_tuvx_from_file(monkeypatch):
    monkeypatch.chdir("src")
    file = "test/data/tuvx/full_from_host/config.json"
    grid_map = get_fixed_grid_map()
    profile_map = get_profile_map(grid_map)
    radiator_map = get_radiator_map(grid_map)
    tuvx = musica.TUVX(grid_map, profile_map, radiator_map, config_path=file)
    assert tuvx is not None

    # Access properties multiple times
    photolysis_names_1 = tuvx.photolysis_rate_names
    photolysis_names_2 = tuvx.photolysis_rate_names
    heating_names_1 = tuvx.heating_rate_names
    heating_names_2 = tuvx.heating_rate_names
    dose_names_1 = tuvx.dose_rate_names
    dose_names_2 = tuvx.dose_rate_names

    # Verify they return the same object (cached)
    assert photolysis_names_1 is photolysis_names_2
    assert heating_names_1 is heating_names_2
    assert dose_names_1 is dose_names_2

    assert len(photolysis_names_1) == 3, "Unexpected number of photolysis rates found"
    assert len(heating_names_1) == 2, "Unexpected number of heating rates found"
    assert len(dose_names_1) == 3, "Unexpected number of dose rates found"

    photolysis_rates, heating_rates, dose_rates = tuvx.run(2.3, 1.0)

    assert len(photolysis_rates) == 4, "Unexpected number of layers for photolysis rates"
    assert len(heating_rates) == 4, "Unexpected number of layers for heating rates"
    assert len(dose_rates) == 4, "Unexpected number of layers for dose rates"
    assert photolysis_rates.shape[1] == len(photolysis_names_1), "Photolysis rates shape mismatch"
    assert heating_rates.shape[1] == len(heating_names_1), "Heating rates shape mismatch"
    assert dose_rates.shape[1] == len(dose_names_1), "Dose rates shape mismatch"


def test_fixed_tuvx_from_string(monkeypatch):
    monkeypatch.chdir("src")
    file = "test/data/tuvx/full_from_host/config.json"
    with open(file, 'r') as f:
        config_str = f.read()
    grid_map = get_fixed_grid_map()
    profile_map = get_profile_map(grid_map)
    radiator_map = get_radiator_map(grid_map)
    tuvx = musica.TUVX(grid_map, profile_map, radiator_map, config_string=config_str)
    assert tuvx is not None

    # Access properties multiple times
    photolysis_names_1 = tuvx.photolysis_rate_names
    photolysis_names_2 = tuvx.photolysis_rate_names
    heating_names_1 = tuvx.heating_rate_names
    heating_names_2 = tuvx.heating_rate_names
    dose_names_1 = tuvx.dose_rate_names
    dose_names_2 = tuvx.dose_rate_names

    # Verify they return the same object (cached)
    assert photolysis_names_1 is photolysis_names_2
    assert heating_names_1 is heating_names_2
    assert dose_names_1 is dose_names_2

    assert len(photolysis_names_1) == 3, "Unexpected number of photolysis rates found"
    assert len(heating_names_1) == 2, "Unexpected number of heating rates found"
    assert len(dose_names_1) == 3, "Unexpected number of dose rates found"

    photolysis_rates, heating_rates, dose_rates = tuvx.run(2.3, 1.0)

    assert len(photolysis_rates) == 4, "Unexpected number of layers for photolysis rates"
    assert len(heating_rates) == 4, "Unexpected number of layers for heating rates"
    assert len(dose_rates) == 4, "Unexpected number of layers for dose rates"
    assert photolysis_rates.shape[1] == len(photolysis_names_1), "Photolysis rates shape mismatch"
    assert heating_rates.shape[1] == len(heating_names_1), "Heating rates shape mismatch"
    assert dose_rates.shape[1] == len(dose_names_1), "Dose rates shape mismatch"


def test_tuvx_initialization_errors():
    """Test error handling during TUVX initialization."""
    # Test with non-existent file
    grid_map = get_grid_map()
    profile_map = get_profile_map(grid_map)
    radiator_map = get_radiator_map(grid_map)
    with pytest.raises(FileNotFoundError):
        musica.TUVX(
            grid_map=grid_map,
            profile_map=profile_map,
            radiator_map=radiator_map,
            config_path="non_existent_config.json")


if __name__ == '__main__':
    pytest.main([__file__])
