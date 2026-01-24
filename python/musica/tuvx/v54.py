"""
TUV-x v5.4 configuration

This module contains configuration settings for the v5.4 configuration of TUV-x
"""

import os
import numpy as np
from .profile import Profile
from .grid import Grid

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
    "O3": "configs/tuvx/data/profiles/atmosphere/o3_v54.dat"
}

def profile(name: str, grid: Grid) -> Profile:
    """
    Returns a standard profile for TUV-x v5.4 by name.
    
    Reads profile data from .dat files and performs linear interpolation
    if the grid values don't match the provided grid.
    
    Args:
        name: Name of the profile (e.g., "O3")
        grid: Grid instance to interpolate the profile onto
    
    Returns:
        Profile instance with data interpolated to the provided grid
    """
    if name not in profile_data_files:
        raise ValueError(f"Profile '{name}' not found in TUV-x v5.4 configuration.")
    
    # Resolve filepath relative to the musica package root
    filepath = profile_data_files[name]
    if not os.path.isabs(filepath):
        # Get the package directory (go up from python/musica/tuvx to the root)
        package_dir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
        filepath = os.path.join(package_dir, filepath)
    
    # Read the data file
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Parse midpoint and edge sections
    midpoint_section = []
    edge_section = []
    current_section = None
    
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'):
            # Check for section headers
            if 'height (km), mid-point' in line:
                current_section = 'midpoint'
            elif 'height (km), edge' in line:
                current_section = 'edge'
            continue
        
        # Parse data lines - only extract first two columns
        parts = line.split(',')
        if current_section == 'midpoint':
            # Format: height (km), mid-point, delta, layer density, ...
            # Extract: height, profile value, layer density
            values = [float(parts[0]), float(parts[1]), float(parts[3])]
            midpoint_section.append(values)
        elif current_section == 'edge':
            # Format: height (km), edge
            values = [float(parts[0]), float(parts[1])]
            edge_section.append(values)
    
    # Convert to numpy arrays
    midpoint_data = np.array(midpoint_section)
    edge_data = np.array(edge_section)
    
    # Extract grid coordinates, profile values, and layer densities
    # midpoint_data columns: 0=height (km), 1=profile value, 2=layer density
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
    
    # Create and return the Profile
    return Profile(
        name=name,
        units="molecule cm-3",  # Standard units for atmospheric species
        grid=grid,
        edge_values=interpolated_edges,
        midpoint_values=interpolated_midpoints,
        layer_densities=interpolated_layer_densities
    )
