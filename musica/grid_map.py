# Copyright (C) 2023-2025 National Center for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x GridMap class.

This module provides a class for managing collections of TUV-x grids.
The GridMap class allows dictionary-style access to grids using (name, units) tuples as keys.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Iterator, Sequence
from . import backend
from .grid import Grid

_backend = backend.get_backend()

GridMap = _backend._tuvx._GridMap if backend.tuvx_available() else None

if backend.tuvx_available():
    original_init = GridMap.__init__

    def __init__(self, **kwargs):
        """Initialize a GridMap instance.

        Args:
            **kwargs: Additional arguments passed to the C++ constructor
        """
        original_init(self, **kwargs)

    GridMap.__init__ = __init__

    def __str__(self):
        """User-friendly string representation."""
        return f"GridMap(num_grids={len(self)})"

    GridMap.__str__ = __str__

    def __repr__(self):
        """Detailed string representation for debugging."""
        grid_details = []
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            grid_details.append(f"({grid.name}, {grid.units})")
        return f"GridMap(grids={grid_details})"

    GridMap.__repr__ = __repr__

    def __len__(self):
        """Return the number of grids in the map."""
        return self.get_number_of_grids()

    GridMap.__len__ = __len__

    def __bool__(self):
        """Return True if the map has any grids."""
        return len(self) > 0

    GridMap.__bool__ = __bool__

    def __getitem__(self, key) -> Grid:
        """Get a grid using dictionary-style access with (name, units) tuple as key.

        Args:
            key: A tuple of (grid_name, grid_units)

        Returns:
            The requested Grid object

        Raises:
            KeyError: If no grid matches the given name and units
            TypeError: If key is not a tuple of (str, str)
        """
        if not isinstance(key, tuple) or len(key) != 2:
            raise TypeError("Grid access requires a tuple of (name, units)")
        name, units = key
        try:
            return self.get_grid(name, units)
        except Exception as e:
            raise KeyError(f"No grid found with name='{name}' and units='{units}'") from e

    GridMap.__getitem__ = __getitem__

    def __setitem__(self, key, grid):
        """Add a grid to the map using dictionary-style access.

        Args:
            key: A tuple of (grid_name, grid_units)
            grid: The Grid object to add

        Raises:
            TypeError: If key is not a tuple, or if key components are not strings
            TypeError: If grid is not a Grid object
            ValueError: If grid name/units don't match the key
        """
        if not isinstance(key, tuple) or len(key) != 2:
            raise TypeError("Grid assignment requires a tuple of (name, units)")
        name, units = key
        if not isinstance(name, str):
            raise TypeError("Grid name must be a string")
        if not isinstance(units, str):
            raise TypeError("Grid units must be a string")
        if not isinstance(grid, Grid):
            raise TypeError("Value must be a Grid object")
        if grid.name != name or grid.units != units:
            raise ValueError("Grid name/units must match the key tuple")
        self.add_grid(grid)

    GridMap.__setitem__ = __setitem__

    def __iter__(self) -> Iterator:
        """Return an iterator over (name, units) tuples of all grids."""
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            yield (grid.name, grid.units)

    GridMap.__iter__ = __iter__

    def __contains__(self, key) -> bool:
        """Check if a grid with given name and units exists in the map.

        Args:
            key: A tuple of (grid_name, grid_units)

        Returns:
            True if a matching grid exists, False otherwise
        """
        if not isinstance(key, tuple) or len(key) != 2:
            return False
        name, units = key
        try:
            grid = self.get_grid(str(name), str(units))
            return grid is not None
        except (ValueError, KeyError):
            return False

    GridMap.__contains__ = __contains__

    def clear(self):
        """Remove all grids from the map."""
        while len(self) > 0:
            self.remove_grid_by_index(0)

    GridMap.clear = clear

    def items(self):
        """Return an iterator over (key, grid) pairs, where key is (name, units)."""
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            yield ((grid.name, grid.units), grid)

    GridMap.items = items

    def keys(self):
        """Return an iterator over grid keys (name, units) tuples."""
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            yield (grid.name, grid.units)

    GridMap.keys = keys

    def values(self):
        """Return an iterator over Grid objects in the map."""
        for i in range(len(self)):
            yield self.get_grid_by_index(i)

    GridMap.values = values
