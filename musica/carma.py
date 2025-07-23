"""
CARMA aerosol model Python interface.

This module provides a simplified Python interface to the CARMA aerosol model.
It allows users to create a CARMA instance and run simulations with specified parameters.

Note: CARMA is only available on macOS and Linux platforms.
"""

from typing import Dict, Optional, List
import numpy as np
import xarray as xr
from . import backend

_backend = backend.get_backend()

version = _backend._carma._get_carma_version() if backend.carma_available() else None

class ParticleShape:
    """Enumeration for particle shapes used in CARMA."""
    SPHERE = 1
    HEXAGON = 2
    CYLINDER = 3


class ParticleType:
    """Enumeration for particle types used in CARMA."""
    INVOLATILE = 1
    VOLATILE = 2
    CORE_MASS = 3
    VOLATILE_CORE = 4
    CORE_MASS_TWO_MOMENTS = 5


class ParticleComposition:
    """Enumeration for particle compositions used in CARMA."""
    ALUMINUM = 1
    SULFURIC_ACID = 2
    DUST = 3
    ICE = 4
    WATER = 5
    BLACK_CARBON = 6
    ORGANIC_CARBON = 7
    OTHER = 8


class CARMAGroupConfig:
    """Configuration for a CARMA particle group.
    
    A CARMA particle group represents a collection of particles with similar properties.
    """

    def __init__(self,
                 id: int = 1,
                 name: str = "default_group",
                 shortname: str = "",
                 rmin: float = 1e-7,
                 rmrat: float = 2.0,
                 ishape: int = ParticleShape.SPHERE,
                 eshape: float = 1.0,
                 is_ice: bool = False,
                 is_fractal: bool = False,
                 do_mie: bool = True,
                 do_wetdep: bool = False,
                 do_drydep: bool = False,
                 do_vtran: bool = True,
                 solfac: float = 0.0,
                 scavcoef: float = 0.0,
                 rmon: float = 0.0,
                 df: Optional[List[float]] = None,
                 falpha: float = 1.0):
        """
        Initialize a CARMA group configuration.

        Args:
            id: Unique identifier for the group (default: 1)
            name: Name of the group (default: "default_group")
            shortname: Short name for the group (default: "")
            rmin: Radius of particles in the first bin [cm] (default: 1e-7)
            rmrat: Ratio of masses of particles in consecutive bins (default: 2.0)
            ishape: Shape of the particles (default: ParticleShape.SPHERE)
            eshape: Ratio of particle length / diameter (default: 1.0)
            is_ice: Whether the particles are ice (default: False)
            is_fractal: Whether the particles are fractal (default: False)
            do_mie: Whether to do Mie calculations (default: True)
            do_wetdep: Whether to include wet deposition (default: False)
            do_drydep: Whether to include dry deposition (default: False)
            do_vtran: Whether to include vertical transport (default: True)
            solfac: Solubility factor for wet deposition (default: 0.0)
            scavcoef: Scavenging coefficient for wet deposition [mm-1] (default: 0.0)
            rmon: Monomer radius of fractal particles [cm] (default: 0.0)
            df: List of fractal dimensions for each size bin (default: None)
            falpha: Fractal packing coefficient (default: 1.0)
        """
        self.id = id
        self.name = name
        self.shortname = shortname
        self.rmin = rmin
        self.rmrat = rmrat
        self.ishape = ishape
        self.eshape = eshape
        self.is_ice = is_ice
        self.is_fractal = is_fractal
        self.do_mie = do_mie
        self.do_wetdep = do_wetdep
        self.do_drydep = do_drydep
        self.do_vtran = do_vtran
        self.solfac = solfac
        self.scavcoef = scavcoef
        self.rmon = rmon
        self.df = df or []
        self.falpha = falpha

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: v for k, v in self.__dict__.items()}


