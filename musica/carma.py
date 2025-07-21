"""
CARMA aerosol model Python interface.

This module provides a simplified Python interface to the CARMA aerosol model.
It allows users to create a CARMA instance and run simulations with specified parameters.

Note: CARMA is only available on macOS and Linux platforms.
"""

from typing import Dict, Optional, List
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

    def __repr__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(max_bins={self.max_bins}, max_groups={self.max_groups}, "
                f"nz={self.nz}, ny={self.ny}, nx={self.nx}, nelem={self.nelem}, "
                f"ngroup={self.ngroup}, nbin={self.nbin}, nsolute={self.nsolute}, "
                f"ngas={self.ngas}, nwave={self.nwave}, dtime={self.dtime}, "
                f"nstep={self.nstep}, deltaz={self.deltaz}, zmin={self.zmin})")
    
    def __str__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(max_bins={self.max_bins}, max_groups={self.max_groups}, "
                f"nz={self.nz}, ny={self.ny}, nx={self.nx}, nelem={self.nelem}, "
                f"ngroup={self.ngroup}, nbin={self.nbin}, nsolute={self.nsolute}, "
                f"ngas={self.ngas}, nwave={self.nwave}, dtime={self.dtime}, "
                f"nstep={self.nstep}, deltaz={self.deltaz}, zmin={self.zmin})")

    def to_dict(self) -> Dict:
        """Convert parameters to dictionary for C++ interface."""
        # Use introspection to get all instance attributes except methods and built-ins
        return {k: v for k, v in self.__dict__.items() if not k.startswith('__') and not callable(v)}

    @classmethod
    def from_dict(cls, params_dict: Dict) -> 'CARMAParameters':
        """Create parameters from dictionary."""
        return cls(**params_dict)


class CARMAOutput:
    """
    Output from CARMA aerosol model simulation.

    This class contains all the output data from a CARMA simulation,
    including atmospheric state, aerosol properties, and diagnostic variables.
    """

    def __init__(self):
        """Initialize empty CARMA output structure."""
        # Dimensions for validation
        self.nz: int = 0
        self.ny: int = 0
        self.nx: int = 0
        self.nelem: int = 0
        self.ngroup: int = 0
        self.nbin: int = 0
        self.ngas: int = 0
        self.nstep: int = 0

        # Grid and coordinate arrays
        self.lat: List[float] = []  # Latitude [degrees]
        self.lon: List[float] = []  # Longitude [degrees]
        self.vertical_center: List[float] = []  # Height at cell centers [m]
        self.vertical_levels: List[float] = []  # Height at cell interfaces [m]

        # Atmospheric state variables (nz elements)
        self.pressure: List[float] = []  # Pressure [Pa]
        self.temperature: List[float] = []  # Temperature [K]
        self.air_density: List[float] = []  # Air density [kg/m3]
        self.radiative_heating: List[float] = []  # Radiative heating [K/s]
        self.delta_temperature: List[float] = []  # Temperature change [K]

        # Gas variables (nz x ngas)
        self.gas_mmr: List[List[float]] = []  # Gas mass mixing ratio [kg/kg]
        # Saturation over liquid
        self.gas_saturation_liquid: List[List[float]] = []
        self.gas_saturation_ice: List[List[float]] = []  # Saturation over ice
        # Evaporation rate over ice
        self.gas_vapor_pressure_ice: List[List[float]] = []
        # Evaporation rate over liquid
        self.gas_vapor_pressure_liquid: List[List[float]] = []
        self.gas_weight_percent: List[List[float]] = []  # Gas weight

        # Group-integrated variables (nz x ngroup)
        self.number_density: List[List[float]] = []  # Number density [#/cm3]
        # Surface area density [cm2/cm3]
        self.surface_area: List[List[float]] = []
        self.mass_density: List[List[float]] = []  # Mass density [g/cm3]
        self.effective_radius: List[List[float]] = []  # Effective radius [cm]
        # Wet effective radius [cm]
        self.effective_radius_wet: List[List[float]] = []
        self.mean_radius: List[List[float]] = []  # Mean radius [cm]
        # Nucleation rate [#/cm3/s]
        self.nucleation_rate: List[List[float]] = []
        # Mass mixing ratio [kg/kg]
        self.mass_mixing_ratio: List[List[float]] = []
        self.projected_area: List[List[float]] = []  # Projected area [cm2/cm3]
        self.aspect_ratio: List[List[float]] = []  # Aspect ratio
        # Vertical mass flux [g/cm2/s]
        self.vertical_mass_flux: List[List[float]] = []
        # Extinction coefficient [1/km]
        self.extinction: List[List[float]] = []
        self.optical_depth: List[List[float]] = []  # Optical depth

        # Bin-resolved variables (nz x ngroup x nbin)
        self.bin_wet_radius: List[List[List[float]]] = []  # Wet radius [um]
        # Number density [#/cm3]
        self.bin_number_density: List[List[List[float]]] = []
        # Particle density [g/cm3]
        self.bin_density: List[List[List[float]]] = []
        # Mass mixing ratio [kg/kg]
        self.bin_mass_mixing_ratio: List[List[List[float]]] = []
        # Deposition velocity [cm/s]
        self.bin_deposition_velocity: List[List[List[float]]] = []

        # Group properties (constant for each group)
        # Bin center radius [cm] (nbin x ngroup)
        self.group_radius: List[List[float]] = []
        self.group_mass: List[List[float]] = []  # Bin mass [g] (nbin x ngroup)
        # Bin volume [cm3] (nbin x ngroup)
        self.group_volume: List[List[float]] = []
        # Radius ratio (nbin x ngroup)
        self.group_radius_ratio: List[List[float]] = []
        # Aspect ratio (nbin x ngroup)
        self.group_aspect_ratio: List[List[float]] = []
        # Fractal dimension (nbin x ngroup)
        self.group_fractal_dimension: List[List[float]] = []

        # Element and group names for identification
        self.element_names: List[str] = []
        self.group_names: List[str] = []
        self.gas_names: List[str] = []
    
    @classmethod
    def from_dict(cls, output_dict: Dict) -> 'CARMAOutput':
        """Create CARMAOutput from dictionary returned by C++ backend."""
        output = cls()

        # Set all attributes from the dictionary
        for key, value in output_dict.items():
            if hasattr(output, key):
                setattr(output, key, value)

        return output


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

    def __repr__(self):
        """String representation of CARMA instance."""
        return f"CARMA() - Version: {version if version else 'Not available'}"
    
    def __str__(self):
        """String representation of CARMA instance."""
        return f"CARMA() - Version: {version if version else 'Not available'}"

    def run(self, parameters: CARMAParameters) -> CARMAOutput:
        """
        Run the CARMA aerosol model simulation.

        Args:
            parameters: CARMAParameters instance containing simulation configuration

        Returns:
            CARMAOutput: Output data from the CARMA simulation

        Raises:
            ValueError: If the simulation fails
        """
        params_dict = parameters.to_dict()
        output_dict = _backend._carma._run_carma_with_parameters(
            self._carma_instance, params_dict)

        # Create CARMAOutput from the returned dictionary
        output = CARMAOutput.from_dict(output_dict)

        return output

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
