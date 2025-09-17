# Copyright (C) 2023-2025 National Center for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
Tests for the TUV-x ProfileMap class.
"""

import pytest
import numpy as np
from musica.grid import Grid
from musica.profile import Profile
from musica.profile_map import ProfileMap, backend

# Skip all tests if TUV-x is not available
pytestmark = pytest.mark.skipif(not backend.tuvx_available(),
                                reason="TUV-x backend is not available")


@pytest.fixture
def sample_grid():
    """Create a sample grid for testing."""
    grid = Grid(name="height", units="km", num_sections=5)
    grid.edges = np.array([0, 2, 4, 6, 8, 10], dtype=np.float64)
    grid.midpoints = np.array([1, 3, 5, 7, 9], dtype=np.float64)
    return grid


@pytest.fixture
def sample_profile(sample_grid):
    """Create a sample profile for testing."""
    profile = Profile(name="temperature", units="K", grid=sample_grid)
    profile.edge_values = np.array([300, 290, 280, 270, 260, 250], dtype=np.float64)
    profile.midpoint_values = np.array([295, 285, 275, 265, 255], dtype=np.float64)
    return profile


def test_profile_map_creation():
    """Test creating a ProfileMap instance"""
    profile_map = ProfileMap()
    assert len(profile_map) == 0
    assert bool(profile_map) is False
    assert str(profile_map) == "ProfileMap(num_profiles=0)"


def test_profile_map_add_get_profile(sample_grid, sample_profile):
    """Test adding and retrieving profiles from the map"""
    profile_map = ProfileMap()

    # Test dictionary-style assignment
    profile_map["temperature", "K"] = sample_profile
    assert len(profile_map) == 1
    assert bool(profile_map) is True

    # Test dictionary-style access
    retrieved_profile = profile_map["temperature", "K"]
    assert retrieved_profile.name == "temperature"
    assert retrieved_profile.units == "K"
    assert np.array_equal(retrieved_profile.edge_values, sample_profile.edge_values)
    assert np.array_equal(retrieved_profile.midpoint_values, sample_profile.midpoint_values)


def test_profile_map_contains(sample_grid, sample_profile):
    """Test checking for profile existence"""
    profile_map = ProfileMap()
    profile_map["temperature", "K"] = sample_profile

    assert ("temperature", "K") in profile_map
    assert ("pressure", "hPa") not in profile_map
    assert "invalid_key" not in profile_map


def test_profile_map_iteration(sample_grid):
    """Test iterating over profiles in the map"""
    profile_map = ProfileMap()

    # Create test profiles
    profiles = [
        ("temperature", "K", [300, 290, 280, 270, 260, 250]),
        ("pressure", "hPa", [1000, 800, 600, 400, 200, 100]),
        ("density", "kg/m3", [1.2, 1.0, 0.8, 0.6, 0.4, 0.2])
    ]

    # Add profiles to map
    for name, units, edge_values in profiles:
        profile = Profile(name=name, units=units, grid=sample_grid)
        profile.edge_values = np.array(edge_values, dtype=np.float64)
        profile_map[name, units] = profile

    # Test keys()
    keys = set(profile_map.keys())
    expected_keys = {(name, units) for name, units, _ in profiles}
    assert keys == expected_keys

    # Test values()
    for profile in profile_map.values():
        assert isinstance(profile, Profile)
        assert (profile.name, profile.units) in expected_keys

    # Test items()
    for key, profile in profile_map.items():
        assert isinstance(profile, Profile)
        assert key in expected_keys
        assert key == (profile.name, profile.units)


def test_profile_map_clear(sample_grid, sample_profile):
    """Test clearing all profiles from the map"""
    profile_map = ProfileMap()
    profile_map["temperature", "K"] = sample_profile

    assert len(profile_map) == 1
    profile_map.clear()
    assert len(profile_map) == 0
    assert ("temperature", "K") not in profile_map


def test_profile_map_errors(sample_grid, sample_profile):
    """Test error handling"""
    profile_map = ProfileMap()

    # Test invalid key type
    with pytest.raises(TypeError):
        profile_map["invalid"] = sample_profile

    with pytest.raises(TypeError):
        profile_map[123, "K"] = sample_profile

    # Test invalid value type
    with pytest.raises(TypeError):
        profile_map["temperature", "K"] = "not_a_profile"

    # Test mismatched profile properties
    with pytest.raises(ValueError):
        profile_map["wrong", "units"] = sample_profile

    # Test accessing non-existent profile
    with pytest.raises(KeyError):
        _ = profile_map["nonexistent", "profile"]
