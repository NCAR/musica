# Copyright (C) 2023-2025 National Center for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x Profile class.

This module provides a class for defining profiles in TUV-x. Profiles represent
physical quantities that vary along a grid, such as temperature or species
concentrations.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Dict, Optional
import numpy as np
from . import backend
from .grid import Grid

_backend = backend.get_backend()

Profile = _backend._tuvx._Profile if backend.tuvx_available() else None

if backend.tuvx_available():
    original_init = Profile.__init__

    def __init__(self, *, name: str, units: str, grid: Grid,
                 edge_values: Optional[np.ndarray] = None,
                 midpoint_values: Optional[np.ndarray] = None,
                 layer_densities: Optional[np.ndarray] = None,
                 **kwargs):
        """Initialize a Profile instance.

        Args:
            name: Name of the profile
            units: Units of the profile values
            grid: Grid on which the profile is defined
            edge_values: Optional array of values at grid edges (length num_sections + 1)
            midpoint_values: Optional array of values at grid midpoints (length num_sections)
            layer_densities: Optional array of layer densities (length num_sections)
            **kwargs: Additional arguments passed to the C++ constructor
        """
        # Call the original C++ constructor correctly
        original_init(self, name=name, units=units, grid=grid, **kwargs)

        # Set values if provided
        if edge_values is not None:
            self.edge_values = edge_values
        if midpoint_values is not None:
            self.midpoint_values = midpoint_values
        if layer_densities is not None:
            self.layer_densities = layer_densities

    Profile.__init__ = __init__

    def __str__(self):
        """User-friendly string representation."""
        return f"Profile(name={self.name}, units={self.units}, number_of_sections={self.number_of_sections})"

    Profile.__str__ = __str__

    def __repr__(self):
        """Detailed string representation for debugging."""
        return (f"Profile(name={self.name}, units={self.units}, number_of_sections={self.number_of_sections}, "
                f"edge_values={self.edge_values}, midpoint_values={self.midpoint_values}, "
                f"layer_densities={self.layer_densities}, "
                f"exo_layer_density={self.exo_layer_density})")

    Profile.__repr__ = __repr__

    def __len__(self):
        """Return the number of sections in the grid."""
        return self.number_of_sections

    Profile.__len__ = __len__

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

    Profile.__eq__ = __eq__

    def __bool__(self):
        """Return True if the profile has a name, units, and one or more sections."""
        return bool(self.name) and bool(self.units) and self.number_of_sections > 0

    Profile.__bool__ = __bool__
