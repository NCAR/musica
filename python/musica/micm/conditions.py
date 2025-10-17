# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from typing import Optional, Union
from ..constants import GAS_CONSTANT

from .. import backend

_backend = backend.get_backend()
Conditions = _backend._micm._Conditions
original_init = Conditions.__init__


def __init__(
    self,
    temperature: Optional[Union[float, int]] = None,
    pressure: Optional[Union[float, int]] = None,
    air_density: Optional[Union[float, int]] = None,
):
    """
    Initializes the Conditions object with the given parameters.

    Args:
        temperature (float): Temperature in Kelvin.
        pressure (float): Pressure in Pascals.
        air_density (float): Air density in mol m-3
    """
    original_init(self)
    if temperature is not None:
        self.temperature = temperature
    if pressure is not None:
        self.pressure = pressure
    if air_density is not None:
        self.air_density = air_density
    elif temperature is not None and pressure is not None:
        self.air_density = 1.0 / (GAS_CONSTANT * temperature / pressure)


Conditions.__doc__ = """
    Conditions class for the MICM solver. If air density is not provided,
    it will be calculated from the Ideal Gas Law using the provided temperature and pressure.

    Attributes:
        temperature (float): Temperature in Kelvin.
        pressure (float): Pressure in Pascals.
        air_density (float): Air density in mol m-3
    """

Conditions.__init__ = __init__
