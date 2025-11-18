# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""Tests for the Radiator class."""
from __future__ import annotations
import pytest  # type: ignore
import numpy as np  # type: ignore
from musica.tuvx.grid import Grid
from musica.tuvx.radiator import Radiator, backend

# Skip all tests if TUV-x is not available
pytestmark = pytest.mark.skipif(not backend.tuvx_available(),
                                reason="TUV-x backend is not available")


@pytest.fixture
def sample_height_grid():
    """Create a sample height grid for testing."""
    grid = Grid(name="height", units="m", num_sections=5)
    grid.edges = np.array([0, 2, 4, 6, 8, 10], dtype=np.float64)
    grid.midpoints = np.array([1, 3, 5, 7, 9], dtype=np.float64)
    return grid


@pytest.fixture
def sample_wavelength_grid():
    """Create a sample wavelength grid for testing."""
    grid = Grid(name="wavelength", units="m", num_sections=3)
    grid.edges = np.array([200e-9, 300e-9, 400e-9, 500e-9], dtype=np.float64)
    grid.midpoints = np.array([250e-9, 350e-9, 450e-9], dtype=np.float64)
    return grid


def test_radiator_initialization(sample_height_grid, sample_wavelength_grid):
    """Test Radiator initialization with various input combinations."""
    # Test basic initialization
    radiator = Radiator(name="test_radiator",
                        height_grid=sample_height_grid,
                        wavelength_grid=sample_wavelength_grid)
    assert radiator.name == "test_radiator"
    assert radiator.number_of_height_sections == sample_height_grid.num_sections
    assert radiator.number_of_wavelength_sections == sample_wavelength_grid.num_sections

    # Test with optical depth values (arrays ordered as [wavelength, height])
    optical_depth_values = np.array([[0.8, 0.85, 0.9, 0.95, 1.0],
                                     [0.75, 0.8, 0.85, 0.9, 0.95],
                                     [0.7, 0.75, 0.8, 0.85, 0.9]], dtype=np.float64)

    ssa_values = np.array([[0.9, 0.92, 0.94, 0.96, 0.98],
                           [0.88, 0.9, 0.92, 0.94, 0.96],
                           [0.86, 0.88, 0.9, 0.92, 0.94]], dtype=np.float64)

    asymmetry_values = np.array([[0.7, 0.72, 0.74, 0.76, 0.78],
                                 [0.68, 0.7, 0.72, 0.74, 0.76],
                                 [0.66, 0.68, 0.7, 0.72, 0.74]], dtype=np.float64)

    radiator = Radiator(name="test_radiator",
                        height_grid=sample_height_grid,
                        wavelength_grid=sample_wavelength_grid,
                        optical_depths=optical_depth_values,
                        single_scattering_albedos=ssa_values,
                        asymmetry_factors=asymmetry_values)
    assert radiator.name == "test_radiator"
    assert radiator.number_of_height_sections == sample_height_grid.num_sections
    assert radiator.number_of_wavelength_sections == sample_wavelength_grid.num_sections
    np.testing.assert_array_equal(radiator.optical_depths, optical_depth_values)
    np.testing.assert_array_equal(radiator.single_scattering_albedos, ssa_values)
    np.testing.assert_array_equal(radiator.asymmetry_factors, asymmetry_values)

    # test with uninitialized optional parameters
    radiator = Radiator(name="test_radiator",
                        height_grid=sample_height_grid,
                        wavelength_grid=sample_wavelength_grid)
    assert radiator.name == "test_radiator"
    assert radiator.number_of_height_sections == sample_height_grid.num_sections
    assert radiator.number_of_wavelength_sections == sample_wavelength_grid.num_sections
    np.testing.assert_array_equal(radiator.optical_depths, np.zeros((sample_wavelength_grid.num_sections,
                                                                     sample_height_grid.num_sections)))
    np.testing.assert_array_equal(radiator.single_scattering_albedos, np.zeros((sample_wavelength_grid.num_sections,
                                                                                sample_height_grid.num_sections)))
    np.testing.assert_array_equal(radiator.asymmetry_factors, np.zeros((sample_wavelength_grid.num_sections,
                                                                        sample_height_grid.num_sections)))


