# Copyright (C) 2023-2025 National Center for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x Grid class.

This module provides a class for defining grids on which TUV-x profiles exist.
Typically, this would be used to define vertical and wavelength grids.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Dict, Optional
import numpy as np
from . import backend

_backend = backend.get_backend()

Grid = _backend._tuvx._Grid if backend.tuvx_available() else None

if backend.tuvx_available():
    original_init = Grid.__init__

    def __init__(self, *, name: str, units: str,
                 num_sections: Optional[int] = None,
                 edges: Optional[np.ndarray] = None,
                 midpoints: Optional[np.ndarray] = None,
                 **kwargs):
        """Initialize a Grid instance. Note that at least one of num_sections, edges, or midpoints
        must be provided.

        Args:
            name: Name of the grid
            units: Units of the grid values
            num_sections: Optional number of grid sections
            edges: Optional array of edge values (length num_sections + 1)
            midpoints: Optional array of midpoint values (length num_sections)
            **kwargs: Additional arguments passed to the C++ constructor
        """
        if (num_sections is None and edges is None and midpoints is None):
            raise ValueError("At least one of num_sections, edges, or midpoints must be provided.")
        if (num_sections is None):
            if (edges is not None):
                num_sections = len(edges) - 1
            elif (midpoints is not None):
                num_sections = len(midpoints)
        # Call the original C++ constructor correctly
        original_init(self, name=name, units=units, num_sections=num_sections, **kwargs)

        # Set edges or midpoints if provided
        if edges is not None:
            self.edges = edges
        if midpoints is not None:
            self.midpoints = midpoints

    Grid.__init__ = __init__

    def __str__(self):
        """User-friendly string representation."""
        return f"Grid(name={self.name}, units={self.units}, num_sections={self.num_sections})"

    Grid.__str__ = __str__

    def __repr__(self):
        """Detailed string representation for debugging."""
        return (f"Grid(name={self.name}, units={self.units}, num_sections={self.num_sections}, "
                f"edges={self.edges}, midpoints={self.midpoints})")

    Grid.__repr__ = __repr__

    def __len__(self):
        """Return the number of sections in the grid."""
        return self.num_sections

    Grid.__len__ = __len__

    def __eq__(self, other):
        """Check equality with another Grid instance."""
        if not isinstance(other, Grid):
            return NotImplemented
        return (self.name == other.name and
                self.units == other.units and
                self.num_sections == other.num_sections and
                np.array_equal(self.edges, other.edges) and
                np.array_equal(self.midpoints, other.midpoints))

    Grid.__eq__ = __eq__

    def __bool__(self):
        """Return True if the grid has sections."""
        return self.num_sections > 0

    Grid.__bool__ = __bool__

    def __contains__(self, value):
        """Check if a value is within the grid bounds."""
        return self.edges[0] <= value <= self.edges[-1]

    Grid.__contains__ = __contains__
