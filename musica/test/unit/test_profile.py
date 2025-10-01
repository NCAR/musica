"""Tests for the Profile class."""
from __future__ import annotations
import pytest  # type: ignore
import numpy as np  # type: ignore
from musica.grid import Grid
from musica.profile import Profile, backend

# Skip all tests if TUV-x is not available
pytestmark = pytest.mark.skipif(not backend.tuvx_available(),
                                reason="TUV-x backend is not available")


@pytest.fixture
def sample_grid():
    """Create a sample grid for testing."""
    grid = Grid(name="test_grid", units="m", num_sections=5)
    grid.edges = np.array([0, 1, 2, 3, 4, 5])
    grid.midpoints = np.array([0.5, 1.5, 2.5, 3.5, 4.5])
    return grid


def test_profile_initialization(sample_grid):
    """Test Profile initialization with various input combinations."""
    # Test basic initialization
    profile = Profile(name="test", units="K", grid=sample_grid)
    assert profile.name == "test"
    assert profile.units == "K"

    # Test number_of_sections
    assert profile.number_of_sections == 5

    # Test with edge_values
    edge_values = np.array([290, 280, 270, 260, 250, 240])
    profile = Profile(name="test", units="K", grid=sample_grid, edge_values=edge_values)
    np.testing.assert_array_equal(profile.edge_values, edge_values)

    # Test with midpoint_values
    midpoint_values = np.array([285, 275, 265, 255, 245])
    profile = Profile(name="test", units="K", grid=sample_grid, midpoint_values=midpoint_values)
    np.testing.assert_array_equal(profile.midpoint_values, midpoint_values)

    # Test with layer_densities
    layer_densities = np.array([1e25, 1e24, 1e23, 1e22, 1e21])
    profile = Profile(name="test", units="K", grid=sample_grid, layer_densities=layer_densities)
    np.testing.assert_array_equal(profile.layer_densities, layer_densities)

    # Test with all values
    profile = Profile(name="test", units="K", grid=sample_grid,
                      edge_values=edge_values,
                      midpoint_values=midpoint_values,
                      layer_densities=layer_densities)
    np.testing.assert_array_equal(profile.edge_values, edge_values)
    np.testing.assert_array_equal(profile.midpoint_values, midpoint_values)
    np.testing.assert_array_equal(profile.layer_densities, layer_densities)


def test_profile_properties(sample_grid):
    """Test Profile property getters and setters."""
    profile = Profile(name="test", units="K", grid=sample_grid)

    # Test number_of_sections
    assert profile.number_of_sections == 5

    # Test edge_values
    edge_values = np.array([290, 280, 270, 260, 250, 240])
    profile.edge_values = edge_values
    np.testing.assert_array_equal(profile.edge_values, edge_values)

    # Test midpoint_values
    midpoint_values = np.array([285, 275, 265, 255, 245])
    profile.midpoint_values = midpoint_values
    np.testing.assert_array_equal(profile.midpoint_values, midpoint_values)

    # Test layer_densities
    layer_densities = np.array([1e25, 1e24, 1e23, 1e22, 1e21])
    profile.layer_densities = layer_densities
    np.testing.assert_array_equal(profile.layer_densities, layer_densities)

    # Test exo_layer_density
    profile.exo_layer_density = 1e20
    assert profile.exo_layer_density == 1e20

    # Test invalid edge_values size
    with pytest.raises(ValueError, match="Array size must be num_sections \\+ 1"):
        profile.edge_values = np.array([290, 280, 270])

    # Test invalid midpoint_values size
    with pytest.raises(ValueError, match="Array size must be num_sections"):
        profile.midpoint_values = np.array([285, 275, 265])

    # Test invalid layer_densities size
    with pytest.raises(ValueError, match="Array size must be num_sections"):
        profile.layer_densities = np.array([1e25, 1e24, 1e23])


def test_profile_string_methods(sample_grid):
    """Test string representations of Profile."""
    profile = Profile(name="test", units="K", grid=sample_grid)
    edge_values = np.array([290, 280, 270, 260, 250, 240])
    profile.edge_values = edge_values

    # Test str()
    assert str(profile).startswith("Profile(name=test, units=K, number_of_sections=5)")

    # Test repr()
    repr_str = repr(profile)
    assert repr_str.startswith("Profile(name=test, units=K, number_of_sections=5")
    assert "edge_values=" in repr_str
    assert "midpoint_values=" in repr_str
    assert "layer_densities=" in repr_str
    assert "exo_layer_density=" in repr_str


def test_profile_comparison(sample_grid):
    """Test Profile comparison methods."""
    edge_values1 = np.array([290, 280, 270, 260, 250, 240])
    profile1 = Profile(name="test", units="K", grid=sample_grid)
    profile1.edge_values = edge_values1

    # Test equal profiles
    profile2 = Profile(name="test", units="K", grid=sample_grid)
    profile2.edge_values = edge_values1
    assert profile1 == profile2

    # Test unequal profiles
    profile3 = Profile(name="different", units="K", grid=sample_grid)
    profile3.edge_values = edge_values1
    assert profile1 != profile3

    profile4 = Profile(name="test", units="°C", grid=sample_grid)
    profile4.edge_values = edge_values1
    assert profile1 != profile4

    edge_values2 = np.array([295, 285, 275, 265, 255, 245])
    profile5 = Profile(name="test", units="K", grid=sample_grid)
    profile5.edge_values = edge_values2
    assert profile1 != profile5


def test_profile_container_methods(sample_grid):
    """Test Profile container methods (len)."""
    profile = Profile(name="test", units="K", grid=sample_grid)
    edge_values = np.array([290, 280, 270, 260, 250, 240])
    profile.edge_values = edge_values

    # Test len()
    assert len(profile) == 5


def test_profile_bool(sample_grid):
    """Test Profile boolean evaluation."""
    profile = Profile(name="test", units="K", grid=sample_grid)
    assert bool(profile) is True  # True if grid has sections


def test_exo_layer_density_calculation(sample_grid):
    """Test exospheric layer density calculation."""
    profile = Profile(name="test", units="K", grid=sample_grid)
    edge_values = np.array([290, 280, 270, 260, 250, 240])
    profile.edge_values = edge_values

    layer_densities = np.array([1e25, 1e24, 1e23, 1e22, 1e21])
    profile.layer_densities = layer_densities

    # Test calculation with scale height
    scale_height = 7000  # meters
    profile.calculate_exo_layer_density(scale_height)
    assert profile.exo_layer_density == pytest.approx(240.0 * scale_height, rel=1e-2)
    assert profile.layer_densities[4] == pytest.approx(1.0e21 + 240.0 * scale_height, rel=1e-2)