class CARMAElementConfig:
    """Configuration for a CARMA particle element.

    A CARMA particle element represents one of the components of a cloud or aerosol particle.
    """

    def __init__(self,
                 id: int = 1,
                 igroup: int = 1,
                 name: str = "default_element",
                 shortname: str = "",
                 rho: float = 1.0,
                 itype: int = ParticleType.INVOLATILE,
                 icomposition: int = ParticleComposition.OTHER,
                 isolute: int = 0,
                 rhobin: Optional[List[float]] = None,
                 arat: Optional[List[float]] = None,
                 kappa: float = 0.0,
                 is_shell: bool = True):
        """
        Initialize a CARMA element configuration.

        Args:
            id: Unique identifier for the element (default: 1)
            igroup: Group ID this element belongs to (default: 1)
            name: Name of the element (default: "default_element")
            shortname: Short name for the element (default: "")
            rho: Density of the element in g/cm3 (default: 1.0)
            itype: Type of the particle (default: ParticleType.INVOLATILE)
            icomposition: Composition of the particle (default: ParticleComposition.OTHER)
            isolute: Index of the solute (default: 0)
            rhobin: List of densities for each size bin (default: None)
            arat: List of area ratios for each size bin (default: None)
            kappa: Hygroscopicity parameter (default: 0.0)
            is_shell: For core/shell optics, whether this element is part of the shell (True) or core (False) (default: True)
        """
        self.id = id
        self.igroup = igroup
        self.name = name
        self.shortname = shortname
        self.rho = rho
        self.itype = itype
        self.icomposition = icomposition
        self.isolute = isolute
        self.rhobin = rhobin or []
        self.arat = arat or []
        self.kappa = kappa
        self.is_shell = is_shell

    def to_dict(self) -> Dict:
        """Convert to dictionary."""
        return {k: v for k, v in self.__dict__.items()}


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
                 nbin: int = 5,
                 nsolute: int = 0,
                 ngas: int = 0,
                 nwave: int = 30,
                 idx_wave: int = 0,
                 dtime: float = 1800.0,
                 nstep: int = 100,
                 deltaz: float = 1000.0,
                 zmin: float = 16500.0,
                 extinction_coefficient: Optional[List[List[List[float]]]] = None,
                 groups: Optional[List[CARMAGroupConfig]] = None,
                 elements: Optional[List[CARMAElementConfig]] = None):
        """
        Initialize CARMA parameters.

        Args:
            max_bins: Maximum number of size bins (default: 100)
            max_groups: Maximum number of groups for fractal dimension (default: 10)
            nz: Number of vertical levels (default: 1)
            ny: Number of y-direction grid points (default: 1)
            nx: Number of x-direction grid points (default: 1)
            nbin: Number of size bins (default: 5)
            nsolute: Number of solutes (default: 0)
            ngas: Number of gases (default: 0)
            nwave: Number of wavelengths for optics (default: 30)
            idx_wave: Index of wavelength for extinction coefficient (default: 0)
            dtime: Time step in seconds (default: 1800.0)
            nstep: Number of time steps (default: 100)
            deltaz: Vertical grid spacing in meters (default: 1000.0)
            zmin: Minimum altitude in meters (default: 16500.0)
            extinction_coefficient: Extinction coefficient qext [NWAVE x NBIN x NGROUP] (default: None)
            groups: List of group configurations (default: None)
            elements: List of element configurations (default: None)
        """
        self.max_bins = max_bins
        self.max_groups = max_groups
        self.nz = nz
        self.ny = ny
        self.nx = nx
        self.nbin = nbin
        self.nsolute = nsolute
        self.ngas = ngas
        self.nwave = nwave
        self.idx_wave = idx_wave
        self.dtime = dtime
        self.nstep = nstep
        self.deltaz = deltaz
        self.zmin = zmin
        self.extinction_coefficient = extinction_coefficient

        # Initialize group and element configurations
        self.groups = groups or []
        self.elements = elements or []

    def add_group(self, group: CARMAGroupConfig):
        """Add a group configuration."""
        self.groups.append(group)
        self.update_dimensions()

    def add_element(self, element: CARMAElementConfig):
        """Add an element configuration."""
        self.elements.append(element)
        self.update_dimensions()

    def __repr__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(max_bins={self.max_bins}, max_groups={self.max_groups}, "
                f"nz={self.nz}, ny={self.ny}, nx={self.nx}, "
                f"nbin={self.nbin}, nsolute={self.nsolute}, "
                f"ngas={self.ngas}, nwave={self.nwave}, dtime={self.dtime}, "
                f"nstep={self.nstep}, deltaz={self.deltaz}, zmin={self.zmin})")

    def __str__(self):
        """String representation of CARMAParameters."""
        return (f"CARMAParameters(max_bins={self.max_bins}, max_groups={self.max_groups}, "
                f"nz={self.nz}, ny={self.ny}, nx={self.nx}, "
                f"nbin={self.nbin}, nsolute={self.nsolute}, "
                f"ngas={self.ngas}, nwave={self.nwave}, dtime={self.dtime}, "
                f"nstep={self.nstep}, deltaz={self.deltaz}, zmin={self.zmin})")

    def to_dict(self) -> Dict:
        """Convert parameters to dictionary for C++ interface."""
        # Get all basic attributes
        params_dict = {}
        for k, v in self.__dict__.items():
            if not k.startswith('__') and not callable(v):
                if k == 'groups':
                    params_dict[k] = [group.to_dict() for group in v]
                elif k == 'elements':
                    params_dict[k] = [element.to_dict() for element in v]
                else:
                    params_dict[k] = v
        return params_dict

    @classmethod
    def from_dict(cls, params_dict: Dict) -> 'CARMAParameters':
        """Create parameters from dictionary."""
        # Handle groups and elements separately
        groups = []
        if 'groups' in params_dict:
            groups = [CARMAGroupConfig(**group_dict)
                      for group_dict in params_dict['groups']]
            del params_dict['groups']

        elements = []
        if 'elements' in params_dict:
            elements = [CARMAElementConfig(**element_dict)
                        for element_dict in params_dict['elements']]
            del params_dict['elements']

        return cls(groups=groups, elements=elements, **params_dict)

    @classmethod
    def create_aluminum_test_config(cls) -> 'CARMAParameters':
        """Create parameters for aluminum test configuration."""
        # Create aluminum group
        group = CARMAGroupConfig(
            id=1,
            name="aluminum",
            shortname="PRALUM",
            rmin=21.5e-6,
            rmrat=2.0,
            ishape=ParticleShape.SPHERE,
            eshape=1.0,
            is_ice=False,
            is_fractal=True,
            do_mie=True,
            do_wetdep=False,
            do_drydep=True,
            do_vtran=True,
            solfac=0.0,
            scavcoef=0.0,
            rmon=21.5e-6,
            df=[1.6] * 5,  # 5 bins with fractal dimension 1.6
            falpha=1.0
        )

        # Create aluminum element
        element = CARMAElementConfig(
            id=1,
            igroup=1,
            name="Aluminum",
            shortname="ALUM",
            rho=3.95,  # g/cm3
            itype= ParticleType.INVOLATILE,
            icomposition= ParticleComposition.ALUMINUM,
            isolute=0,
            rhobin=[],
            arat=[],
            kappa=0.0,
            is_shell=True
        )

        params = cls(
            max_bins=100,
            max_groups=10,
            nz=1,
            ny=1,
            nx=1,
            nbin=5,
            nsolute=0,
            ngas=0,
            nwave=30,
            idx_wave=0,
            deltaz=1000.0,
            zmin=16500.0,
            extinction_coefficient=None,  # Not used in this test
            groups=[group],
            elements=[element]
        )

        params.dtime=1800.0
        params.nstep=432000 / params.dtime

        return params


