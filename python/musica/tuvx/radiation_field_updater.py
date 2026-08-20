# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x RadiationFieldUpdater class.

This module provides a class that lets a host application push a radiation field into
TUV-x's "from host" radiative transfer solver.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Optional
import numpy as np


class RadiationFieldUpdater:
    """Lets a host application push a radiation field into TUV-x's "from host" solver.

    Obtained only via ``TUVX.get_radiation_field_updater()``; never constructed directly.
    """

    def __init__(self, cpp_updater, num_vertical_interfaces: int, num_wavelength_bins: int):
        self._cpp = cpp_updater
        self._num_vertical_interfaces = num_vertical_interfaces
        self._num_wavelength_bins = num_wavelength_bins

    def update(self,
               direct_actinic_flux: np.ndarray,
               upward_actinic_flux: np.ndarray,
               downward_actinic_flux: np.ndarray,
               direct_irradiance: Optional[np.ndarray] = None,
               upward_irradiance: Optional[np.ndarray] = None,
               downward_irradiance: Optional[np.ndarray] = None):
        """
        Set the radiation field TUV-x will use for the next run.

        All arrays have shape (num_wavelength_bins, num_vertical_interfaces), matching the
        axis order used by Radiator's optical property arrays. Interface 0 is the lowest
        altitude. All values are dimensionless and must NOT include the extraterrestrial
        flux profile or the Earth-Sun distance factor -- TUV-x applies both downstream.

        Call this before every TUVX.run(). TUV-x does not raise an error if a run is
        missed, it silently reuses the last field set (or an all-zero field, if update()
        was never called).

        Args:
            direct_actinic_flux: Direct component of the actinic flux (required)
            upward_actinic_flux: Diffuse upwelling component of the actinic flux (required)
            downward_actinic_flux: Diffuse downwelling component of the actinic flux (required)
            direct_irradiance: Direct component of the irradiance (optional). Used only for
                dose rates; an omitted component is treated as all zeros for this call.
            upward_irradiance: Diffuse upwelling component of the irradiance (optional; see above)
            downward_irradiance: Diffuse downwelling component of the irradiance (optional; see above)

        Raises:
            ValueError: If an array does not have shape (num_wavelength_bins, num_vertical_interfaces)
        """
        self._cpp.update(
            direct_actinic_flux,
            upward_actinic_flux,
            downward_actinic_flux,
            self._num_vertical_interfaces,
            self._num_wavelength_bins,
            direct_irradiance,
            upward_irradiance,
            downward_irradiance,
        )
