import pytest
import musica
import numpy as np

available = musica.backend.tuvx_available()
pytestmark = pytest.mark.skipif(not available, reason="TUV-x backend is not available")


def get_grid_map():
    heights = musica.tuvx.v54.height_grid()
    wavelengths = musica.tuvx.v54.wavelength_grid()
    grid_map = musica.GridMap()
    grid_map["height", "km"] = heights
    grid_map["wavelength", "nm"] = wavelengths
    return grid_map


def get_profile_map(grid_map):
    # Simple exponential profiles for testing
    normalized_profile = np.exp(-(grid_map["height", "km"].midpoints - 0.5) / 25)
    midpoints = 1.0e-6 * 2.54e19 * normalized_profile
    ozone = musica.Profile(name="O3",
                           units="molecule cm-3",
                           grid=grid_map["height",
                                         "km"],
                           midpoint_values=midpoints,
                           calculate_layer_densities=True)
    midpoints = 2.54e19 * normalized_profile
    air = musica.Profile(name="air",
                         units="molecule cm-3",
                         grid=grid_map["height",
                                       "km"],
                         midpoint_values=midpoints,
                         calculate_layer_densities=True)
    midpoints = 0.21 * 2.54e19 * normalized_profile
    oxygen = musica.Profile(name="O2",
                            units="molecule cm-3",
                            grid=grid_map["height",
                                          "km"],
                            midpoint_values=midpoints,
                            calculate_layer_densities=True)
    ozone.calculate_exo_layer_density(8.5)
    oxygen.calculate_exo_layer_density(8.5)
    air.calculate_exo_layer_density(8.5)
    midpoints = 298.0 * normalized_profile
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

    sza = 0.3  # Radians
    earth_sun_distance = 1.0  # AU

    dataset = tuvx.run(sza, earth_sun_distance)

    assert len(dataset["heating_rates"].coords["heating_rate"]
               ) == 0, "Heating rate names should be empty for this config"
    assert len(dataset["photolysis_rate_constants"].coords["reaction"]) > 0, "No photolysis rate names found"
    assert len(dataset["dose_rates"].coords["dose_rate"]) > 0, "No dose rate names found"
    assert len(dataset["heating_rates"].coords["vertical_edge"]
               ) == 121, "Number of layers should be 121 for heating rates"
    assert len(dataset["photolysis_rate_constants"].coords["vertical_edge"]
               ) == 121, "Number of layers should be 121 for photolysis rates"
    assert len(dataset["dose_rates"].coords["vertical_edge"]) == 121, "Number of layers should be 121 for dose rates"
    assert dataset["heating_rates"].shape[1] == 0, "Should be no heating rates for this config"
    assert dataset["photolysis_rate_constants"].shape[1] == len(
        tuvx.photolysis_rate_names), "Photolysis rates shape mismatch"
    assert dataset["dose_rates"].shape[1] == len(tuvx.dose_rate_names), "Dose rates shape mismatch"

    # Make sure photolysis rates are reasonable
    assert np.all(dataset["photolysis_rate_constants"] >= 0), "Negative photolysis rates found"
    assert np.any(dataset["photolysis_rate_constants"] > 0), "All photolysis rates are zero"

    # Double all the species concentrations and verify rates decrease
    profile_map["O3", "molecule cm-3"].midpoint_values *= 2.0
    profile_map["O2", "molecule cm-3"].midpoint_values *= 2.0
    profile_map["air", "molecule cm-3"].midpoint_values *= 2.0
    profile_map["O3", "molecule cm-3"].calculate_layer_densities(grid_map["height", "km"])
    profile_map["O2", "molecule cm-3"].calculate_layer_densities(grid_map["height", "km"])
    profile_map["air", "molecule cm-3"].calculate_layer_densities(grid_map["height", "km"])
    profile_map["O3", "molecule cm-3"].calculate_exo_layer_density(8.5)
    profile_map["O2", "molecule cm-3"].calculate_exo_layer_density(8.5)
    profile_map["air", "molecule cm-3"].calculate_exo_layer_density(8.5)
    dataset_doubled = tuvx.run(sza, earth_sun_distance)

    # Verify that the photolysis of O2 and O3 decreased
    o2_index = tuvx.photolysis_rate_names["O2+hv->O+O"]
    o3_o1d_index = tuvx.photolysis_rate_names["O3+hv->O2+O(1D)"]
    o3_o3p_index = tuvx.photolysis_rate_names["O3+hv->O2+O(3P)"]
    assert np.all(dataset_doubled["photolysis_rate_constants"][:, o2_index] <=
                  dataset["photolysis_rate_constants"][:, o2_index]), "O2 photolysis did not decrease"
    assert np.all(dataset_doubled["photolysis_rate_constants"][:, o3_o1d_index] <=
                  dataset["photolysis_rate_constants"][:, o3_o1d_index]), "O3 (1D) photolysis did not decrease"
    assert np.all(dataset_doubled["photolysis_rate_constants"][:, o3_o3p_index] <=
                  dataset["photolysis_rate_constants"][:, o3_o3p_index]), "O3 (3P) photolysis did not decrease"

    # Verify the labels in the XArray dataset are ordered the same as in the TUV-x label dicts
    for i, reaction in enumerate(tuvx.photolysis_rate_names):
        assert dataset["photolysis_rate_constants"].coords["reaction"][i].item(
        ) == reaction, "Photolysis rate names order mismatch"
    for i, dose_rate in enumerate(tuvx.dose_rate_names):
        assert dataset["dose_rates"].coords["dose_rate"][i].item() == dose_rate, "Dose rate names order mismatch"
    for i, heating_rate in enumerate(tuvx.heating_rate_names):
        assert dataset["heating_rates"].coords["heating_rate"][i].item(
        ) == heating_rate, "Heating rate names order mismatch"

    # Check the SZA and Earth-Sun distance values
    assert np.isclose(dataset["solar_zenith_angle"].item(), sza), "Solar zenith angle value mismatch"
    assert np.isclose(dataset["earth_sun_distance"].item(), earth_sun_distance), "Earth-Sun distance value mismatch"

    # Check the dimensions, units, and values for the height and wavelength grids
    height_midpoints = dataset["vertical_midpoint"].values
    height_edges = dataset["vertical_edge"].values
    wavelength_midpoints = dataset["wavelength_midpoint"].values
    wavelength_edges = dataset["wavelength_edge"].values
    np.testing.assert_array_almost_equal(height_midpoints, grid_map["height", "km"].midpoints, decimal=6,
                                         err_msg="Height midpoints values mismatch")
    np.testing.assert_array_almost_equal(height_edges, grid_map["height", "km"].edges, decimal=6,
                                         err_msg="Height edges values mismatch")
    np.testing.assert_array_almost_equal(wavelength_midpoints, grid_map["wavelength", "nm"].midpoints, decimal=6,
                                         err_msg="Wavelength midpoints values mismatch")
    np.testing.assert_array_almost_equal(wavelength_edges, grid_map["wavelength", "nm"].edges, decimal=6,
                                         err_msg="Wavelength edges values mismatch")
    assert dataset["vertical_midpoint"].attrs["units"] == "km", "Height midpoints units mismatch"
    assert dataset["vertical_edge"].attrs["units"] == "km", "Height edges units mismatch"
    assert dataset["wavelength_midpoint"].attrs["units"] == "nm", "Wavelength midpoints units mismatch"
    assert dataset["wavelength_edge"].attrs["units"] == "nm", "Wavelength edges units mismatch"


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