def _carma_dict_to_xarray(output_dict: Dict) -> xr.Dataset:
    """
    Convert CARMA output dictionary to xarray Dataset for netCDF export.

    This function creates an xarray Dataset with proper dimensions and coordinates
    following the structure used in the CARMA Fortran aluminum test.

    Args:
        output_dict: Dictionary containing CARMA output data from C++ backend

    Returns:
        xr.Dataset: Dataset containing all CARMA output variables with proper
                   dimensions and metadata
    """
    # Extract dimensions from the dictionary
    nz = output_dict.get('nz', 0)
    ny = output_dict.get('ny', 0)
    nx = output_dict.get('nx', 0)
    nbin = output_dict.get('nbin', 0)
    ngas = output_dict.get('ngas', 0)
    nstep = output_dict.get('nstep', 0)

    # Create coordinates
    coords = {}

    # Spatial coordinates
    lat = output_dict.get('lat', [])
    lon = output_dict.get('lon', [])
    vertical_center = output_dict.get('vertical_center', [])
    vertical_levels = output_dict.get('vertical_levels', [])

    if lat:
        coords['lat'] = ('y', lat)
    if lon:
        coords['lon'] = ('x', lon)
    if vertical_center:
        coords['z'] = ('z', vertical_center)
    if vertical_levels:
        coords['z_levels'] = ('z_levels', vertical_levels)

    # Bin coordinates (1-indexed like Fortran)
    coords['bin'] = ('bin', list(range(1, nbin + 1)))
    coords['group'] = ('group', list(range(1, ngroup + 1)))

    if ngas > 0:
        coords['gas'] = ('gas', list(range(1, ngas + 1)))

    # Initialize data variables dictionary
    data_vars = {}

    # Atmospheric state variables
    pressure = output_dict.get('pressure', [])
    if pressure:
        data_vars['p'] = (
            'z', pressure, {'units': 'Pa', 'long_name': 'Pressure'})

    temperature = output_dict.get('temperature', [])
    if temperature:
        data_vars['t'] = ('z', temperature, {
                          'units': 'K', 'long_name': 'Temperature'})

    air_density = output_dict.get('air_density', [])
    if air_density:
        data_vars['rhoa'] = ('z', air_density, {
                             'units': 'kg/m3', 'long_name': 'Air density'})

    radiative_heating = output_dict.get('radiative_heating', [])
    if radiative_heating:
        data_vars['rlheat'] = ('z', radiative_heating, {
                               'units': 'K/s', 'long_name': 'Radiative heating'})

    delta_temperature = output_dict.get('delta_temperature', [])
    if delta_temperature:
        data_vars['dT'] = ('z', delta_temperature, {
                           'units': 'K', 'long_name': 'Temperature change'})

    # Gas variables (if any gases present)
    gas_mmr = output_dict.get('gas_mmr', [])
    if ngas > 0 and gas_mmr:
        data_vars['gas_mmr'] = (('z', 'gas'), np.array(gas_mmr),
                                {'units': 'kg/kg', 'long_name': 'Gas mass mixing ratio'})

    gas_saturation_liquid = output_dict.get('gas_saturation_liquid', [])
    if ngas > 0 and gas_saturation_liquid:
        data_vars['gas_satliq'] = (('z', 'gas'), np.array(gas_saturation_liquid),
                                   {'long_name': 'Gas saturation over liquid'})

    gas_saturation_ice = output_dict.get('gas_saturation_ice', [])
    if ngas > 0 and gas_saturation_ice:
        data_vars['gas_satice'] = (('z', 'gas'), np.array(gas_saturation_ice),
                                   {'long_name': 'Gas saturation over ice'})

    gas_weight_percent = output_dict.get('gas_weight_percent', [])
    if ngas > 0 and gas_weight_percent:
        data_vars['gas_wt'] = (('z', 'gas'), np.array(gas_weight_percent),
                               {'units': '%', 'long_name': 'Gas weight percent'})

    # Group-integrated variables
    number_density = output_dict.get('number_density', [])
    if number_density:
        data_vars['nd'] = (('z', 'group'), np.array(number_density),
                           {'units': '#/cm3', 'long_name': 'Number density'})

    surface_area = output_dict.get('surface_area', [])
    if surface_area:
        data_vars['ad'] = (('z', 'group'), np.array(surface_area),
                           {'units': 'cm2/cm3', 'long_name': 'Surface area density'})

    mass_density = output_dict.get('mass_density', [])
    if mass_density:
        data_vars['md'] = (('z', 'group'), np.array(mass_density),
                           {'units': 'g/cm3', 'long_name': 'Mass density'})

    effective_radius = output_dict.get('effective_radius', [])
    if effective_radius:
        data_vars['re'] = (('z', 'group'), np.array(effective_radius),
                           {'units': 'cm', 'long_name': 'Effective radius'})

    effective_radius_wet = output_dict.get('effective_radius_wet', [])
    if effective_radius_wet:
        data_vars['rew'] = (('z', 'group'), np.array(effective_radius_wet),
                            {'units': 'cm', 'long_name': 'Wet effective radius'})

    mean_radius = output_dict.get('mean_radius', [])
    if mean_radius:
        data_vars['rm'] = (('z', 'group'), np.array(mean_radius),
                           {'units': 'cm', 'long_name': 'Mean radius'})

    mass_mixing_ratio = output_dict.get('mass_mixing_ratio', [])
    if mass_mixing_ratio:
        data_vars['mr'] = (('z', 'group'), np.array(mass_mixing_ratio),
                           {'units': 'kg/kg', 'long_name': 'Mass mixing ratio'})

    projected_area = output_dict.get('projected_area', [])
    if projected_area:
        data_vars['pa'] = (('z', 'group'), np.array(projected_area),
                           {'units': 'cm2/cm3', 'long_name': 'Projected area'})

    aspect_ratio = output_dict.get('aspect_ratio', [])
    if aspect_ratio:
        data_vars['ar'] = (('z', 'group'), np.array(aspect_ratio),
                           {'long_name': 'Aspect ratio'})

    vertical_mass_flux = output_dict.get('vertical_mass_flux', [])
    if vertical_mass_flux:
        data_vars['vm'] = (('z', 'group'), np.array(vertical_mass_flux),
                           {'units': 'g/cm2/s', 'long_name': 'Vertical mass flux'})

    extinction = output_dict.get('extinction', [])
    if extinction:
        data_vars['ex_vis'] = (('z', 'group'), np.array(extinction),
                               {'units': '1/km', 'long_name': 'Extinction coefficient'})

    optical_depth = output_dict.get('optical_depth', [])
    if optical_depth:
        data_vars['od_vis'] = (('z', 'group'), np.array(optical_depth),
                               {'long_name': 'Optical depth'})

    # Bin-resolved variables
    bin_wet_radius = output_dict.get('bin_wet_radius', [])
    if bin_wet_radius:
        data_vars['wr_bin'] = (('z', 'group', 'bin'), np.array(bin_wet_radius),
                               {'units': 'um', 'long_name': 'Bin wet radius'})

    bin_number_density = output_dict.get('bin_number_density', [])
    if bin_number_density:
        data_vars['nd_bin'] = (('z', 'group', 'bin'), np.array(bin_number_density),
                               {'units': '#/cm3', 'long_name': 'Bin number density'})

    bin_density = output_dict.get('bin_density', [])
    if bin_density:
        data_vars['ro_bin'] = (('z', 'group', 'bin'), np.array(bin_density),
                               {'units': 'g/cm3', 'long_name': 'Bin particle density'})

    bin_mass_mixing_ratio = output_dict.get('bin_mass_mixing_ratio', [])
    if bin_mass_mixing_ratio:
        data_vars['mr_bin'] = (('z', 'group', 'bin'), np.array(bin_mass_mixing_ratio),
                               {'units': 'kg/kg', 'long_name': 'Bin mass mixing ratio'})

    bin_deposition_velocity = output_dict.get('bin_deposition_velocity', [])
    if bin_deposition_velocity:
        data_vars['vd_bin'] = (('z', 'group', 'bin'), np.array(bin_deposition_velocity),
                               {'units': 'cm/s', 'long_name': 'Bin deposition velocity'})

    # Group properties (constant arrays)
    group_radius = output_dict.get('group_radius', [])
    if group_radius:
        # Convert to numpy array and check dimensions
        arr = np.array(group_radius)
        if arr.ndim == 2:
            # If it's already (nbin, ngroup), use as-is
            if arr.shape == (nbin, ngroup):
                data_vars['r'] = (('bin', 'group'), arr,
                                  {'units': 'cm', 'long_name': 'Bin center radius'})
            # If it's (ngroup, nbin), transpose it
            elif arr.shape == (ngroup, nbin):
                data_vars['r'] = (('bin', 'group'), arr.T,
                                  {'units': 'cm', 'long_name': 'Bin center radius'})
        elif arr.ndim == 1:
            # If it's 1D, reshape to (nbin, 1) assuming single group
            data_vars['r'] = (('bin', 'group'), arr.reshape(-1, 1),
                              {'units': 'cm', 'long_name': 'Bin center radius'})

    group_mass = output_dict.get('group_mass', [])
    if group_mass:
        arr = np.array(group_mass)
        if arr.ndim == 2:
            if arr.shape == (nbin, ngroup):
                data_vars['rmass'] = (('bin', 'group'), arr,
                                      {'units': 'g', 'long_name': 'Bin mass'})
            elif arr.shape == (ngroup, nbin):
                data_vars['rmass'] = (('bin', 'group'), arr.T,
                                      {'units': 'g', 'long_name': 'Bin mass'})
        elif arr.ndim == 1:
            data_vars['rmass'] = (('bin', 'group'), arr.reshape(-1, 1),
                                  {'units': 'g', 'long_name': 'Bin mass'})

    group_volume = output_dict.get('group_volume', [])
    if group_volume:
        arr = np.array(group_volume)
        if arr.ndim == 2:
            if arr.shape == (nbin, ngroup):
                data_vars['vol'] = (('bin', 'group'), arr,
                                    {'units': 'cm3', 'long_name': 'Bin volume'})
            elif arr.shape == (ngroup, nbin):
                data_vars['vol'] = (('bin', 'group'), arr.T,
                                    {'units': 'cm3', 'long_name': 'Bin volume'})
        elif arr.ndim == 1:
            data_vars['vol'] = (('bin', 'group'), arr.reshape(-1, 1),
                                {'units': 'cm3', 'long_name': 'Bin volume'})

    group_radius_ratio = output_dict.get('group_radius_ratio', [])
    if group_radius_ratio:
        arr = np.array(group_radius_ratio)
        if arr.ndim == 2:
            if arr.shape == (nbin, ngroup):
                data_vars['rrat'] = (('bin', 'group'), arr,
                                     {'long_name': 'Radius ratio'})
            elif arr.shape == (ngroup, nbin):
                data_vars['rrat'] = (('bin', 'group'), arr.T,
                                     {'long_name': 'Radius ratio'})
        elif arr.ndim == 1:
            data_vars['rrat'] = (('bin', 'group'), arr.reshape(-1, 1),
                                 {'long_name': 'Radius ratio'})

    group_aspect_ratio = output_dict.get('group_aspect_ratio', [])
    if group_aspect_ratio:
        arr = np.array(group_aspect_ratio)
        if arr.ndim == 2:
            if arr.shape == (nbin, ngroup):
                data_vars['arat'] = (('bin', 'group'), arr,
                                     {'long_name': 'Aspect ratio'})
            elif arr.shape == (ngroup, nbin):
                data_vars['arat'] = (('bin', 'group'), arr.T,
                                     {'long_name': 'Aspect ratio'})
        elif arr.ndim == 1:
            data_vars['arat'] = (('bin', 'group'), arr.reshape(-1, 1),
                                 {'long_name': 'Aspect ratio'})

    group_fractal_dimension = output_dict.get('group_fractal_dimension', [])
    if group_fractal_dimension:
        arr = np.array(group_fractal_dimension)
        if arr.ndim == 2:
            if arr.shape == (nbin, ngroup):
                data_vars['df'] = (('bin', 'group'), arr,
                                   {'long_name': 'Fractal dimension'})
            elif arr.shape == (ngroup, nbin):
                data_vars['df'] = (('bin', 'group'), arr.T,
                                   {'long_name': 'Fractal dimension'})
        elif arr.ndim == 1:
            data_vars['df'] = (('bin', 'group'), arr.reshape(-1, 1),
                               {'long_name': 'Fractal dimension'})

    # Create the dataset
    ds = xr.Dataset(
        data_vars=data_vars,
        coords=coords,
        attrs={
            'title': 'CARMA aerosol model output',
            'description': 'Output from CARMA aerosol simulation',
            'nz': nz,
            'ny': ny,
            'nx': nx,
            'nbin': nbin,
            'ngas': ngas,
            'nstep': nstep
        }
    )

    return ds


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

    def run(self, parameters: CARMAParameters) -> xr.Dataset:
        """
        Run the CARMA aerosol model simulation.

        Args:
            parameters: CARMAParameters instance containing simulation configuration

        Returns:
            xr.Dataset: Dataset containing all CARMA output variables with proper
                       dimensions and metadata

        Raises:
            ValueError: If the simulation fails
        """
        params_dict = parameters.to_dict()
        output_dict = _backend._carma._run_carma_with_parameters(
            self._carma_instance, params_dict)

        # Convert dictionary directly to xarray Dataset
        return _carma_dict_to_xarray(output_dict)