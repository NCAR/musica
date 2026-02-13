# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x Profile class.

This module provides a class for defining profiles in TUV-x. Profiles represent
physical quantities that vary along a grid, such as temperature or species
concentrations.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Optional
import numpy as np
from .. import backend
from .._base import CppWrapper, _unwrap
from .grid import Grid

_backend = backend.get_backend()
_Profile = _backend._tuvx._Profile if backend.tuvx_available() else None


class Profile(CppWrapper):
    """A profile of physical quantities defined on a TUV-x grid.

    Attributes:
        name: Name of the profile.
        units: Units of the profile values.
        number_of_sections: Number of sections in the underlying grid.
        edge_values: Array of values at grid edges (zero-copy).
        midpoint_values: Array of values at grid midpoints (zero-copy).
        layer_densities: Array of layer densities (zero-copy).
        exo_layer_density: Exoatmospheric layer density value.
    """

    _unavailable_message = "TUV-x was not included in your build of MUSICA."

    @classmethod
    def _check_available(cls):
        return backend.tuvx_available()

    def __init__(self, *, name: str, units: str, grid: Grid,
                 edge_values: Optional[np.ndarray] = None,
                 midpoint_values: Optional[np.ndarray] = None,
                 layer_densities: Optional[np.ndarray] = None,
                 calculate_layer_densities: bool = False,
                 exo_layer_density: Optional[float] = 0.0,
                 **kwargs):
        """Initialize a Profile instance.

        Args:
            name: Name of the profile.
            units: Units of the profile values.
            grid: Grid on which the profile is defined.
            edge_values: Optional array of values at grid edges (length ``num_sections + 1``).
            midpoint_values: Optional array of values at grid midpoints (length ``num_sections``).
            layer_densities: Optional array of layer densities (length ``num_sections``).
            calculate_layer_densities: If True, calculate layer densities from midpoint values.
            exo_layer_density: Optional exoatmospheric layer density value (scalar). If provided,
                               this value will be used for the exoatmospheric layer density, and
                               added to the uppermost layer density for consistency.
            **kwargs: Additional arguments passed to the C++ constructor.
        """
        if not self._check_available():
            raise RuntimeError(self._unavailable_message)
        self._cpp = _Profile(name=name, units=units, grid=_unwrap(grid), **kwargs)

        if edge_values is None and midpoint_values is None:
            self.edge_values = np.zeros(grid.num_sections + 1, dtype=np.float64)
            self.midpoint_values = np.zeros(grid.num_sections, dtype=np.float64)
            self.layer_densities = np.zeros(grid.num_sections, dtype=np.float64)
        if edge_values is not None:
            self.edge_values = edge_values
        elif midpoint_values is not None:
            edge_values = np.zeros(midpoint_values.size + 1, dtype=midpoint_values.dtype)
            edge_values[1:-1] = 0.5 * (midpoint_values[:-1] + midpoint_values[1:])
            edge_values[0] = midpoint_values[0] - (edge_values[1] - midpoint_values[0])
            edge_values[-1] = midpoint_values[-1] + (midpoint_values[-1] - edge_values[-2])
            self.edge_values = edge_values
        if midpoint_values is not None:
            self.midpoint_values = midpoint_values
        elif edge_values is not None:
            self.midpoint_values = 0.5 * (edge_values[:-1] + edge_values[1:])
        if layer_densities is not None:
            if calculate_layer_densities:
                raise ValueError("Cannot provide layer_densities and set calculate_layer_densities=True")
            self.layer_densities = layer_densities
        elif calculate_layer_densities:
            self.calculate_layer_densities(grid)
        if exo_layer_density < 0.0:
            raise ValueError("exo_layer_density must be non-negative")
        if exo_layer_density > 0.0:
            self.exo_layer_density = exo_layer_density

    @property
    def name(self) -> str:
        """Name of the profile."""
        return self._cpp.name

    @name.setter
    def name(self, value: str):
        self._cpp.name = value

    @property
    def units(self) -> str:
        """Units of the profile values."""
        return self._cpp.units

    @units.setter
    def units(self, value: str):
        self._cpp.units = value

    @property
    def number_of_sections(self) -> int:
        """Number of sections in the underlying grid."""
        return self._cpp.number_of_sections

    @property
    def edge_values(self) -> np.ndarray:
        """Array of values at grid edges (zero-copy view into C++ memory)."""
        return self._cpp.edge_values

    @edge_values.setter
    def edge_values(self, value: np.ndarray):
        self._cpp.edge_values = value

    @property
    def midpoint_values(self) -> np.ndarray:
        """Array of values at grid midpoints (zero-copy view into C++ memory)."""
        return self._cpp.midpoint_values

    @midpoint_values.setter
    def midpoint_values(self, value: np.ndarray):
        self._cpp.midpoint_values = value

    @property
    def layer_densities(self) -> np.ndarray:
        """Array of layer densities (zero-copy view into C++ memory)."""
        return self._cpp.layer_densities

    @layer_densities.setter
    def layer_densities(self, value: np.ndarray):
        self._cpp.layer_densities = value

    @property
    def exo_layer_density(self) -> float:
        """Exoatmospheric layer density value."""
        return self._cpp.exo_layer_density

    @exo_layer_density.setter
    def exo_layer_density(self, value: float):
        self._cpp.exo_layer_density = value

    def calculate_exo_layer_density(self, scale_height: float):
        """Calculate the exoatmospheric layer density.

        Args:
            scale_height: The scale height used for the calculation.
        """
        self._cpp.calculate_exo_layer_density(scale_height)

    def calculate_layer_densities(self, grid: Grid, conv: Optional[float] = None):
        """Calculate layer densities from midpoint values and grid spacing.

        Args:
            grid: Grid on which the profile is defined.
            conv: Conversion factor to apply (default is 1.0 or 1.0e5 for
                  height in km and concentrations in molecules cm-3).
        """
        if conv is None:
            if grid.name == "height" and grid.units == "km" and self.units == "molecule cm-3":
                conv = 1e5
            else:
                conv = 1.0
        deltas = grid.edges[1:] - grid.edges[:-1]
        self.layer_densities = self.midpoint_values * deltas * conv

    def __str__(self):
        """User-friendly string representation."""
        return f"Profile(name={self.name}, units={self.units}, number_of_sections={self.number_of_sections})"

    def __repr__(self):
        """Detailed string representation for debugging."""
        return (f"Profile(name={self.name}, units={self.units}, number_of_sections={self.number_of_sections}, "
                f"edge_values={self.edge_values}, midpoint_values={self.midpoint_values}, "
                f"layer_densities={self.layer_densities}, "
                f"exo_layer_density={self.exo_layer_density})")

    def __len__(self):
        """Return the number of sections in the grid."""
        return self.number_of_sections

    def __eq__(self, other):
        """Check equality with another Profile instance."""
        if not isinstance(other, Profile):
            return NotImplemented
        return (self.name == other.name and
                self.units == other.units and
                self.number_of_sections == other.number_of_sections and
                np.array_equal(self.edge_values, other.edge_values) and
                np.array_equal(self.midpoint_values, other.midpoint_values) and
                np.array_equal(self.layer_densities, other.layer_densities) and
                self.exo_layer_density == other.exo_layer_density)

    def __bool__(self):
        """Return True if the profile has a name, units, and one or more sections."""
        return bool(self.name) and bool(self.units) and self.number_of_sections > 0