def test_fixed_tuvx_from_file():
    file = "configs/tuvx/full_from_host/config.json"
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

    dataset = tuvx.run(2.3, 1.0)

    assert len(dataset["photolysis_rate_constants"].coords["vertical_edge"]
               ) == 4, "Unexpected number of layers for photolysis rates"
    assert len(dataset["heating_rates"].coords["vertical_edge"]) == 4, "Unexpected number of layers for heating rates"
    assert len(dataset["dose_rates"].coords["vertical_edge"]) == 4, "Unexpected number of layers for dose rates"
    assert dataset["photolysis_rate_constants"].shape[1] == len(photolysis_names_1), "Photolysis rates shape mismatch"
    assert dataset["heating_rates"].shape[1] == len(heating_names_1), "Heating rates shape mismatch"
    assert dataset["dose_rates"].shape[1] == len(dose_names_1), "Dose rates shape mismatch"


def test_fixed_tuvx_from_string():
    file = "configs/tuvx/full_from_host/config.json"
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

    dataset = tuvx.run(2.3, 1.0)

    assert len(dataset["photolysis_rate_constants"].coords["vertical_edge"]
               ) == 4, "Unexpected number of layers for photolysis rates"
    assert len(dataset["heating_rates"].coords["vertical_edge"]) == 4, "Unexpected number of layers for heating rates"
    assert len(dataset["dose_rates"].coords["vertical_edge"]) == 4, "Unexpected number of layers for dose rates"
    assert dataset["photolysis_rate_constants"].shape[1] == len(photolysis_names_1), "Photolysis rates shape mismatch"
    assert dataset["heating_rates"].shape[1] == len(heating_names_1), "Heating rates shape mismatch"
    assert dataset["dose_rates"].shape[1] == len(dose_names_1), "Dose rates shape mismatch"


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
