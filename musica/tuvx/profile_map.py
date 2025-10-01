# Copyright (C) 2023-2025 National Center for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x ProfileMap class.

This module provides a class for managing collections of TUV-x profiles.
The ProfileMap class allows dictionary-style access to profiles using (name, units) tuples as keys.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Iterator, Sequence
from . import backend
from .profile import Profile

_backend = backend.get_backend()

ProfileMap = _backend._tuvx._ProfileMap if backend.tuvx_available() else None

if backend.tuvx_available():
    original_init = ProfileMap.__init__

    def __init__(self, **kwargs):
        """Initialize a ProfileMap instance.

        Args:
            **kwargs: Additional arguments passed to the C++ constructor
        """
        original_init(self, **kwargs)

    ProfileMap.__init__ = __init__

    def __str__(self):
        """User-friendly string representation."""
        return f"ProfileMap(num_profiles={len(self)})"

    ProfileMap.__str__ = __str__

    def __repr__(self):
        """Detailed string representation for debugging."""
        profile_details = []
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            profile_details.append(f"({profile.name}, {profile.units})")
        return f"ProfileMap(profiles={profile_details})"

    ProfileMap.__repr__ = __repr__

    def __len__(self):
        """Return the number of profiles in the map."""
        return self.get_number_of_profiles()

    ProfileMap.__len__ = __len__

    def __bool__(self):
        """Return True if the map has any profiles."""
        return len(self) > 0

    ProfileMap.__bool__ = __bool__

    def __getitem__(self, key) -> Profile:
        """Get a profile using dictionary-style access with (name, units) tuple as key.

        Args:
            key: A tuple of (profile_name, profile_units)

        Returns:
            The requested Profile object

        Raises:
            KeyError: If no profile matches the given name and units
            TypeError: If key is not a tuple of (str, str)
        """
        if not isinstance(key, tuple) or len(key) != 2:
            raise TypeError("Profile access requires a tuple of (name, units)")
        name, units = key
        try:
            return self.get_profile(name, units)
        except Exception as e:
            raise KeyError(f"No profile found with name='{name}' and units='{units}'") from e

    ProfileMap.__getitem__ = __getitem__

    def __setitem__(self, key, profile):
        """Add a profile to the map using dictionary-style access.

        Args:
            key: A tuple of (profile_name, profile_units)
            profile: The Profile object to add

        Raises:
            TypeError: If key is not a tuple, or if key components are not strings
            TypeError: If profile is not a Profile object
            ValueError: If profile name/units don't match the key
        """
        if not isinstance(key, tuple) or len(key) != 2:
            raise TypeError("Profile assignment requires a tuple of (name, units)")
        name, units = key
        if not isinstance(name, str):
            raise TypeError("Profile name must be a string")
        if not isinstance(units, str):
            raise TypeError("Profile units must be a string")
        if not isinstance(profile, Profile):
            raise TypeError("Value must be a Profile object")
        if profile.name != name or profile.units != units:
            raise ValueError("Profile name/units must match the key tuple")
        self.add_profile(profile)

    ProfileMap.__setitem__ = __setitem__

    def __iter__(self) -> Iterator:
        """Return an iterator over (name, units) tuples of all profiles."""
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            yield (profile.name, profile.units)

    ProfileMap.__iter__ = __iter__

    def __contains__(self, key) -> bool:
        """Check if a profile with given name and units exists in the map.

        Args:
            key: A tuple of (profile_name, profile_units)

        Returns:
            True if a matching profile exists, False otherwise
        """
        if not isinstance(key, tuple) or len(key) != 2:
            return False
        name, units = key
        try:
            profile = self.get_profile(str(name), str(units))
            return profile is not None
        except (ValueError, KeyError):
            return False

    ProfileMap.__contains__ = __contains__

    def clear(self):
        """Remove all profiles from the map."""
        while len(self) > 0:
            self.remove_profile_by_index(0)

    ProfileMap.clear = clear

    def items(self):
        """Return an iterator over (key, profile) pairs, where key is (name, units)."""
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            yield ((profile.name, profile.units), profile)

    ProfileMap.items = items

    def keys(self):
        """Return an iterator over profile keys (name, units) tuples."""
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            yield (profile.name, profile.units)

    ProfileMap.keys = keys

    def values(self):
        """Return an iterator over Profile objects in the map."""
        for i in range(len(self)):
            yield self.get_profile_by_index(i)

    ProfileMap.values = values
