"""
Integration tests for the TUV-x radiation field updater.

test_host_supplied_radiation_field_drives_photolysis_rates mirrors tuv-x's own
test/unit/radiative_transfer/solver_from_host.F90 (test_core_with_host_radiation_field) and
the C++ reference test in src/test/unit/tuvx/tuvx_c_api.cpp
(HostSuppliedRadiationFieldDrivesPhotolysisRates), using the same config and the same
hand-computed reference formula, translated to the Python API. The config and reference
values come directly from that test, not reconstructed by hand.
"""

import pytest
import numpy as np
import musica
from musica.utils import find_config_path

available = musica.backend.tuvx_available()
pytestmark = pytest.mark.skipif(not available, reason="TUV-x backend is not available")


def _get_fixed_grid_map():
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


def _get_profile_map(grid_map):
    normalized_profile = np.exp(-(grid_map["height", "km"].midpoints - 0.5) / 25)
    midpoints = 1.0e-6 * 2.54e19 * normalized_profile
    ozone = musica.Profile(name="O3", units="molecule cm-3", grid=grid_map["height", "km"],
                           midpoint_values=midpoints, calculate_layer_densities=True)
    midpoints = 2.54e19 * normalized_profile
    air = musica.Profile(name="air", units="molecule cm-3", grid=grid_map["height", "km"],
                         midpoint_values=midpoints, calculate_layer_densities=True)
    midpoints = 0.21 * 2.54e19 * normalized_profile
    oxygen = musica.Profile(name="O2", units="molecule cm-3", grid=grid_map["height", "km"],
                            midpoint_values=midpoints, calculate_layer_densities=True)
    ozone.calculate_exo_layer_density(8.5)
    oxygen.calculate_exo_layer_density(8.5)
    air.calculate_exo_layer_density(8.5)
    midpoints = 298.0 * normalized_profile
    temperature = musica.Profile(name="temperature", units="K", grid=grid_map["height", "km"],
                                 midpoint_values=midpoints)
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


def _get_radiator_map(grid_map):
    ssa = 0.99 * np.ones((grid_map["wavelength", "nm"].num_sections, grid_map["height", "km"].num_sections))
    asymmetry = 0.61 * np.ones((grid_map["wavelength", "nm"].num_sections, grid_map["height", "km"].num_sections))
    od = np.tile(1.0e-6 * np.exp(-(grid_map["height", "km"].midpoints - 120) / 7),
                 (grid_map["wavelength", "nm"].num_sections, 1))
    clouds = musica.Radiator(name="clouds", height_grid=grid_map["height", "km"],
                             wavelength_grid=grid_map["wavelength", "nm"], optical_depths=od,
                             single_scattering_albedos=ssa, asymmetry_factors=asymmetry)
    radiator_map = musica.RadiatorMap()
    radiator_map["clouds"] = clouds
    return radiator_map


def test_no_radiation_field_updater_for_non_host_solver():
    # This config uses tuv-x's "delta eddington" solver, not "from host".
    file = find_config_path("tuvx", "full_from_host", "config_python.json")
    grid_map = _get_fixed_grid_map()
    profile_map = _get_profile_map(grid_map)
    radiator_map = _get_radiator_map(grid_map)
    tuvx = musica.TUVX(grid_map, profile_map, radiator_map, config_path=file)

    assert tuvx.get_radiation_field_updater() is None


