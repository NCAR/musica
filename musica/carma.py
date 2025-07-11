"""
CARMA aerosol model Python interface.

Note: CARMA is only available on macOS and Linux platforms.
"""

from . import backend

_backend = backend.get_backend()

version = _backend._carma._get_carma_version() if backend.carma_available() else None


class CARMA:
    """
    A Python interface to the CARMA aerosol model.
    """

    def __init__(self):
        if not backend.carma_available():
            raise ValueError(
                "CARMA backend is not available on this platform.")
