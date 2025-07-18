"""
CARMA aerosol model Python interface.

This module provides a simplified Python interface to the CARMA aerosol model.
It allows users to create a CARMA instance and run simulations with specified parameters.

Note: CARMA is only available on macOS and Linux platforms.
"""

from typing import Dict, Optional
from . import backend

_backend = backend.get_backend()

version = _backend._carma._get_carma_version() if backend.carma_available() else None


class CARMAParameters:
    """
    Parameters for CARMA aerosol model simulation.

    This class encapsulates all the parameters needed to configure and run
    a CARMA simulation, including model dimensions, time stepping, and
    spatial parameters.
    """

    def __init__(self,
                 max_bins: int = 100,
                 max_groups: int = 10,
                 nz: int = 1,
                 ny: int = 1,
                 nx: int = 1,
                 nelem: int = 1,
                 ngroup: int = 1,
                 nbin: int = 5,
                 nsolute: int = 0,
                 ngas: int = 0,
                 nwave: int = 30,
                 dtime: float = 1800.0,
                 nstep: int = 100,
                 deltaz: float = 1000.0,
                 zmin: float = 16500.0):
        """
        Initialize CARMA parameters.

        Args:
            max_bins: Maximum number of size bins (default: 100)
            max_groups: Maximum number of groups for fractal dimension (default: 10)
            nz: Number of vertical levels (default: 1)
            ny: Number of y-direction grid points (default: 1)
            nx: Number of x-direction grid points (default: 1)
            nelem: Number of elements (default: 1)
            ngroup: Number of groups (default: 1)
            nbin: Number of size bins (default: 5)
            nsolute: Number of solutes (default: 0)
            ngas: Number of gases (default: 0)
            nwave: Number of wavelengths for optics (default: 30)
            dtime: Time step in seconds (default: 1800.0)
            nstep: Number of time steps (default: 100)
            deltaz: Vertical grid spacing in meters (default: 1000.0)
            zmin: Minimum altitude in meters (default: 16500.0)
        """
        self.max_bins = max_bins
        self.max_groups = max_groups
        self.nz = nz
        self.ny = ny
        self.nx = nx
        self.nelem = nelem
        self.ngroup = ngroup
        self.nbin = nbin
        self.nsolute = nsolute
        self.ngas = ngas
        self.nwave = nwave
        self.dtime = dtime
        self.nstep = nstep
        self.deltaz = deltaz
        self.zmin = zmin

    def to_dict(self) -> Dict:
        """Convert parameters to dictionary for C++ interface."""
        # Use introspection to get all instance attributes except methods and built-ins
        return {k: v for k, v in self.__dict__.items() if not k.startswith('__') and not callable(v)}

    @classmethod
    def from_dict(cls, params_dict: Dict) -> 'CARMAParameters':
        """Create parameters from dictionary."""
        return cls(**params_dict)


class CARMA:
    """
    A Python interface to the CARMA aerosol model.

    This class provides a simplified interface for running CARMA simulations
    with configurable parameters.
    """

    def __init__(self):
        """
        Initialize a CARMA instance.

        Raises:
            ValueError: If CARMA backend is not available
        """
        if not backend.carma_available():
            raise ValueError(
                "CARMA backend is not available on this platform.")

        self._carma_instance = _backend._carma._create_carma()

    def __del__(self):
        """Clean up the CARMA instance."""
        if hasattr(self, '_carma_instance') and self._carma_instance is not None:
            _backend._carma._delete_carma(self._carma_instance)

    def run(self, parameters: CARMAParameters) -> None:
        """
        Run the CARMA aerosol model simulation.

        Args:
            parameters: CARMAParameters instance containing simulation configuration

        Raises:
            ValueError: If the simulation fails
        """
        params_dict = parameters.to_dict()
        _backend._carma._run_carma_with_parameters(
            self._carma_instance, params_dict)

    @staticmethod
    def get_aluminum_test_parameters() -> CARMAParameters:
        """
        Get parameters for aluminum test configuration.

        Returns:
            CARMAParameters configured for aluminum test
        """
        if not backend.carma_available():
            raise ValueError(
                "CARMA backend is not available on this platform.")

        params_dict = _backend._carma._get_aluminum_test_params()
        return CARMAParameters.from_dict(params_dict)

    @staticmethod
    def get_fractal_optics_test_parameters() -> CARMAParameters:
        """
        Get parameters for fractal optics test configuration.

        Returns:
            CARMAParameters configured for fractal optics test
        """
        if not backend.carma_available():
            raise ValueError(
                "CARMA backend is not available on this platform.")

        params_dict = _backend._carma._get_fractal_optics_test_params()
        return CARMAParameters.from_dict(params_dict)

    @staticmethod
    def get_sulfate_test_parameters() -> CARMAParameters:
        """
        Get parameters for sulfate test configuration.

        Returns:
            CARMAParameters configured for sulfate test
        """
        if not backend.carma_available():
            raise ValueError(
                "CARMA backend is not available on this platform.")

        params_dict = _backend._carma._get_sulfate_test_params()
        return CARMAParameters.from_dict(params_dict)
