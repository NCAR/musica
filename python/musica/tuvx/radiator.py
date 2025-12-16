# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x Radiator class.

This module provides a class for defining radiators in TUV-x. Radiators represent
optically active species that should be considered in radiative transfer calculations.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Optional
import numpy as np
from .. import backend
from .grid import Grid

_backend = backend.get_backend()

Radiator = _backend._tuvx._Radiator if backend.tuvx_available() else None

if backend.tuvx_available():
    original_init = Radiator.__init__

    def __init__(self, *, name: str, height_grid: Grid, wavelength_grid: Grid,
                 optical_depths: Optional[np.ndarray] = None,
                 single_scattering_albedos: Optional[np.ndarray] = None,
                 asymmetry_factors: Optional[np.ndarray] = None,
                 **kwargs):
        """Initialize a Radiator instance.

        Args:
            name: Name of the radiator
            height_grid: Grid on which the radiator is defined (height)
            wavelength_grid: Grid on which the radiator is defined (wavelength)
            optical_depths: Optional 2D array of optical depths
                                       (shape: num_heights x num_wavelengths)
            single_scattering_albedos: Optional 2D array of single scattering albedos
                                       (shape: num_heights x num_wavelengths)
            asymmetry_factors: Optional 2D array of asymmetry parameters
                                  (shape: num_heights x num_wavelengths)
            **kwargs: Additional arguments passed to the C++ constructor
        """
        # Call the original C++ constructor correctly
        original_init(self, name=name, height_grid=height_grid,
                      wavelength_grid=wavelength_grid, **kwargs)

        # Set properties if provided
        if optical_depths is not None:
            self.optical_depths = optical_depths
        if single_scattering_albedos is not None:
            self.single_scattering_albedos = single_scattering_albedos
        if asymmetry_factors is not None:
            self.asymmetry_factors = asymmetry_factors

    Radiator.__init__ = __init__

    def __str__(self):
        """User-friendly string representation."""
        return (f"Radiator(name={self.name}, "
                f"num_height_sections={self.number_of_height_sections}, "
                f"num_wavelength_sections={self.number_of_wavelength_sections})")

    Radiator.__str__ = __str__

    def __repr__(self):
        """Detailed string representation for debugging."""
        return (f"Radiator(name={self.name}, "
                f"num_height_sections={self.number_of_height_sections}, "
                f"num_wavelength_sections={self.number_of_wavelength_sections}, "
                f"optical_depths={self.optical_depths}, "
                f"single_scattering_albedos={self.single_scattering_albedos}, "
                f"asymmetry_factors={self.asymmetry_factors})")

    Radiator.__repr__ = __repr__

    def __eq__(self, other):
        """Check equality between two Radiator instances."""
        if not isinstance(other, Radiator):
            return NotImplemented
        return (self.name == other.name and
                np.array_equal(self.optical_depths, other.optical_depths) and
                np.array_equal(self.single_scattering_albedos, other.single_scattering_albedos) and
                np.array_equal(self.asymmetry_factors, other.asymmetry_factors) and
                self.number_of_height_sections == other.number_of_height_sections and
                self.number_of_wavelength_sections == other.number_of_wavelength_sections)

    Radiator.__eq__ = __eq__

    def __bool__(self):
        """Return True if the radiator has name and height and wavelength sections."""
        return (bool(self.name) and
                self.number_of_height_sections > 0 and
                self.number_of_wavelength_sections > 0)

    Radiator.__bool__ = __bool__
