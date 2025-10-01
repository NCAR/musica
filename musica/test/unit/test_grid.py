"""Tests for the Grid class."""
from __future__ import annotations
import pytest  # type: ignore
import numpy as np  # type: ignore
from musica.grid import Grid, backend

# Skip all tests if TUV-x is not available
pytestmark = pytest.mark.skipif(not backend.tuvx_available(),
                                reason="TUV-x backend is not available")


def test_grid_initialization():
    """Test Grid initialization with various input combinations."""
    # Test with num_sections
    grid = Grid(name="test", units="m", num_sections=5)
    assert grid.name == "test"
    assert grid.units == "m"
    assert grid.num_sections == 5

    # Test with edges
    edges = np.array([0, 1, 2, 3, 4, 5])
    grid = Grid(name="test", units="m", edges=edges)
    assert grid.name == "test"
    assert grid.units == "m"
    assert grid.num_sections == 5
    np.testing.assert_array_equal(grid.edges, edges)

    # Test with midpoints
    midpoints = np.array([0.5, 1.5, 2.5, 3.5, 4.5])
    grid = Grid(name="test", units="m", midpoints=midpoints)
    assert grid.name == "test"
    assert grid.units == "m"
    assert grid.num_sections == 5
    np.testing.assert_array_equal(grid.midpoints, midpoints)

    # Test with both edges and midpoints
    grid = Grid(name="test", units="m", edges=edges, midpoints=midpoints)
    assert grid.name == "test"
    assert grid.units == "m"
    assert grid.num_sections == 5
    np.testing.assert_array_equal(grid.edges, edges)
    np.testing.assert_array_equal(grid.midpoints, midpoints)

    # Test invalid initialization
    with pytest.raises(ValueError, match="At least one of num_sections, edges, or midpoints must be provided"):
        Grid(name="test", units="m")


def test_grid_properties():
    """Test Grid property getters and setters."""
    grid = Grid(name="test", units="m", num_sections=5)

    # Test edges
    edges = np.array([0, 1, 2, 3, 4, 5])
    grid.edges = edges
    np.testing.assert_array_equal(grid.edges, edges)

    # Test midpoints
    midpoints = np.array([0.5, 1.5, 2.5, 3.5, 4.5])
    grid.midpoints = midpoints
    np.testing.assert_array_equal(grid.midpoints, midpoints)

    # Test invalid edges size
    with pytest.raises(ValueError, match="Array size must be num_sections \\+ 1"):
        grid.edges = np.array([0, 1, 2])

    # Test invalid midpoints size
    with pytest.raises(ValueError, match="Array size must be num_sections"):
        grid.midpoints = np.array([0.5, 1.5, 2.5])


def test_grid_string_methods():
    """Test string representations of Grid."""
    grid = Grid(name="test", units="m", num_sections=5)
    edges = np.array([0, 1, 2, 3, 4, 5])
    grid.edges = edges

    # Test str()
    expected_str = "Grid(name=test, units=m, num_sections=5)"
    assert str(grid) == expected_str

    # Test repr()
    assert repr(grid).startswith("Grid(name=test, units=m, num_sections=5")
    assert "edges=" in repr(grid)
    assert "midpoints=" in repr(grid)


def test_grid_comparison():
    """Test Grid comparison methods."""
    grid1 = Grid(name="test", units="m", num_sections=5)
    grid1.edges = np.array([0, 1, 2, 3, 4, 5])

    # Test equal grids
    grid2 = Grid(name="test", units="m", num_sections=5)
    grid2.edges = np.array([0, 1, 2, 3, 4, 5])
    assert grid1 == grid2

    # Test unequal grids
    grid3 = Grid(name="test", units="m", num_sections=6)
    grid3.edges = np.array([0, 1, 2, 3, 4, 5, 6])
    assert grid1 != grid3

    grid4 = Grid(name="test", units="m", num_sections=5)
    grid4.edges = np.array([0, 2, 4, 6, 8, 10])
    assert grid1 != grid4

    grid5 = Grid(name="different", units="m", num_sections=5)
    grid5.edges = np.array([0, 1, 2, 3, 4, 5])
    assert grid1 != grid5

    grid6 = Grid(name="test", units="cm", num_sections=5)
    grid6.edges = np.array([0, 1, 2, 3, 4, 5])
    assert grid1 != grid6


def test_grid_container_methods():
    """Test Grid container methods (len, contains)."""
    grid = Grid(name="test", units="m", num_sections=5)
    edges = np.array([0, 1, 2, 3, 4, 5])
    grid.edges = edges

    # Test len()
    assert len(grid) == 5

    # Test contains
    assert 2.5 in grid
    assert -1 not in grid
    assert 6 not in grid


def test_grid_bool():
    """Test Grid boolean evaluation."""
    grid = Grid(name="test", units="m", num_sections=5)
    assert bool(grid) is True

    # Note: Cannot test num_sections=0 case as it's prevented by constructor
    # but the __bool__ method would return False in that case
