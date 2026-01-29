"""
TUV-x vTS1/TSMLT configuration.

This module contains configuration settings for the TS1/TSMLT configuration of TUV-x.
"""

import os
import numpy as np

from musica.tuvx.tuvx import TUVX
from musica.tuvx.grid import Grid
from musica.tuvx.profile import Profile
from musica.tuvx.radiator import Radiator
from .v54 import height_grid
from .v54 import profile_from_map
from .v54 import radiator_from_map

def get_tuvx_calculator() -> TUVX:
    """Returns a TUV-x instance configured for the vTS1/TSMLT photolysis setup."""
    from .tuvx import TUVX
    from .grid_map import GridMap
    from .profile_map import ProfileMap
    from .radiator_map import RadiatorMap

    # Set up grids
    grids = GridMap()
    grids["height", "km"] = height_grid()
    grids["wavelength", "nm"] = wavelength_grid()

    # Set up profiles
    profiles = ProfileMap()
    profiles["air", "molecule cm-3"] = profile("air", grids["height", "km"])
    profiles["O3", "molecule cm-3"] = profile("O3", grids["height", "km"])
    profiles["O2", "molecule cm-3"] = profile("O2", grids["height", "km"])
    profiles["temperature", "K"] = profile("temperature", grids["height", "km"])
    profiles["surface albedo", "none"] = profile("surface albedo", grids["wavelength", "nm"])
    profiles["extraterrestrial flux", "photon cm-2 s-1"] = profile(
        "extraterrestrial flux", grids["wavelength", "nm"]
    )

    # Set up radiators
    radiators = RadiatorMap()
    radiators["aerosol"] = radiator("aerosol", grids["height", "km"], grids["wavelength", "nm"])

    # Create TUV-x instance with v5.4 configuration file
    tuvx = TUVX(
        grid_map=grids,
        profile_map=profiles,
        radiator_map=radiators,
        config_path=config_file_path()
    )

    return tuvx


def config_file_path() -> str:
    """Returns the file path to the TUV-x vTS1/TSMLT configuration JSON file."""
    # Get the package directory (musica package root from musica/tuvx/vTS1.py)
    package_dir = os.path.dirname(os.path.dirname(__file__))
    config_path = os.path.join(package_dir, "configs", "tuvx", "ts1_tsmlt.json")
    return config_path


def wavelength_grid() -> Grid:
    """Returns the vTS1/TSMLT wavelength grid for TUV-x."""
    wavelengths = Grid(name="wavelength", units="nm", num_sections=102)
    wavelengths.edges = np.array([
        120,
        121.4,
        121.9,
        123.5,
        124.3,
        125.5,
        126.3,
        127.1,
        130.1,
        131.1,
        135,
        140,
        145,
        150,
        155,
        160,
        165,
        168,
        171,
        173,
        174.4,
        175.4,
        177,
        178.6,
        180.2,
        181.8,
        183.5,
        185.2,
        186.9,
        188.7,
        190.5,
        192.3,
        194.2,
        196.1,
        198,
        200,
        202,
        204.1,
        206.2,
        208,
        211,
        214,
        217,
        220,
        223,
        226,
        229,
        232,
        235,
        238,
        241,
        244,
        247,
        250,
        253,
        256,
        259,
        263,
        267,
        271,
        275,
        279,
        283,
        287,
        291,
        295,
        298.5,
        302.5,
        305.5,
        308.5,
        311.5,
        314.5,
        317.5,
        322.5,
        327.5,
        332.5,
        337.5,
        342.5,
        347.5,
        350,
        355,
        360,
        365,
        370,
        375,
        380,
        385,
        390,
        395,
        400,
        405,
        410,
        415,
        420,
        430,
        440,
        450,
        500,
        550,
        600,
        650,
        700,
        750,
    ])
    wavelengths.midpoints = 0.5 * (wavelengths.edges[:-1] + wavelengths.edges[1:])
    return wavelengths

profile_data_files = {
    "O2": "configs/tuvx/data/profiles/atmosphere/o2.v54.dat",
    "O3": "configs/tuvx/data/profiles/atmosphere/o3.v54.dat",
    "air": "configs/tuvx/data/profiles/atmosphere/air.v54.dat",
    "temperature": "configs/tuvx/data/profiles/atmosphere/temperature.v54.dat",
    "surface albedo": "configs/tuvx/data/profiles/solar/surface_albedo.ts1.dat",
    "extraterrestrial flux": "configs/tuvx/data/profiles/solar/extraterrestrial_flux.ts1.dat"
}


def profile(name: str, grid: Grid) -> Profile:
    """Returns a standard profile for TUV-x vTS1/TSMLT by name.
    
    Raises a ValueError if the profile name is not recognized or
    if the profile data file grid does not match the provided grid.
    
    Args:
        name: Name of the profile (e.g., "O3", "air", "temperature")
        grid: Grid instance to interpolate the profile onto
    Returns:
        Profile instance with data interpolated to the provided grid
    """
    return profile_from_map(profile_data_files, name, grid)


radiator_data_files = {
    "aerosol": "configs/tuvx/data/radiators/aerosol.ts1.dat"
}


def radiator(name: str, height_grid: Grid, wavelength_grid: Grid) -> Radiator:
    """Returns a standard radiator for TUV-x vTS1/TSMLT by name.
    
    Raises a ValueError if the radiator name is not recognized or
    if the radiator data file grids do not match the provided grids.
    
    Args:
        name: Name of the radiator (e.g., "aerosol")
        height_grid: Height grid instance
        wavelength_grid: Wavelength grid instance
    Returns:
        Radiator instance with data loaded from the corresponding TS1/TSMLT data file
    """
    return radiator_from_map(radiator_data_files, name, height_grid, wavelength_grid)

