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
    ngroup = output_dict.get('ngroup', 0)
    ngas = output_dict.get('ngas', 0)
    nelem = output_dict.get('nelem', 0)
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
    if nelem > 0:
        coords['elem'] = ('elem', list(range(1, nelem + 1)))

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
            'nelem': nelem,
            'ngroup': ngroup,
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
