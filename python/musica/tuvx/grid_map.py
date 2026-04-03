# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x GridMap class.

This module provides a class for managing collections of TUV-x grids.
The GridMap class allows dictionary-style access to grids using (name, units) tuples as keys.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Iterator
from .. import backend
from .._base import CppWrapper, _unwrap
from .grid import Grid

_backend = backend.get_backend()
_GridMap = _backend._tuvx._GridMap if backend.tuvx_available() else None


class GridMap(CppWrapper):
    """A collection of TUV-x grids with dictionary-style access.

    Grids are accessed using ``(name, units)`` tuples as keys.
    """

    def __init__(self, **kwargs):
        """Initialize a GridMap instance.

        Args:
            **kwargs: Additional arguments passed to the C++ constructor.
        """
        if not backend.tuvx_available():
            raise ValueError("TUV-x backend is not available.")
        self._cpp = _GridMap(**kwargs)

    def add_grid(self, grid: Grid):
        """Add a grid to the map."""
        self._cpp.add_grid(_unwrap(grid))

    def get_grid(self, name: str, units: str) -> Grid:
        """Get a grid by name and units."""
        return Grid._from_cpp(self._cpp.get_grid(name, units))

    def get_grid_by_index(self, index: int) -> Grid:
        """Get a grid by its index."""
        return Grid._from_cpp(self._cpp.get_grid_by_index(index))

    def get_number_of_grids(self) -> int:
        """Return the number of grids in the map."""
        return self._cpp.get_number_of_grids()

    def remove_grid_by_index(self, index: int):
        """Remove a grid by its index."""
        self._cpp.remove_grid_by_index(index)

    def __str__(self):
        """User-friendly string representation."""
        return f"GridMap(num_grids={len(self)})"

    def __repr__(self):
        """Detailed string representation for debugging."""
        grid_details = []
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            grid_details.append(f"({grid.name}, {grid.units})")
        return f"GridMap(grids={grid_details})"

    def __len__(self):
        """Return the number of grids in the map."""
        return self.get_number_of_grids()

    def __bool__(self):
        """Return True if the map has any grids."""
        return len(self) > 0

    def __getitem__(self, key) -> Grid:
        """Get a grid using dictionary-style access with (name, units) tuple as key.

        Args:
            key: A tuple of (grid_name, grid_units).

        Returns:
            The requested Grid object.

        Raises:
            KeyError: If no grid matches the given name and units.
            TypeError: If key is not a tuple of (str, str).
        """
        if not isinstance(key, tuple) or len(key) != 2:
            raise TypeError("Grid access requires a tuple of (name, units)")
        name, units = key
        try:
            return self.get_grid(name, units)
        except Exception as e:
            raise KeyError(f"No grid found with name='{name}' and units='{units}'") from e

    def __setitem__(self, key, grid):
        """Add a grid to the map using dictionary-style access.

        Args:
            key: A tuple of (grid_name, grid_units).
            grid: The Grid object to add.

        Raises:
            TypeError: If key is not a tuple, or if key components are not strings.
            TypeError: If grid is not a Grid object.
            ValueError: If grid name/units don't match the key.
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
            raise ValueError(f"Grid name/units must match the key tuple: {(grid.name, grid.units)} != {(name, units)}")
        self.add_grid(grid)

    def __iter__(self) -> Iterator:
        """Return an iterator over (name, units) tuples of all grids."""
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            yield (grid.name, grid.units)

    def __contains__(self, key) -> bool:
        """Check if a grid with given name and units exists in the map.

        Args:
            key: A tuple of (grid_name, grid_units).

        Returns:
            True if a matching grid exists, False otherwise.
        """
        if not isinstance(key, tuple) or len(key) != 2:
            return False
        name, units = key
        try:
            grid = self.get_grid(str(name), str(units))
            return grid is not None
        except (ValueError, KeyError):
            return False

    def clear(self):
        """Remove all grids from the map."""
        while len(self) > 0:
            self.remove_grid_by_index(0)

    def items(self):
        """Return an iterator over (key, grid) pairs, where key is (name, units)."""
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            yield ((grid.name, grid.units), grid)

    def keys(self):
        """Return an iterator over grid keys (name, units) tuples."""
        for i in range(len(self)):
            grid = self.get_grid_by_index(i)
            yield (grid.name, grid.units)

    def values(self):
        """Return an iterator over Grid objects in the map."""
        for i in range(len(self)):
            yield self.get_grid_by_index(i)
