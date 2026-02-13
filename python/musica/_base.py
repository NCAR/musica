# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0


class CppWrapper:
    """Base class for Python wrapper classes around pybind11 C++ objects.

    Subclasses hold the underlying C++ object in ``self._cpp`` and forward
    attribute access as needed.
    """

    _unavailable_message = "Backend not available."

    @classmethod
    def _from_cpp(cls, cpp_obj):
        """Wrap a raw C++ pybind11 object in the Python wrapper class."""
        instance = object.__new__(cls)
        instance._cpp = cpp_obj
        return instance

    @classmethod
    def _check_available(cls):
        """Return True if the backend for this class is available.

        Override in subclasses that depend on optional backends (e.g. TUV-x).
        """
        return True


class CppField:
    """Descriptor that forwards attribute access to the underlying ``_cpp`` object.

    Usage::

        class MyWrapper(CppWrapper):
            name = CppField()           # forwards to self._cpp.name
            value = CppField('cpp_val') # forwards to self._cpp.cpp_val
    """

    def __init__(self, cpp_attr=None):
        self.cpp_attr = cpp_attr

    def __set_name__(self, owner, name):
        if self.cpp_attr is None:
            self.cpp_attr = name

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        return getattr(obj._cpp, self.cpp_attr)

    def __set__(self, obj, value):
        setattr(obj._cpp, self.cpp_attr, value)


def _unwrap(obj):
    """Extract the underlying C++ object from a wrapper, or return as-is."""
    if hasattr(obj, '_cpp'):
        return obj._cpp
    return obj


def _unwrap_list(items):
    """Unwrap a list of wrapped objects."""
    return [_unwrap(item) for item in items]


def _wrap_list(cls, items):
    """Wrap a list of C++ objects in the given wrapper class."""
    return [cls._from_cpp(item) for item in items]
