# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x RadiatorMap class.

This module provides a class for managing collections of TUV-x radiators.
The RadiatorMap class allows dictionary-style access to radiators using their names as keys.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Iterator
from .. import backend
from .radiator import Radiator

_backend = backend.get_backend()

RadiatorMap = _backend._tuvx._RadiatorMap if backend.tuvx_available() else None

if backend.tuvx_available():
    original_init = RadiatorMap.__init__

    def __init__(self, **kwargs):
        """Initialize a RadiatorMap instance.

        Args:
            **kwargs: Additional arguments passed to the C++ constructor
        """
        original_init(self, **kwargs)

    RadiatorMap.__init__ = __init__

    def __str__(self):
        """User-friendly string representation."""
        return f"RadiatorMap(num_radiators={len(self)})"

    RadiatorMap.__str__ = __str__

    def __repr__(self):
        """Detailed string representation for debugging."""
        radiator_details = []
        for i in range(len(self)):
            radiator = self.get_radiator_by_index(i)
            radiator_details.append(f"{radiator.name}")
        return f"RadiatorMap(radiators={radiator_details})"

    RadiatorMap.__repr__ = __repr__

    def __len__(self):
        """Return the number of radiators in the map."""
        return self.get_number_of_radiators()

    RadiatorMap.__len__ = __len__

    def __bool__(self):
        """Return True if the map has any radiators."""
        return len(self) > 0

    RadiatorMap.__bool__ = __bool__

    def __getitem__(self, key) -> Radiator:
        """Get a radiator by name.

        Args:
            key: Name of the radiator to retrieve

        Returns:
            Radiator instance corresponding to the given name

        Raises:
            KeyError: If no radiator with the given name exists
        """
        if not isinstance(key, str):
            raise TypeError("Radiator name must be a string.")
        try:
            radiator = self.get_radiator(key)
            if radiator is None:
                raise KeyError(f"Radiator with name '{key}' not found.")
            return radiator
        except Exception as e:
            raise KeyError(f"No radiator found with name='{key}'") from e

    RadiatorMap.__getitem__ = __getitem__

    def __setitem__(self, key: str, radiator: Radiator):
        """Set a radiator in the map by name.

        Args:
            key: Name of the radiator to set
            radiator: Radiator instance to associate with the given name
        """
        if not isinstance(key, str):
            raise TypeError("Radiator name must be a string.")
        if not isinstance(radiator, Radiator):
            raise TypeError("Value must be a Radiator instance.")
        if radiator.name != key:
            raise ValueError("Radiator name does not match the key.")
        self.add_radiator(radiator)

    RadiatorMap.__setitem__ = __setitem__

    def __iter__(self) -> Iterator[Radiator]:
        """Iterator over the radiators in the map.

        Yields:
            Radiator instances in the map
        """
        for i in range(len(self)):
            yield self.get_radiator_by_index(i).name

    RadiatorMap.__iter__ = __iter__

    def __contains__(self, key: str) -> bool:
        """Check if a radiator with the given name exists in the map.

        Args:
            key: Name of the radiator to check

        Returns:
            True if a radiator with the given name exists, False otherwise
        """
        if not isinstance(key, str):
            return False
        try:
            radiator = self.get_radiator(str(key))
            return radiator is not None
        except (ValueError, KeyError):
            return False

    RadiatorMap.__contains__ = __contains__

    def clear(self):
        """Remove all radiators from the map."""
        num_radiators = len(self)
        for _ in range(num_radiators):
            self.remove_radiator_by_index(0)

    RadiatorMap.clear = clear

    def items(self):
        """Iterator over (name, radiator) pairs in the map.

        Yields:
            Tuples of (radiator_name, Radiator instance)
        """
        for i in range(len(self)):
            radiator = self.get_radiator_by_index(i)
            yield (radiator.name, radiator)

    RadiatorMap.items = items

    def keys(self):
        """Iterator over radiator names in the map.

        Yields:
            Radiator names as strings
        """
        for i in range(len(self)):
            radiator = self.get_radiator_by_index(i)
            yield radiator.name

    RadiatorMap.keys = keys

    def values(self):
        """Iterator over Radiator instances in the map.

        Yields:
            Radiator instances
        """
        for i in range(len(self)):
            yield self.get_radiator_by_index(i)

    RadiatorMap.values = values
