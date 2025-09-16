# Copyright (C) 2023-2025 National Center for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
Tests for the TUV-x GridMap class.
"""

import pytest
import numpy as np
from musica.grid import Grid, backend
from musica.grid_map import GridMap

# Skip all tests if TUV-x is not available
pytestmark = pytest.mark.skipif(not backend.tuvx_available(),
                                reason="TUV-x backend is not available")


def test_grid_map_creation():
    """Test creating a GridMap instance"""
    grid_map = GridMap()
    assert len(grid_map) == 0
    assert bool(grid_map) is False
    assert str(grid_map) == "GridMap(num_grids=0)"


def test_grid_map_add_get_grid():
    """Test adding and retrieving grids from the map"""
    grid_map = GridMap()

    # Create a grid to add
    grid = Grid(name="height", units="km", num_sections=5)
    edges = np.array([0, 2, 4, 6, 8, 10], dtype=np.float64)
    grid.edges = edges

    # Test dictionary-style assignment
    grid_map["height", "km"] = grid
    assert len(grid_map) == 1
    assert bool(grid_map) is True

    # Test dictionary-style access
    retrieved_grid = grid_map["height", "km"]
    assert retrieved_grid.name == "height"
    assert retrieved_grid.units == "km"
    assert retrieved_grid.num_sections == 5
    assert np.array_equal(retrieved_grid.edges, edges)


def test_grid_map_contains():
    """Test checking for grid existence"""
    grid_map = GridMap()
    grid = Grid(name="wavelength", units="nm", num_sections=3)

    grid_map["wavelength", "nm"] = grid

    assert ("wavelength", "nm") in grid_map
    assert ("height", "km") not in grid_map
    assert "invalid_key" not in grid_map


def test_grid_map_iteration():
    """Test iterating over grids in the map"""
    grid_map = GridMap()

    grids = [
        ("height", "km", 5),
        ("wavelength", "nm", 3),
        ("time", "s", 2)
    ]

    # Add grids to map
    for name, units, sections in grids:
        grid = Grid(name=name, units=units, num_sections=sections)
        grid_map[name, units] = grid

    # Test keys()
    keys = set(grid_map.keys())
    expected_keys = {(name, units) for name, units, _ in grids}
    assert keys == expected_keys

    # Test values()
    for grid in grid_map.values():
        assert isinstance(grid, Grid)
        assert (grid.name, grid.units) in expected_keys

    # Test items()
    for key, grid in grid_map.items():
        assert isinstance(grid, Grid)
        assert key in expected_keys
        assert key == (grid.name, grid.units)


def test_grid_map_clear():
    """Test clearing all grids from the map"""
    grid_map = GridMap()

    grid = Grid(name="height", units="km", num_sections=5)
    grid_map["height", "km"] = grid

    assert len(grid_map) == 1
    grid_map.clear()
    assert len(grid_map) == 0
    assert ("height", "km") not in grid_map


def test_grid_map_errors():
    """Test error handling"""
    grid_map = GridMap()
    grid = Grid(name="height", units="km", num_sections=5)

    # Test invalid key type
    with pytest.raises(TypeError):
        grid_map["invalid"] = grid

    with pytest.raises(TypeError):
        grid_map[123, "km"] = grid

    # Test invalid value type
    with pytest.raises(TypeError):
        grid_map["height", "km"] = "not_a_grid"

    # Test mismatched grid properties
    with pytest.raises(ValueError):
        grid_map["wrong", "units"] = grid

    # Test accessing non-existent grid
    with pytest.raises(KeyError):
        _ = grid_map["nonexistent", "grid"]
