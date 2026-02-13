# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x Grid class.

This module provides a class for defining grids on which TUV-x profiles exist.
Typically, this would be used to define vertical and wavelength grids.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Optional
import numpy as np
from .. import backend
from .._base import CppWrapper

_backend = backend.get_backend()
_Grid = _backend._tuvx._Grid if backend.tuvx_available() else None


class Grid(CppWrapper):
    """A grid on which TUV-x profiles are defined.

    Attributes:
        name: Name of the grid.
        units: Units of the grid values.
        num_sections: Number of grid sections.
        edges: Array of edge values (length ``num_sections + 1``).
            Shares memory with the C++ object (zero-copy).
        midpoints: Array of midpoint values (length ``num_sections``).
            Shares memory with the C++ object (zero-copy).
    """

    _unavailable_message = "TUV-x was not included in your build of MUSICA."

    @classmethod
    def _check_available(cls):
        return backend.tuvx_available()

    def __init__(self, *, name: str, units: str,
                 num_sections: Optional[int] = None,
                 edges: Optional[np.ndarray] = None,
                 midpoints: Optional[np.ndarray] = None,
                 **kwargs):
        """Initialize a Grid instance.

        At least one of *num_sections*, *edges*, or *midpoints* must be provided.

        Args:
            name: Name of the grid.
            units: Units of the grid values.
            num_sections: Optional number of grid sections.
            edges: Optional array of edge values (length ``num_sections + 1``).
            midpoints: Optional array of midpoint values (length ``num_sections``).
            **kwargs: Additional arguments passed to the C++ constructor.
        """
        if not self._check_available():
            raise RuntimeError(self._unavailable_message)
        if num_sections is None and edges is None and midpoints is None:
            raise ValueError("At least one of num_sections, edges, or midpoints must be provided.")
        if num_sections is None:
            if edges is not None:
                num_sections = len(edges) - 1
            elif midpoints is not None:
                num_sections = len(midpoints)
        self._cpp = _Grid(name=name, units=units, num_sections=num_sections, **kwargs)
        if edges is not None:
            self.edges = edges
        if midpoints is not None:
            self.midpoints = midpoints

    @property
    def name(self) -> str:
        """Name of the grid."""
        return self._cpp.name

    @name.setter
    def name(self, value: str):
        self._cpp.name = value

    @property
    def units(self) -> str:
        """Units of the grid values."""
        return self._cpp.units

    @units.setter
    def units(self, value: str):
        self._cpp.units = value

    @property
    def num_sections(self) -> int:
        """Number of grid sections."""
        return self._cpp.num_sections

    @property
    def edges(self) -> np.ndarray:
        """Array of edge values (zero-copy view into C++ memory)."""
        return self._cpp.edges

    @edges.setter
    def edges(self, value: np.ndarray):
        self._cpp.edges = value

    @property
    def midpoints(self) -> np.ndarray:
        """Array of midpoint values (zero-copy view into C++ memory)."""
        return self._cpp.midpoints

    @midpoints.setter
    def midpoints(self, value: np.ndarray):
        self._cpp.midpoints = value

    def __str__(self):
        """User-friendly string representation."""
        return f"Grid(name={self.name}, units={self.units}, num_sections={self.num_sections})"

    def __repr__(self):
        """Detailed string representation for debugging."""
        return (f"Grid(name={self.name}, units={self.units}, num_sections={self.num_sections}, "
                f"edges={self.edges}, midpoints={self.midpoints})")

    def __len__(self):
        """Return the number of sections in the grid."""
        return self.num_sections

    def __eq__(self, other):
        """Check equality with another Grid instance."""
        if not isinstance(other, Grid):
            return NotImplemented
        return (self.name == other.name and
                self.units == other.units and
                self.num_sections == other.num_sections and
                np.array_equal(self.edges, other.edges) and
                np.array_equal(self.midpoints, other.midpoints))

    def __bool__(self):
        """Return True if the grid has sections."""
        return self.num_sections > 0

    def __contains__(self, value):
        """Check if a value is within the grid bounds."""
        return self.edges[0] <= value <= self.edges[-1]
