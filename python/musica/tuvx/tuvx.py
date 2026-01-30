"""
TUV-x photolysis calculator Python interface.

This module provides a simplified Python interface to the TUV-x photolysis calculator.
It allows users to create a TUV-x instance from a JSON configuration file and
calculate photolysis rates and heating rates.

Note: TUV-x is only available on macOS and Linux platforms.
"""

import os
import json
import tempfile
from typing import Dict, Optional
import numpy as np
import xarray as xr
from .. import backend

_backend = backend.get_backend()

# Import the GridMap class from the backend
if backend.tuvx_available():
    from .grid_map import GridMap
    from .profile import Profile
    from .profile_map import ProfileMap
    from .radiator import Radiator
    from .radiator_map import RadiatorMap
else:
    GridMap = None
    Profile = None
    ProfileMap = None
    Radiator = None
    RadiatorMap = None


class TUVX:
    """
    A Python interface to the TUV-x photolysis calculator.

    This class provides a simplified interface that only requires a JSON/YAML configuration
    to set up and run TUV-x calculations. All parameters (solar zenith angle,
    earth-sun distance, atmospheric profiles, etc.) are specified in the JSON/YAML configuration.

    The configuration can be in a file or provided as a string. Exactly one of `config_path` or
    `config_string` must be provided.
    """

    def __init__(self,
                 grid_map: GridMap,
                 profile_map: ProfileMap,
                 radiator_map: RadiatorMap,
                 config_path: Optional[str] = None,
                 config_string: Optional[str] = None):
        """
        Initialize a TUV-x instance from a configuration file.

        Args:
            grid_map: GridMap instance containing grid definitions (height, wavelength)
            profile_map: ProfileMap instance containing atmospheric profiles (temperature, species concentrations, surface albedo, ET flux)
            radiator_map: RadiatorMap instance containing radiator definitions (optically active species)
            config_path: Path to the JSON configuration file (files paths in the json/yaml must be absolute or relative to the config file location)
            config_string: JSON configuration as a string

        Raises:
            FileNotFoundError: If the configuration file doesn't exist
            ValueError: If TUV-x initialization fails or TUVX is not available
        """
        if not backend.tuvx_available():
            raise ValueError("TUV-x backend is not available.")

        if (config_path is None and config_string is None) or \
           (config_path is not None and config_string is not None):
            raise ValueError(
                "Exactly one of config_path or config_string must be provided.")

        if config_path is not None and not os.path.exists(config_path):
            raise FileNotFoundError(
                f"Configuration file not found: {config_path}")

        if config_path is not None:
            # Change the working directory to the config file's directory
            original_cwd = os.getcwd()
            new_cwd = os.path.dirname(config_path)
            if new_cwd:
                os.chdir(new_cwd)
            file_path = os.path.basename(config_path)
            try:
                self._tuvx_instance = _backend._tuvx._create_tuvx_from_file(
                    file_path, grid_map, profile_map, radiator_map)
            finally:
                os.chdir(original_cwd)
            self._config_path = config_path
            self._config_string = None
        elif config_string is not None:
            self._tuvx_instance = _backend._tuvx._create_tuvx_from_string(
                config_string, grid_map, profile_map, radiator_map)
            self._config_path = None
            self._config_string = config_string

        # Cache the names for efficiency
        self._photolysis_names = None
        self._heating_names = None
        self._dose_names = None

    def __del__(self):
        """Clean up the TUV-x instance."""
        if hasattr(self, '_tuvx_instance') and self._tuvx_instance is not None:
            _backend._tuvx._delete_tuvx(self._tuvx_instance)

    @property
    def photolysis_rate_names(self) -> dict[str, int]:
        """
        Get the names of photolysis rates.

        Returns:
            Dictionary mapping photolysis rate names to their indices in the output arrays
        """
        if self._photolysis_names is None:
            self._photolysis_names = _backend._tuvx._get_photolysis_rate_constants_ordering(
                self._tuvx_instance)
        return self._photolysis_names

    @property
    def heating_rate_names(self) -> dict[str, int]:
        """
        Get the names of heating rates.

        Returns:
            Dictionary mapping heating rate names to their indices in the output arrays
        """
        if self._heating_names is None:
            self._heating_names = _backend._tuvx._get_heating_rates_ordering(
                self._tuvx_instance)
        return self._heating_names

    @property
    def dose_rate_names(self) -> dict[str, int]:
        """
        Get the names of dose rates.

        Returns:
            Dictionary mapping dose rate names to their indices in the output arrays
        """
        if self._dose_names is None:
            self._dose_names = _backend._tuvx._get_dose_rates_ordering(
                self._tuvx_instance)
        return self._dose_names

    def run(self, sza: float, earth_sun_distance: float) -> xr.Dataset:
        """
        Run the TUV-x photolysis calculator.

        All parameters (solar zenith angle, Earth-Sun distance, atmospheric profiles,
        etc.) are read from the JSON configuration file.

        Args:
            sza: Solar zenith angle in radians
            earth_sun_distance: Earth-Sun distance in astronomical units (AU)

        Returns:
            XArray Dataset with data variables:
            - photolysis_rate_constants: Shape (n_reactions, n_vertical_edge) [s^-1]
            - heating_rates: Shape (n_heating_rates, n_vertical_edge) [K s^-1]
            - dose_rates: Shape (n_dose_rates, n_vertical_edge) [W m^-2]
            - actinic_flux: Shape (n_wavelengths, n_vertical_edge, 3 components: direct, upwelling, downwelling) [photons cm^-2 s^-1 nm^-1]
            - spectral_irradiance: Shape (n_wavelengths, n_vertical_edge, 3 components: direct, upwelling, downwelling) [W m^-2 nm^-1]
        """
        photolysis_rates, heating_rates, dose_rates, actinic_flux, \
            spectral_irradiance = _backend._tuvx._run_tuvx(
                self._tuvx_instance, sza, earth_sun_distance)

        # Create arrays of names sorted by index
        reaction_names = [name for name, _ in sorted(
            self.photolysis_rate_names.items(), key=lambda item: item[1]
        )]
        heating_names = [name for name, _ in sorted(
            self.heating_rate_names.items(), key=lambda item: item[1]
        )]
        dose_names = [name for name, _ in sorted(
            self.dose_rate_names.items(), key=lambda item: item[1]
        )]
        radiation_components = ['direct', 'upwelling', 'downwelling']

        # Sanity check on array dimensions
        assert photolysis_rates.shape[0] == len(reaction_names), \
            f"Photolysis rates shape does not match number of reactions {photolysis_rates.shape[0]} /= {len(reaction_names)}"
        assert heating_rates.shape[0] == len(heating_names), \
            f"Heating rates shape does not match number of heating rates {heating_rates.shape[0]} /= {len(heating_names)}"
        assert dose_rates.shape[0] == len(dose_names), \
            f"Dose rates shape does not match number of dose rates {dose_rates.shape[0]} /= {len(dose_names)}"
        assert actinic_flux.shape[2] == len(radiation_components), \
            f"Actinic flux shape does not match number of radiation components {actinic_flux.shape[2]} /= {len(radiation_components)}"
        assert spectral_irradiance.shape[2] == len(radiation_components), \
            f"Spectral irradiance shape does not match number of radiation components {spectral_irradiance.shape[2]} /= {len(radiation_components)}"

        # Get the height and wavelength grids from the GridMap
        grids = self.get_grid_map()
        height_grid = grids["height", "km"]
        wavelength_grid = grids["wavelength", "nm"]

        # Sanity check on height grid dimensions
        assert height_grid.edges.size == photolysis_rates.shape[1], \
            f"Height grid sections do not match number of layers in photolysis rates {height_grid.edges.size} /= {photolysis_rates.shape[1]}"
        assert height_grid.edges.size == heating_rates.shape[1], \
            f"Height grid sections do not match number of layers in heating rates {height_grid.edges.size} /= {heating_rates.shape[1]}"
        assert height_grid.edges.size == dose_rates.shape[1], \
            f"Height grid sections do not match number of layers in dose rates {height_grid.edges.size} /= {dose_rates.shape[1]}"
        assert height_grid.edges.size == actinic_flux.shape[1], \
            f"Height grid sections do not match number of layers in actinic flux {height_grid.edges.size} /= {actinic_flux.shape[1]}"
        assert height_grid.edges.size == spectral_irradiance.shape[1], \
            f"Height grid sections do not match number of layers in spectral irradiance {height_grid.edges.size} /= {spectral_irradiance.shape[1]}"
        # Sanity check on wavelength grid dimensions
        assert wavelength_grid.midpoints.size == actinic_flux.shape[0], \
            f"Wavelength grid sections do not match number of wavelengths in actinic flux {wavelength_grid.midpoints.size} /= {actinic_flux.shape[0]}"
        assert wavelength_grid.midpoints.size == spectral_irradiance.shape[0], \
            f"Wavelength grid sections do not match number of wavelengths in spectral irradiance {wavelength_grid.midpoints.size} /= {spectral_irradiance.shape[0]}"
        dataset_vars = {
            'photolysis_rate_constants': (('reaction', 'vertical_edge'), photolysis_rates, {'units': 's^-1'}),
            'heating_rates': (('heating_rate', 'vertical_edge'), heating_rates, {'units': 'K s^-1'}),
            'dose_rates': (('dose_rate', 'vertical_edge'), dose_rates, {'units': 'W m^-2'}),
            'actinic_flux': (('wavelength_midpoint', 'vertical_edge', 'component'), actinic_flux, {'units': 'photons cm^-2 s^-1 nm^-1'}),
            'spectral_irradiance': (('wavelength_midpoint', 'vertical_edge', 'component'), spectral_irradiance, {'units': 'W m^-2 nm^-1'}),
            "solar_zenith_angle": ((), sza, {'units': 'radians'}),
            "earth_sun_distance": ((), earth_sun_distance, {'units': 'AU'}),
        }

        return xr.Dataset(
            data_vars=dataset_vars,
            coords={
                'reaction': reaction_names,
                'heating_rate': heating_names,
                'dose_rate': dose_names,
                'vertical_midpoint': (('vertical_midpoint',), height_grid.midpoints, {'units': 'km'}),
                'vertical_edge': (('vertical_edge',), height_grid.edges, {'units': 'km'}),
                'wavelength_midpoint': (('wavelength_midpoint',), wavelength_grid.midpoints, {'units': 'nm'}),
                'wavelength_edge': (('wavelength_edge',), wavelength_grid.edges, {'units': 'nm'}),
                'component': (('component',), radiation_components),
            }
        )

    def get_grid_map(self) -> GridMap:
        """
        Get the GridMap used in this TUV-x instance.

        Returns:
            GridMap instance
        """
        return _backend._tuvx._get_grid_map(self._tuvx_instance)

    def get_profile_map(self) -> ProfileMap:
        """
        Get the ProfileMap used in this TUV-x instance.

        Returns:
            ProfileMap instance
        """
        return _backend._tuvx._get_profile_map(self._tuvx_instance)

    def get_radiator_map(self) -> RadiatorMap:
        """
        Get the RadiatorMap used in this TUV-x instance.

        Returns:
            RadiatorMap instance
        """
        return _backend._tuvx._get_radiator_map(self._tuvx_instance)

    def get_photolysis_rate_constant(
        self,
        reaction_name: str,
        photolysis_rates: np.ndarray
    ) -> np.ndarray:
        """
        Extract photolysis rate constants for a specific reaction.

        Args:
            reaction_name: Name of the photolysis reaction
            photolysis_rates: Output from run() method

        Returns:
            1D array of photolysis rate constants for all layers [s^-1]

        Raises:
            KeyError: If reaction_name is not found
        """
        names = self.photolysis_rate_names
        if reaction_name not in names:
            raise KeyError(
                f"Reaction '{reaction_name}' not found. "
                f"Available reactions: {names}"
            )

        reaction_index = names[reaction_name]
        return photolysis_rates[:, reaction_index]

    def get_heating_rate(
        self,
        rate_name: str,
        heating_rates: np.ndarray
    ) -> np.ndarray:
        """
        Extract heating rates for a specific rate type.

        Args:
            rate_name: Name of the heating rate
            heating_rates: Output from run() method

        Returns:
            1D array of heating rates for all layers [K s^-1]

        Raises:
            KeyError: If rate_name is not found
        """
        names = self.heating_rate_names
        if rate_name not in names:
            raise KeyError(
                f"Heating rate '{rate_name}' not found. "
                f"Available rates: {names}"
            )

        rate_index = names[rate_name]
        return heating_rates[:, rate_index]

    def get_dose_rate(
        self,
        rate_name: str,
        dose_rates: np.ndarray
    ) -> np.ndarray:
        """
        Extract dose rates for a specific rate type.

        Args:
            rate_name: Name of the dose rate
            dose_rates: Output from run() method

        Returns:
            1D array of dose rates for all layers [W m^-2]

        Raises:
            KeyError: If rate_name is not found
        """
        names = self.dose_rate_names
        if rate_name not in names:
            raise KeyError(
                f"Dose rate '{rate_name}' not found. "
                f"Available rates: {names}"
            )

        rate_index = names[rate_name]
        return dose_rates[:, rate_index]

    @staticmethod
    def create_config_from_dict(config_dict: Dict) -> 'TUVX':
        """
        Create a TUVX instance from a configuration dictionary.

        Args:
            config_dict: Configuration dictionary

        Returns:
            TUVX instance initialized with the configuration

        Raises:
            ValueError: If TUV-x backend is not available
            FileNotFoundError: If required data files are not found
        """
        with tempfile.NamedTemporaryFile(
                mode='w', suffix='.json', delete=True) as temp_file:
            json.dump(config_dict, temp_file, indent=2)
            temp_file.flush()  # Ensure all data is written to disk
            return TUVX(temp_file.name)

    @staticmethod
    def create_config_from_json_string(json_string: str) -> 'TUVX':
        """
        Create a TUVX instance from a JSON configuration string.

        Args:
            json_string: JSON configuration as string

        Returns:
            TUVX instance initialized with the configuration

        Raises:
            json.JSONDecodeError: If json_string is not valid JSON
            ValueError: If TUV-x backend is not available
            FileNotFoundError: If required data files are not found
        """
        config_dict = json.loads(json_string)
        return TUVX.create_config_from_dict(config_dict)
