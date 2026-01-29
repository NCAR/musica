"""
TUV-x v5.4 configuration

This module contains configuration settings for the v5.4 configuration of TUV-x
"""

import os
import numpy as np
from .grid import Grid
from .profile import Profile
from .radiator import Radiator
from musica.tuvx import TUVX

def get_tuvx_calculator() -> TUVX:
    """Returns a TUV-x instance configured for the v5.4 photolysis setup."""
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
    """Returns the file path to the TUV-x v5.4 configuration JSON file."""
    # Get the package directory (musica package root from musica/tuvx/v54.py)
    package_dir = os.path.dirname(os.path.dirname(__file__))
    config_path = os.path.join(package_dir, "configs", "tuvx", "tuv_5_4.json")
    return config_path


def height_grid() -> Grid:
    """Returns the v5.4 height grid for TUV-x."""
    heights = Grid(name="height", units="km", num_sections=120)
    heights.edges = np.linspace(0, 120, 121)
    heights.midpoints = 0.5 * (heights.edges[:-1] + heights.edges[1:])
    return heights


def wavelength_grid() -> Grid:
    """Returns the v5.4 wavelength grid for TUV-x."""
    wavelengths = Grid(name="wavelength", units="nm", num_sections=156)
    wavelengths.edges = np.array(
        [
            120.0000,
            121.4000,
            121.9000,
            122.3000,
            123.1000,
            123.8000,
            124.6000,
            125.4000,
            126.2000,
            127.0000,
            128.6000,
            129.4000,
            130.3000,
            132.0000,
            135.0000,
            137.0000,
            145.0000,
            155.0000,
            165.0000,
            170.0000,
            175.4000,
            177.0000,
            178.6000,
            180.2000,
            181.8000,
            183.5000,
            185.2000,
            186.9000,
            188.7000,
            190.5000,
            192.3000,
            194.2000,
            196.1000,
            198.0000,
            200.0000,
            202.0000,
            204.1000,
            206.2000,
            208.3330,
            210.5260,
            212.7660,
            215.0540,
            217.3910,
            219.7800,
            222.2220,
            224.7190,
            227.2730,
            229.8850,
            232.5580,
            235.2940,
            238.0950,
            240.9640,
            243.9020,
            246.9140,
            250.0000,
            253.1650,
            256.4100,
            259.7400,
            263.1580,
            266.6670,
            270.2700,
            273.9730,
            277.7780,
            281.6900,
            285.7140,
            289.8550,
            294.1180,
            298.5000,
            302.5000,
            303.5000,
            304.5000,
            305.5000,
            306.5000,
            307.5000,
            308.5000,
            309.5000,
            310.5000,
            311.5000,
            312.5000,
            313.5000,
            314.5000,
            317.5000,
            322.5000,
            327.5000,
            332.5000,
            337.5000,
            342.5000,
            347.5000,
            352.5000,
            357.5000,
            362.5000,
            367.5000,
            372.5000,
            377.5000,
            382.5000,
            387.5000,
            392.5000,
            397.5000,
            402.5000,
            407.5000,
            412.5000,
            417.5000,
            422.5000,
            427.5000,
            432.5000,
            437.5000,
            442.5000,
            447.5000,
            452.5000,
            457.5000,
            462.5000,
            467.5000,
            472.5000,
            477.5000,
            482.5000,
            487.5000,
            492.5000,
            497.5000,
            502.5000,
            507.5000,
            512.5000,
            517.5000,
            522.5000,
            527.5000,
            532.5000,
            537.5000,
            542.5000,
            547.5000,
            552.5000,
            557.5000,
            562.5000,
            567.5000,
            572.5000,
            577.5000,
            582.5000,
            587.5000,
            592.5000,
            597.5000,
            602.5000,
            607.5000,
            612.5000,
            617.5000,
            622.5000,
            627.5000,
            632.5000,
            637.5000,
            642.5000,
            647.1000,
            655.0000,
            665.0000,
            675.0000,
            685.0000,
            695.0000,
            705.0000,
            715.0000,
            725.0000,
            735.0000,
        ]
    )
    wavelengths.midpoints = 0.5 * (wavelengths.edges[:-1] + wavelengths.edges[1:])
    return wavelengths


profile_data_files = {
    "O2": "configs/tuvx/data/profiles/atmosphere/o2.v54.dat",
    "O3": "configs/tuvx/data/profiles/atmosphere/o3.v54.dat",
    "air": "configs/tuvx/data/profiles/atmosphere/air.v54.dat",
    "temperature": "configs/tuvx/data/profiles/atmosphere/temperature.v54.dat",
    "surface albedo": "configs/tuvx/data/profiles/solar/surface_albedo.v54.dat",
    "extraterrestrial flux": "configs/tuvx/data/profiles/solar/extraterrestrial_flux.v54.dat"
}

