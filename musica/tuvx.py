"""
TUV-x photolysis calculator Python interface.

Note: TUV-x is only available on macOS and Linux platforms.
"""

from . import backend

_backend = backend.get_backend()

# Module-level version attribute
version = _backend._tuvx._get_tuvx_version() if backend.tuvx_available() else None


class TUVX:
    """
    A Python interface to the TUV-x photolysis calculator.
    """

    def __init__(self):
        if not backend.tuvx_available():
            raise ValueError(
                "TUV-x backend is not available on this platform.")
