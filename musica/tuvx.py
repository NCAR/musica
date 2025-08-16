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
from typing import Dict, Tuple, List
import numpy as np
from . import backend

_backend = backend.get_backend()

version = _backend._tuvx._get_tuvx_version() if backend.tuvx_available() else None


class TUVX:
    """
    A Python interface to the TUV-x photolysis calculator.

    This class provides a simplified interface that only requires a JSON configuration
    file to set up and run TUV-x calculations. All parameters (solar zenith angle,
    earth-sun distance, atmospheric profiles, etc.) are specified in the JSON config.
    """

    def __init__(self, config_path: str):
        """
        Initialize a TUV-x instance from a configuration file.

        Args:
            config_path: Path to the JSON configuration file

        Raises:
            FileNotFoundError: If the configuration file doesn't exist
            ValueError: If TUV-x initialization fails or TUVX is not available
        """
        if not backend.tuvx_available():
            raise ValueError("TUV-x backend is not available on windows.")

        if not os.path.exists(config_path):
            raise FileNotFoundError(
                f"Configuration file not found: {config_path}")

        self._tuvx_instance = _backend._tuvx._create_tuvx(config_path)
        self._config_path = config_path

        # Cache the names for efficiency
        self._photolysis_names = None
        self._heating_names = None

    def __del__(self):
        """Clean up the TUV-x instance."""
        if hasattr(self, '_tuvx_instance') and self._tuvx_instance is not None:
            _backend._tuvx._delete_tuvx(self._tuvx_instance)

    @property
    def photolysis_rate_names(self) -> List[str]:
        """
        Get the names of photolysis rates.

        Returns:
            List of photolysis rate names
        """
        if self._photolysis_names is None:
            self._photolysis_names = _backend._tuvx._get_photolysis_rate_names(
                self._tuvx_instance)
        return self._photolysis_names

    @property
    def heating_rate_names(self) -> List[str]:
        """
        Get the names of heating rates.

        Returns:
            List of heating rate names
        """
        if self._heating_names is None:
            self._heating_names = _backend._tuvx._get_heating_rate_names(
                self._tuvx_instance)
        return self._heating_names

    def run(self) -> Tuple[np.ndarray, np.ndarray]:
        """
        Run the TUV-x photolysis calculator.

        All parameters (solar zenith angle, Earth-Sun distance, atmospheric profiles,
        etc.) are read from the JSON configuration file.

        Returns:
            Tuple of (photolysis_rate_constants, heating_rates) as numpy arrays
            - photolysis_rate_constants: Shape (n_layers, n_reactions) [s^-1]
            - heating_rates: Shape (n_layers, n_heating_rates) [K s^-1]
        """
        photolysis_rates, heating_rates = _backend._tuvx._run_tuvx(
            self._tuvx_instance)

        return photolysis_rates, heating_rates

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

        reaction_index = names.index(reaction_name)
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

        rate_index = names.index(rate_name)
        return heating_rates[:, rate_index]

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