def profile(name: str, grid: Grid) -> Profile:
    """
    Returns a standard profile for TUV-x v5.4 by name.
    
    Reads profile data from .dat files and performs linear interpolation
    if the grid values don't match the provided grid.
    
    Args:
        name: Name of the profile (e.g., "O3", "air", "temperature")
        grid: Grid instance to interpolate the profile onto
    Returns:
        Profile instance with data interpolated to the provided grid
    """
    return profile_from_map(profile_data_files, name, grid)


def profile_from_map(file_map: dict, name: str, grid: Grid) -> Profile:
    """
    Returns a standard profile for TUV-x v5.4 by name.
    
    Reads profile data from .dat files and performs linear interpolation
    if the grid values don't match the provided grid.
    
    Args:
        file_map: Dictionary mapping profile names to data file paths
        name: Name of the profile (e.g., "O3", "air", "temperature")
        grid: Grid instance to interpolate the profile onto
    
    Returns:
        Profile instance with data interpolated to the provided grid
    """
    if name not in file_map:
        raise ValueError(f"Profile '{name}' not found in TUV-x v5.4 configuration.")
    
    # Resolve filepath relative to the musica package root
    filepath = file_map[name]
    if not os.path.isabs(filepath):
        # Get the package directory (musica package root from musica/tuvx/v54.py)
        package_dir = os.path.dirname(os.path.dirname(__file__))
        filepath = os.path.join(package_dir, filepath)
    
    # Read the data file
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Extract units from header if not provided
    units: str | None = None
    for line in lines:
        if line.startswith(' # Profile:') and '(' in line and ')' in line:
            # Extract units from header like: # Profile: O3 (molecule cm-3)
            start = line.find('(') + 1
            end = line.find(')', start)
            units = line[start:end]
            break
    if units is None:
        units = "unknown"
    
    # Parse midpoint and edge sections
    midpoint_section = []
    edge_section = []
    current_section = None
    
    # Detect grid type from header
    grid_type = None
    for line in lines:
        if '# height (km), mid-point' in line or '# height (km), edge' in line:
            grid_type = 'height'
            break
        elif '# wavelength (nm), mid-point' in line or '# wavelength (nm), edge' in line:
            grid_type = 'wavelength'
            break
    
    if grid_type is None:
        raise ValueError(f"Could not determine grid type from file headers")
    

    exo_layer_density = 0.0  # Default value if not present
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'):
            # Check for section headers (works for both height and wavelength)
            if 'mid-point' in line and ('height (km)' in line or 'wavelength (nm)' in line):
                current_section = 'midpoint'
            elif 'edge' in line and ('height (km)' in line or 'wavelength (nm)' in line):
                current_section = 'edge'
            continue
        
        # Parse data lines - only extract first two columns
        parts = line.split(',')
        # Skip lines where all values are '---' (completely empty rows)
        if all(p.strip() == '---' for p in parts if p.strip()):
            continue
        if current_section == 'midpoint':
            # Format: height (km), mid-point, delta, layer density, ...
            # Extract: height, profile value, layer density
            # Skip '---' markers - they indicate unallocated/out-of-bounds array elements
            try:
                height = float(parts[0]) if parts[0].strip() != '---' else None
                midpoint = float(parts[1]) if parts[1].strip() != '---' else None
                layer_density = float(parts[3]) if len(parts) > 3 and parts[3].strip() != '---' else None
                values = [height, midpoint, layer_density if layer_density is not None else 0.0]
                # Only add rows with height and midpoint values
                if height is not None and midpoint is not None:
                    midpoint_section.append(values)
                else:
                    exo_layer_density = float(parts[4]) if len(parts) > 4 and parts[4].strip() != '---' else 0.0
            except (ValueError, IndexError):
                raise ValueError(f"Error parsing midpoint data line: {line}")
        elif current_section == 'edge':
            # Format: height (km), edge
            try:
                height = float(parts[0]) if parts[0].strip() != '---' else None
                edge = float(parts[1]) if parts[1].strip() != '---' else None
                # Only add row if we have valid numerical data
                if height is not None and edge is not None:
                    values = [height, edge]
                    edge_section.append(values)
            except (ValueError, IndexError):
                raise ValueError(f"Error parsing edge data line: {line}")
    
    # Convert to numpy arrays
    midpoint_data = np.array(midpoint_section)
    edge_data = np.array(edge_section)
    
    # Extract grid coordinates, profile values, and layer densities
    # midpoint_data columns: 0=height (km), 1=profile value, 2=layer density, 3=exo layer density
    file_grid_midpoints = midpoint_data[:, 0]
    file_midpoint_values = midpoint_data[:, 1]
    file_layer_densities = midpoint_data[:, 2]
    
    # edge_data columns: 0=height (km), 1=profile value
    file_grid_edges = edge_data[:, 0]
    file_edge_values = edge_data[:, 1]
    
    # Determine if we need to interpolate based on whether grids match
    grids_match = (len(grid.midpoints) == len(file_grid_midpoints) and
                   np.allclose(grid.midpoints, file_grid_midpoints) and
                   len(grid.edges) == len(file_grid_edges) and
                   np.allclose(grid.edges, file_grid_edges))
    
    if not grids_match:
        # Interpolate profile values to the provided grid coordinates
        interpolated_midpoints = np.interp(
            grid.midpoints,
            file_grid_midpoints,
            file_midpoint_values
        )
        interpolated_edges = np.interp(
            grid.edges,
            file_grid_edges,
            file_edge_values
        )
        interpolated_layer_densities = np.interp(
            grid.midpoints,
            file_grid_midpoints,
            file_layer_densities
        )
    else:
        # Grids match, use values directly
        interpolated_midpoints = file_midpoint_values
        interpolated_edges = file_edge_values
        interpolated_layer_densities = file_layer_densities

    # Remove the exo layer density from the top layer density if present
    # because it will be added back in by the Profile constructor
    if exo_layer_density > 0.0:
        interpolated_layer_densities[-1] -= exo_layer_density
    
    # Create and return the Profile
    return Profile(
        name=name,
        units=units,
        grid=grid,
        edge_values=interpolated_edges,
        midpoint_values=interpolated_midpoints,
        layer_densities=interpolated_layer_densities,
        exo_layer_density=exo_layer_density
    )

