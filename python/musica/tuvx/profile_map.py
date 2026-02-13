# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
"""
TUV-x ProfileMap class.

This module provides a class for managing collections of TUV-x profiles.
The ProfileMap class allows dictionary-style access to profiles using (name, units) tuples as keys.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from typing import Iterator
from .. import backend
from .._base import CppWrapper, _unwrap
from .profile import Profile

_backend = backend.get_backend()
_ProfileMap = _backend._tuvx._ProfileMap if backend.tuvx_available() else None


class ProfileMap(CppWrapper):
    """A collection of TUV-x profiles with dictionary-style access.

    Profiles are accessed using ``(name, units)`` tuples as keys.
    """

    _unavailable_message = "TUV-x was not included in your build of MUSICA."

    @classmethod
    def _check_available(cls):
        return backend.tuvx_available()

    def __init__(self, **kwargs):
        """Initialize a ProfileMap instance.

        Args:
            **kwargs: Additional arguments passed to the C++ constructor.
        """
        if not self._check_available():
            raise RuntimeError(self._unavailable_message)
        self._cpp = _ProfileMap(**kwargs)

    def add_profile(self, profile: Profile):
        """Add a profile to the map."""
        self._cpp.add_profile(_unwrap(profile))

    def get_profile(self, name: str, units: str) -> Profile:
        """Get a profile by name and units."""
        return Profile._from_cpp(self._cpp.get_profile(name, units))

    def get_profile_by_index(self, index: int) -> Profile:
        """Get a profile by its index."""
        return Profile._from_cpp(self._cpp.get_profile_by_index(index))

    def get_number_of_profiles(self) -> int:
        """Return the number of profiles in the map."""
        return self._cpp.get_number_of_profiles()

    def remove_profile_by_index(self, index: int):
        """Remove a profile by its index."""
        self._cpp.remove_profile_by_index(index)

    def __str__(self):
        """User-friendly string representation."""
        return f"ProfileMap(num_profiles={len(self)})"

    def __repr__(self):
        """Detailed string representation for debugging."""
        profile_details = []
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            profile_details.append(f"({profile.name}, {profile.units})")
        return f"ProfileMap(profiles={profile_details})"

    def __len__(self):
        """Return the number of profiles in the map."""
        return self.get_number_of_profiles()

    def __bool__(self):
        """Return True if the map has any profiles."""
        return len(self) > 0

    def __getitem__(self, key) -> Profile:
        """Get a profile using dictionary-style access with (name, units) tuple as key.

        Args:
            key: A tuple of (profile_name, profile_units).

        Returns:
            The requested Profile object.

        Raises:
            KeyError: If no profile matches the given name and units.
            TypeError: If key is not a tuple of (str, str).
        """
        if not isinstance(key, tuple) or len(key) != 2:
            raise TypeError("Profile access requires a tuple of (name, units)")
        name, units = key
        try:
            return self.get_profile(name, units)
        except Exception as e:
            raise KeyError(f"No profile found with name='{name}' and units='{units}'") from e

    def __setitem__(self, key, profile):
        """Add a profile to the map using dictionary-style access.

        Args:
            key: A tuple of (profile_name, profile_units).
            profile: The Profile object to add.

        Raises:
            TypeError: If key is not a tuple, or if key components are not strings.
            TypeError: If profile is not a Profile object.
            ValueError: If profile name/units don't match the key.
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
            raise ValueError(f"Profile name/units must match the key tuple: {(profile.name, profile.units)}")
        self.add_profile(profile)

    def __iter__(self) -> Iterator:
        """Return an iterator over (name, units) tuples of all profiles."""
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            yield (profile.name, profile.units)

    def __contains__(self, key) -> bool:
        """Check if a profile with given name and units exists in the map.

        Args:
            key: A tuple of (profile_name, profile_units).

        Returns:
            True if a matching profile exists, False otherwise.
        """
        if not isinstance(key, tuple) or len(key) != 2:
            return False
        name, units = key
        try:
            profile = self.get_profile(str(name), str(units))
            return profile is not None
        except (ValueError, KeyError):
            return False

    def clear(self):
        """Remove all profiles from the map."""
        while len(self) > 0:
            self.remove_profile_by_index(0)

    def items(self):
        """Return an iterator over (key, profile) pairs, where key is (name, units)."""
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            yield ((profile.name, profile.units), profile)

    def keys(self):
        """Return an iterator over profile keys (name, units) tuples."""
        for i in range(len(self)):
            profile = self.get_profile_by_index(i)
            yield (profile.name, profile.units)

    def values(self):
        """Return an iterator over Profile objects in the map."""
        for i in range(len(self)):
            yield self.get_profile_by_index(i)
