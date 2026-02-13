# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
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
from .._base import CppWrapper, _unwrap
from .grid import Grid

_backend = backend.get_backend()
_Radiator = _backend._tuvx._Radiator if backend.tuvx_available() else None


class Radiator(CppWrapper):
    """An optically active species for TUV-x radiative transfer calculations.

    Attributes:
        name: Name of the radiator.
        number_of_height_sections: Number of height grid sections.
        number_of_wavelength_sections: Number of wavelength grid sections.
        optical_depths: 2D array of optical depths (zero-copy).
        single_scattering_albedos: 2D array of single scattering albedos (zero-copy).
        asymmetry_factors: 2D array of asymmetry parameters (zero-copy).
    """

    _unavailable_message = "TUV-x was not included in your build of MUSICA."

    @classmethod
    def _check_available(cls):
        return backend.tuvx_available()

    def __init__(self, *, name: str, height_grid: Grid, wavelength_grid: Grid,
                 optical_depths: Optional[np.ndarray] = None,
                 single_scattering_albedos: Optional[np.ndarray] = None,
                 asymmetry_factors: Optional[np.ndarray] = None,
                 **kwargs):
        """Initialize a Radiator instance.

        Args:
            name: Name of the radiator.
            height_grid: Grid on which the radiator is defined (height).
            wavelength_grid: Grid on which the radiator is defined (wavelength).
            optical_depths: Optional 2D array of optical depths
                            (shape: num_wavelengths x num_heights).
            single_scattering_albedos: Optional 2D array of single scattering albedos
                                       (shape: num_wavelengths x num_heights).
            asymmetry_factors: Optional 2D array of asymmetry parameters
                               (shape: num_wavelengths x num_heights).
            **kwargs: Additional arguments passed to the C++ constructor.
        """
        if not self._check_available():
            raise RuntimeError(self._unavailable_message)
        self._cpp = _Radiator(name=name, height_grid=_unwrap(height_grid),
                              wavelength_grid=_unwrap(wavelength_grid), **kwargs)
        if optical_depths is not None:
            self.optical_depths = optical_depths
        if single_scattering_albedos is not None:
            self.single_scattering_albedos = single_scattering_albedos
        if asymmetry_factors is not None:
            self.asymmetry_factors = asymmetry_factors

    @property
    def name(self) -> str:
        """Name of the radiator."""
        return self._cpp.name

    @name.setter
    def name(self, value: str):
        self._cpp.name = value

    @property
    def number_of_height_sections(self) -> int:
        """Number of height grid sections."""
        return self._cpp.number_of_height_sections

    @property
    def number_of_wavelength_sections(self) -> int:
        """Number of wavelength grid sections."""
        return self._cpp.number_of_wavelength_sections

    @property
    def optical_depths(self) -> np.ndarray:
        """2D array of optical depths (zero-copy view into C++ memory)."""
        return self._cpp.optical_depths

    @optical_depths.setter
    def optical_depths(self, value: np.ndarray):
        self._cpp.optical_depths = value

    @property
    def single_scattering_albedos(self) -> np.ndarray:
        """2D array of single scattering albedos (zero-copy view into C++ memory)."""
        return self._cpp.single_scattering_albedos

    @single_scattering_albedos.setter
    def single_scattering_albedos(self, value: np.ndarray):
        self._cpp.single_scattering_albedos = value

    @property
    def asymmetry_factors(self) -> np.ndarray:
        """2D array of asymmetry parameters (zero-copy view into C++ memory)."""
        return self._cpp.asymmetry_factors

    @asymmetry_factors.setter
    def asymmetry_factors(self, value: np.ndarray):
        self._cpp.asymmetry_factors = value

    def __str__(self):
        """User-friendly string representation."""
        return (f"Radiator(name={self.name}, "
                f"num_height_sections={self.number_of_height_sections}, "
                f"num_wavelength_sections={self.number_of_wavelength_sections})")

    def __repr__(self):
        """Detailed string representation for debugging."""
        return (f"Radiator(name={self.name}, "
                f"num_height_sections={self.number_of_height_sections}, "
                f"num_wavelength_sections={self.number_of_wavelength_sections}, "
                f"optical_depths={self.optical_depths}, "
                f"single_scattering_albedos={self.single_scattering_albedos}, "
                f"asymmetry_factors={self.asymmetry_factors})")

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

    def __bool__(self):
        """Return True if the radiator has name and height and wavelength sections."""
        return (bool(self.name) and
                self.number_of_height_sections > 0 and
                self.number_of_wavelength_sections > 0)