def test_host_supplied_radiation_field_drives_photolysis_rates():
    file = find_config_path("tuvx", "host_radiation_field", "config.json")
    grid_map = musica.GridMap()
    profile_map = musica.ProfileMap()
    radiator_map = musica.RadiatorMap()
    tuvx = musica.TUVX(grid_map, profile_map, radiator_map, config_path=file)

    updater = tuvx.get_radiation_field_updater()
    assert updater is not None

    n_interfaces = 5  # height grid: 1 to 5 km, delta 1 km -> 4 cells + 1
    n_bins = 6  # wavelength grid: 400 to 700 nm, delta 50 nm -> 6 cells
    etfl = 1.0e14  # photon cm^-2 s^-1, every bin
    xsqy = {"jfoo": 2.0 * 0.5, "jbar": 4.0 * 0.25}  # cross section * quantum yield
    lambda_bins = np.array([425.0, 475.0, 525.0, 575.0, 625.0, 675.0])  # wavelength bin midpoints [nm]
    # dose rate "all bins" weights every bin; "upper bins" weights only bins above 500 nm
    dose_weights = {
        "all bins": np.ones(n_bins),
        "upper bins": (lambda_bins > 500.0).astype(float),
    }
    earth_sun_distance = 0.9
    hc = 6.626068e-34 * 2.99792458e8  # Planck's constant * speed of light [J m], from tuv-x's tuvx_constants
    sza_radians = 42.0 * np.pi / 180.0

    # Arrays have shape (num_wavelength_bins, num_vertical_interfaces); interface 0 is the
    # lowest altitude. 1-based interface/bin numbers match tuv-x's own test exactly.
    interfaces, bins = np.meshgrid(np.arange(1, n_interfaces + 1), np.arange(1, n_bins + 1))
    direct_actinic_flux = 0.2 * interfaces + 0.05 * bins
    upward_actinic_flux = 0.01 * interfaces + 0.0 * bins
    downward_actinic_flux = 0.03 * bins + 0.0 * interfaces
    direct_irradiance = 0.1 * interfaces + 0.02 * bins
    upward_irradiance = 0.004 * interfaces + 0.0 * bins
    downward_irradiance = 0.006 * bins + 0.0 * interfaces

    # TUV-x forms the photolysis rate constants from the actinic flux components alone, so
    # this call supplies only those three -- the irradiance components default to zero, so
    # the dose rates come back zero.
    updater.update(direct_actinic_flux, upward_actinic_flux, downward_actinic_flux)

    dataset = tuvx.run(sza_radians, earth_sun_distance)

    total_flux_per_interface = (direct_actinic_flux + upward_actinic_flux + downward_actinic_flux).sum(axis=0)
    for name, xsqy_value in xsqy.items():
        expected = etfl * earth_sun_distance * xsqy_value * total_flux_per_interface
        actual = dataset["photolysis_rate_constants"].sel(reaction=name).values
        np.testing.assert_allclose(actual, expected, rtol=1.0e-8, err_msg=f"reaction {name}")

    assert dataset["dose_rates"].values == pytest.approx(0.0, abs=1.0e-8), \
        "dose rates must be zero when irradiance is omitted"

    # A second call that also supplies the irradiance components must produce non-zero dose
    # rates, matching the exact energy-flux formula.
    updater.update(
        direct_actinic_flux,
        upward_actinic_flux,
        downward_actinic_flux,
        direct_irradiance=direct_irradiance,
        upward_irradiance=upward_irradiance,
        downward_irradiance=downward_irradiance,
    )

    dataset = tuvx.run(sza_radians, earth_sun_distance)

    total_irradiance_per_bin_and_interface = direct_irradiance + upward_irradiance + downward_irradiance
    for name, weights in dose_weights.items():
        expected = (
            total_irradiance_per_bin_and_interface
            * earth_sun_distance
            * etfl
            * (hc / (lambda_bins * 1.0e-13))[:, np.newaxis]
            * weights[:, np.newaxis]
        ).sum(axis=0)
        actual = dataset["dose_rates"].sel(dose_rate=name).values
        np.testing.assert_allclose(actual, expected, rtol=1.0e-8, err_msg=f"dose rate {name}")


def test_update_rejects_mismatched_shape():
    file = find_config_path("tuvx", "host_radiation_field", "config.json")
    grid_map = musica.GridMap()
    profile_map = musica.ProfileMap()
    radiator_map = musica.RadiatorMap()
    tuvx = musica.TUVX(grid_map, profile_map, radiator_map, config_path=file)

    updater = tuvx.get_radiation_field_updater()
    assert updater is not None

    wrong_shape = np.zeros((3, 3))
    with pytest.raises(ValueError):
        updater.update(wrong_shape, wrong_shape, wrong_shape)