radiator_data_files = {
    "aerosol": "configs/tuvx/data/radiators/aerosol.v54.dat"
}

def radiator(radiator_name: str, heights: Grid, wavelengths: Grid) -> Radiator:
    """
    Returns a standard radiator for TUV-x v5.4 by name.
    
    Args:
        radiator_name: Name of the radiator (e.g., "aerosol")
        heights: Height grid for the radiator
        wavelengths: Wavelength grid for the radiator
    Returns:
        Radiator instance with data loaded from the corresponding v5.4 data file
    """
    return radiator_from_map(radiator_data_files, radiator_name, heights, wavelengths)


def radiator_from_map(file_map: dict, radiator_name: str, heights: Grid, wavelengths: Grid) -> Radiator:
    """
    Returns a standard radiator for TUV-x v5.4 by name.
    
    Args:
        file_map: Dictionary mapping radiator names to data file paths
        radiator_name: Name of the radiator (e.g., "aerosol")
        heights: Height grid for the radiator
        wavelengths: Wavelength grid for the radiator
    
    Returns:
        Radiator instance with data loaded from the corresponding v5.4 data file
    """
    if radiator_name not in file_map:
        raise ValueError(f"Radiator '{radiator_name}' not found in v5.4 configuration")
    
    filepath = file_map[radiator_name]
    
    # Get the absolute path to the data file (musica package root from musica/tuvx/v54.py)
    package_dir = os.path.dirname(os.path.dirname(__file__))
    full_path = os.path.join(package_dir, filepath)
    
    # Parse the file
    with open(full_path, 'r') as f:
        lines = f.readlines()
    
    # Skip header lines and parse data
    # Format: Height (km), Wavelength (nm), Layer OD, Layer SSA, Layer G
    data = []
    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith('#'):
            continue
        parts = stripped.split()
        if len(parts) >= 5:
            height = float(parts[0])
            wavelength = float(parts[1])
            optical_depth = float(parts[2])
            ssa = float(parts[3])
            g = float(parts[4])
            data.append([height, wavelength, optical_depth, ssa, g])
    
    data = np.array(data)
    
    # Organize data into 2D arrays (wavelengths x heights)
    # Note: Radiator expects shape (num_wavelengths, num_heights)
    num_heights = heights.num_sections
    num_wavelengths = wavelengths.num_sections
    
    optical_depths = np.zeros((num_wavelengths, num_heights))
    single_scattering_albedos = np.zeros((num_wavelengths, num_heights))
    asymmetry_factors = np.zeros((num_wavelengths, num_heights))
    
    # Fill arrays by matching heights and wavelengths
    for row in data:
        file_height, file_wavelength, od, ssa, g = row
        
        # Find closest height index
        height_idx = np.argmin(np.abs(heights.midpoints - file_height))
        
        # Find closest wavelength index
        wavelength_idx = np.argmin(np.abs(wavelengths.midpoints - file_wavelength))
        
        optical_depths[wavelength_idx, height_idx] = od
        single_scattering_albedos[wavelength_idx, height_idx] = ssa
        asymmetry_factors[wavelength_idx, height_idx] = g
    
    return Radiator(
        name=radiator_name,
        height_grid=heights,
        wavelength_grid=wavelengths,
        optical_depths=optical_depths,
        single_scattering_albedos=single_scattering_albedos,
        asymmetry_factors=asymmetry_factors
    )
