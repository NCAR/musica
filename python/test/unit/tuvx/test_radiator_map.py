# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""Tests for the RadiatorMap class."""

import pytest
import numpy as np
from musica.tuvx.grid import Grid
from musica.tuvx.radiator import Radiator
from musica.tuvx.radiator_map import RadiatorMap, backend

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
    grid = Grid(name="wavelength", units="m", num_sections=4)
    grid.edges = np.array([200e-9, 300e-9, 400e-9, 500e-9, 600e-9], dtype=np.float64)
    grid.midpoints = np.array([250e-9, 350e-9, 450e-9, 550e-9], dtype=np.float64)
    return grid


def test_radiator_map_creation():
    """Test creating a RadiatorMap instance"""
    radiator_map = RadiatorMap()
    assert len(radiator_map) == 0
    assert bool(radiator_map) is False
    assert str(radiator_map) == "RadiatorMap(num_radiators=0)"


def test_radiator_map_add_get_radiator(sample_height_grid, sample_wavelength_grid):
    """Test adding and retrieving radiators from the map"""
    radiator_map = RadiatorMap()

    # Create a sample radiator
    radiator = Radiator(
        name="test_radiator",
        height_grid=sample_height_grid,
        wavelength_grid=sample_wavelength_grid,
        optical_depths=np.random.rand(4, 5),
        single_scattering_albedos=np.random.rand(4, 5),
        asymmetry_factors=np.random.rand(4, 5)
    )

    # Test dictionary-style assignment
    radiator_map["test_radiator"] = radiator
    assert len(radiator_map) == 1
    assert bool(radiator_map) is True

    # Test dictionary-style access
    retrieved_radiator = radiator_map["test_radiator"]
    assert retrieved_radiator.name == "test_radiator"
    np.testing.assert_array_equal(retrieved_radiator.optical_depths, radiator.optical_depths)
    np.testing.assert_array_equal(retrieved_radiator.single_scattering_albedos, radiator.single_scattering_albedos)
    np.testing.assert_array_equal(retrieved_radiator.asymmetry_factors, radiator.asymmetry_factors)


def test_radiator_map_contains(sample_height_grid, sample_wavelength_grid):
    """Test checking for radiator existence"""
    radiator_map = RadiatorMap()

    # Create a sample radiator
    radiator = Radiator(
        name="test_radiator",
        height_grid=sample_height_grid,
        wavelength_grid=sample_wavelength_grid
    )

    radiator_map["test_radiator"] = radiator
    assert ("test_radiator" in radiator_map) is True
    assert ("nonexistent_radiator" in radiator_map) is False


def test_radiator_map_iteration(sample_height_grid, sample_wavelength_grid):
    """Test iterating over radiators in the map"""
    radiator_map = RadiatorMap()

    # Create test radiators
    for i in range(3):
        radiator = Radiator(
            name=f"radiator_{i}",
            height_grid=sample_height_grid,
            wavelength_grid=sample_wavelength_grid
        )
        radiator_map[f"radiator_{i}"] = radiator

    # Test iteration
    names = set()
    for name, radiator in radiator_map.items():
        names.add(name)
        assert isinstance(radiator, Radiator)
    assert names == {"radiator_0", "radiator_1", "radiator_2"}

    # test keys
    keys = set(radiator_map.keys())
    assert keys == {"radiator_0", "radiator_1", "radiator_2"}

    # test values
    for radiator in radiator_map.values():
        assert isinstance(radiator, Radiator)
        assert radiator.name in {"radiator_0", "radiator_1", "radiator_2"}

    # test items
    for name, radiator in radiator_map.items():
        assert name in {"radiator_0", "radiator_1", "radiator_2"}
        assert isinstance(radiator, Radiator)


def test_radiator_map_clear(sample_height_grid, sample_wavelength_grid):
    """Test clearing the radiator map"""
    radiator_map = RadiatorMap()

    # Create a sample radiator
    radiator = Radiator(
        name="test_radiator",
        height_grid=sample_height_grid,
        wavelength_grid=sample_wavelength_grid
    )

    radiator_map["test_radiator"] = radiator
    assert len(radiator_map) == 1

    radiator_map.clear()
    assert len(radiator_map) == 0
    assert bool(radiator_map) is False


def test_radiator_map_error(sample_height_grid, sample_wavelength_grid):
    """Test error handling in RadiatorMap"""
    radiator_map = RadiatorMap()

    # Attempt to retrieve a nonexistent radiator
    with pytest.raises(KeyError):
        _ = radiator_map["nonexistent_radiator"]

    # Attempt to add an invalid radiator
    with pytest.raises(TypeError):
        radiator_map["invalid_radiator"] = "not_a_radiator"

    # Invalid key type
    with pytest.raises(TypeError):
        radiator_map[123] = Radiator(
            name="test_radiator",
            height_grid=sample_height_grid,
            wavelength_grid=sample_wavelength_grid
        )

    # Invalid value type
    with pytest.raises(TypeError):
        radiator_map["invalid_radiator"] = 456