def test_radiator_properties(sample_height_grid, sample_wavelength_grid):
    """Test Radiator property getters and setters."""
    radiator = Radiator(name="test_radiator",
                        height_grid=sample_height_grid,
                        wavelength_grid=sample_wavelength_grid)

    # Test number_of_height_sections
    assert radiator.number_of_height_sections == sample_height_grid.num_sections

    # Test number_of_wavelength_sections
    assert radiator.number_of_wavelength_sections == sample_wavelength_grid.num_sections

    # Test optical_depth_values
    optical_depth_values = np.array([[0.8, 0.85, 0.9, 0.95, 1.0],
                                     [0.75, 0.8, 0.85, 0.9, 0.95],
                                     [0.7, 0.75, 0.8, 0.85, 0.9]], dtype=np.float64)
    radiator.optical_depths = optical_depth_values
    np.testing.assert_array_equal(radiator.optical_depths, optical_depth_values)

    # Test single_scattering_albedo_values
    ssa_values = np.array([[0.9, 0.92, 0.94, 0.96, 0.98],
                           [0.88, 0.9, 0.92, 0.94, 0.96],
                           [0.86, 0.88, 0.9, 0.92, 0.94]], dtype=np.float64)
    radiator.single_scattering_albedos = ssa_values
    np.testing.assert_array_equal(radiator.single_scattering_albedos, ssa_values)

    # Test asymmetry_factor_values
    asymmetry_values = np.array([[0.7, 0.72, 0.74, 0.76, 0.78],
                                 [0.68, 0.7, 0.72, 0.74, 0.76],
                                 [0.66, 0.68, 0.7, 0.72, 0.74]], dtype=np.float64)
    radiator.asymmetry_factors = asymmetry_values
    np.testing.assert_array_equal(radiator.asymmetry_factors, asymmetry_values)

    # test invalid shape assignment
    with pytest.raises(ValueError, match="Array shape must be"):
        radiator.optical_depths = np.array([[0.8, 0.85],
                                            [0.75, 0.8],
                                            [0.7, 0.75],
                                            [0.65, 0.7],
                                            [0.6, 0.65]], dtype=np.float64)
    with pytest.raises(ValueError, match="Array shape must be"):
        radiator.single_scattering_albedos = np.array([[0.9, 0.92],
                                                       [0.88, 0.9],
                                                       [0.86, 0.88],
                                                       [0.84, 0.86],
                                                       [0.82, 0.84]], dtype=np.float64)
    with pytest.raises(ValueError, match="Array shape must be"):
        radiator.asymmetry_factors = np.array([[0.7, 0.72],
                                               [0.68, 0.7],
                                               [0.66, 0.68],
                                               [0.64, 0.66],
                                               [0.62, 0.64]], dtype=np.float64)


def test_radiator_string_methods(sample_height_grid, sample_wavelength_grid):
    """Test Radiator string representation methods."""
    radiator = Radiator(name="test_radiator",
                        height_grid=sample_height_grid,
                        wavelength_grid=sample_wavelength_grid)

    expected_str = ("Radiator(name=test_radiator, "
                    "num_height_sections=5, "
                    "num_wavelength_sections=3)")
    assert str(radiator) == expected_str

    expected_repr = (f"Radiator(name=test_radiator, "
                     f"num_height_sections=5, "
                     f"num_wavelength_sections=3, "
                     f"optical_depths={radiator.optical_depths}, "
                     f"single_scattering_albedos={radiator.single_scattering_albedos}, "
                     f"asymmetry_factors={radiator.asymmetry_factors})")
    assert repr(radiator) == expected_repr


def test_radiator_comparison(sample_height_grid, sample_wavelength_grid):
    """Test Radiator comparison methods."""
    radiator1 = Radiator(name="test_radiator",
                         height_grid=sample_height_grid,
                         wavelength_grid=sample_wavelength_grid)
    radiator2 = Radiator(name="test_radiator",
                         height_grid=sample_height_grid,
                         wavelength_grid=sample_wavelength_grid)

    # Initially, both radiators should be equal
    assert radiator1 == radiator2

    # Modify optical depth values in radiator2
    optical_depth_values = np.array([[0.8, 0.85, 0.9, 0.95, 1.0],
                                    [0.75, 0.8, 0.85, 0.9, 0.95],
                                    [0.7, 0.75, 0.8, 0.85, 0.9]], dtype=np.float64)
    radiator2.optical_depths = optical_depth_values
    assert radiator1 != radiator2
    radiator1.optical_depths = optical_depth_values
    assert radiator1 == radiator2

    # Modify single scattering albedo values in radiator2
    ssa_values = np.array([[0.9, 0.92, 0.94, 0.96, 0.98],
                           [0.88, 0.9, 0.92, 0.94, 0.96],
                           [0.86, 0.88, 0.9, 0.92, 0.94]], dtype=np.float64)
    radiator2.single_scattering_albedos = ssa_values
    assert radiator1 != radiator2
    radiator1.single_scattering_albedos = ssa_values
    assert radiator1 == radiator2

    # Modify asymmetry factor values in radiator2
    asymmetry_values = np.array([[0.7, 0.72, 0.74, 0.76, 0.78],
                                 [0.68, 0.7, 0.72, 0.74, 0.76],
                                 [0.66, 0.68, 0.7, 0.72, 0.74]], dtype=np.float64)
    radiator2.asymmetry_factors = asymmetry_values
    assert radiator1 != radiator2
    radiator1.asymmetry_factors = asymmetry_values
    assert radiator1 == radiator2


def test_radiator_bool(sample_height_grid, sample_wavelength_grid):
    """Test Radiator boolean methods."""
    radiator = Radiator(name="test_radiator",
                        height_grid=sample_height_grid,
                        wavelength_grid=sample_wavelength_grid)
    assert bool(radiator) is True  # True if name and grids are set
