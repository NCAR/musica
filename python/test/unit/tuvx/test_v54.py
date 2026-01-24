"""
Tests for TUV-x v5.4 configuration module.

This module tests the grid creation and profile loading functionality
for the v5.4 configuration of TUV-x.
"""

import os
import pytest
import numpy as np
from musica.tuvx import v54
from musica.tuvx.grid import backend

# Skip all tests if TUV-x is not available
pytestmark = pytest.mark.skipif(not backend.tuvx_available(),
                                reason="TUV-x backend is not available")


class TestV54Grids:
    """Tests for v5.4 grid creation functions."""
    
    def test_height_grid_dimensions(self):
        """Test that height grid has correct dimensions."""
        grid = v54.height_grid()
        
        assert grid.name == "height"
        assert grid.units == "km"
        assert grid.num_sections == 120
        assert len(grid.edges) == 121
        assert len(grid.midpoints) == 120
    
    def test_height_grid_values(self):
        """Test that height grid has correct values."""
        grid = v54.height_grid()
        
        # Check edges range from 0 to 120 km
        assert grid.edges[0] == 0.0
        assert grid.edges[-1] == 120.0
        
        # Check that edges are uniformly spaced
        expected_edges = np.linspace(0, 120, 121)
        np.testing.assert_array_almost_equal(grid.edges, expected_edges)
        
        # Check that midpoints are calculated correctly
        expected_midpoints = 0.5 * (expected_edges[:-1] + expected_edges[1:])
        np.testing.assert_array_almost_equal(grid.midpoints, expected_midpoints)
    
    def test_wavelength_grid_dimensions(self):
        """Test that wavelength grid has correct dimensions."""
        grid = v54.wavelength_grid()
        
        assert grid.name == "wavelength"
        assert grid.units == "nm"
        assert grid.num_sections == 156
        assert len(grid.edges) == 157
        assert len(grid.midpoints) == 156
    
    def test_wavelength_grid_values(self):
        """Test that wavelength grid has correct boundary values."""
        grid = v54.wavelength_grid()
        
        # Check boundary wavelengths
        assert grid.edges[0] == 120.0
        assert grid.edges[-1] == 735.0
        
        # Check some specific edge values
        assert grid.edges[1] == 121.4
        assert grid.edges[2] == 121.9
        
        # Check that midpoints are calculated correctly
        expected_midpoints = 0.5 * (grid.edges[:-1] + grid.edges[1:])
        np.testing.assert_array_almost_equal(grid.midpoints, expected_midpoints)


class TestV54Profile:
    """Tests for v5.4 profile loading and interpolation."""
    
    def test_profile_load_o3(self):
        """Test loading O3 profile."""
        grid = v54.height_grid()
        prof = v54.profile("O3", grid)
        
        assert prof.name == "O3"
        assert prof.units == "molecule cm-3"
        assert prof.number_of_sections == 120
    
    def test_profile_invalid_name(self):
        """Test that invalid profile name raises error."""
        grid = v54.height_grid()
        
        with pytest.raises(ValueError, match="Profile 'INVALID' not found"):
            v54.profile("INVALID", grid)
    
    def test_profile_exact_reproduction(self):
        """Test that profile can exactly reproduce the data file values."""
        # Load the profile with the standard v5.4 grid
        grid = v54.height_grid()
        prof = v54.profile("O3", grid)
        
        # Read the original data file
        filepath = "configs/tuvx/data/profiles/atmosphere/o3_v54.dat"
        package_dir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(__file__)))))
        full_path = os.path.join(package_dir, filepath)
        
        with open(full_path, 'r') as f:
            file_lines = f.readlines()
        
        # Extract the data lines from the file
        file_midpoint_lines = []
        file_edge_lines = []
        current_section = None
        
        for line in file_lines:
            stripped = line.strip()
            if not stripped or stripped.startswith('#'):
                if 'height (km), mid-point' in stripped:
                    current_section = 'midpoint'
                elif 'height (km), edge' in stripped:
                    current_section = 'edge'
                continue
            
            if current_section == 'midpoint':
                file_midpoint_lines.append(stripped)
            elif current_section == 'edge':
                file_edge_lines.append(stripped)
        
        # Convert profile values back to text format and compare
        # Midpoint section format: height (km), mid-point, delta, layer density, exo layer density, burden density
        # Edge section format: height (km), edge
        # We'll compare all columns that aren't marked as '---'
        
        # Helper function to format numbers matching the file format
        def format_scientific(value):
            """Format a number in scientific notation matching the .dat file format."""
            # Format with 16 decimal places in scientific notation
            formatted = f"{value:.16E}"
            # Python uses E+01 or E-01, but file uses E+001 or E-001
            # Need to pad the exponent to 3 digits
            if 'E' in formatted:
                parts = formatted.split('E')
                mantissa = parts[0]
                exponent = parts[1]
                # Pad exponent to 3 digits (including sign)
                if exponent.startswith('+') or exponent.startswith('-'):
                    sign = exponent[0]
                    exp_num = exponent[1:]
                    exponent_padded = f"{sign}{exp_num.zfill(3)}"
                else:
                    exponent_padded = exponent.zfill(3)
                formatted = f"{mantissa}E{exponent_padded}"
            return formatted
        
        # Check midpoint lines
        assert len(prof.midpoint_values) == len(file_midpoint_lines), \
            f"Number of midpoints mismatch: {len(prof.midpoint_values)} vs {len(file_midpoint_lines)}"
        
        for i, (grid_val, prof_val, layer_density) in enumerate(zip(grid.midpoints, prof.midpoint_values, prof.layer_densities)):
            file_line_parts = file_midpoint_lines[i].split(',')
            
            # Column 0: height (km) at midpoint
            file_grid = file_line_parts[0].strip()
            grid_formatted = format_scientific(grid_val)
            assert grid_formatted == file_grid, \
                f"Grid midpoint mismatch at index {i}:\n  Expected: {file_grid}\n  Got: {grid_formatted}"
            
            # Column 1: profile value at midpoint
            file_prof = file_line_parts[1].strip()
            prof_formatted = format_scientific(prof_val)
            assert prof_formatted == file_prof, \
                f"Profile midpoint value mismatch at index {i}:\n  Expected: {file_prof}\n  Got: {prof_formatted}"
            
            # Column 2: delta (difference) - we can skip this as it's derived
            
            # Column 3: layer density
            file_layer_density = file_line_parts[3].strip()
            # Check if it's a valid number (not '---')
            if file_layer_density != '---':
                layer_density_formatted = f"{layer_density:.16E}"
                # The file uses regular decimal notation for layer densities, not scientific
                # Let's check what format is actually used
                if 'E' in file_layer_density or 'e' in file_layer_density:
                    # It's in scientific notation
                    layer_density_formatted = format_scientific(layer_density)
                else:
                    # Regular decimal notation
                    layer_density_formatted = f"{layer_density:.12f}"
                
                # For now, just verify they represent the same value
                file_layer_density_val = float(file_layer_density)
                assert abs(layer_density - file_layer_density_val) / max(abs(file_layer_density_val), 1e-10) < 1e-6, \
                    f"Layer density mismatch at index {i}:\n  Expected: {file_layer_density}\n  Got: {layer_density}"
            
            # Columns 4 and 5 are marked as '---' so we skip them
        
        # Check edge lines
        assert len(prof.edge_values) == len(file_edge_lines), \
            f"Number of edges mismatch: {len(prof.edge_values)} vs {len(file_edge_lines)}"
        
        for i, (grid_val, prof_val) in enumerate(zip(grid.edges, prof.edge_values)):
            file_line_parts = file_edge_lines[i].split(',')
            
            # Column 0: height (km) at edge
            file_grid = file_line_parts[0].strip()
            grid_formatted = format_scientific(grid_val)
            assert grid_formatted == file_grid, \
                f"Grid edge mismatch at index {i}:\n  Expected: {file_grid}\n  Got: {grid_formatted}"
            
            # Column 1: profile value at edge
            file_prof = file_line_parts[1].strip()
            prof_formatted = format_scientific(prof_val)
            assert prof_formatted == file_prof, \
                f"Profile edge value mismatch at index {i}:\n  Expected: {file_prof}\n  Got: {prof_formatted}"
    
    def test_profile_interpolation(self):
        """Test that profile interpolation works with different grid."""
        # Create a coarser grid (60 sections instead of 120)
        from musica.tuvx.grid import Grid
        
        coarse_grid = Grid(name="height", units="km", num_sections=60)
        coarse_grid.edges = np.linspace(0, 120, 61)
        coarse_grid.midpoints = 0.5 * (coarse_grid.edges[:-1] + coarse_grid.edges[1:])
        
        # Load profile with coarse grid
        prof = v54.profile("O3", coarse_grid)
        
        # Check dimensions
        assert len(prof.midpoint_values) == 60
        assert len(prof.edge_values) == 61
        
        # Check that values are reasonable (not NaN or zero everywhere)
        assert not np.any(np.isnan(prof.midpoint_values))
        assert not np.any(np.isnan(prof.edge_values))
        assert np.any(prof.midpoint_values > 0)
        assert np.any(prof.edge_values > 0)
    
    def test_profile_fine_grid_interpolation(self):
        """Test that profile interpolation works with finer grid."""
        from musica.tuvx.grid import Grid
        
        # Create a finer grid (240 sections instead of 120)
        fine_grid = Grid(name="height", units="km", num_sections=240)
        fine_grid.edges = np.linspace(0, 120, 241)
        fine_grid.midpoints = 0.5 * (fine_grid.edges[:-1] + fine_grid.edges[1:])
        
        # Load profile with fine grid
        prof = v54.profile("O3", fine_grid)
        
        # Check dimensions
        assert len(prof.midpoint_values) == 240
        assert len(prof.edge_values) == 241
        
        # Check that values are reasonable
        assert not np.any(np.isnan(prof.midpoint_values))
        assert not np.any(np.isnan(prof.edge_values))
        assert np.any(prof.midpoint_values > 0)
        assert np.any(prof.edge_values > 0)
        
        # Interpolated values should be monotonically decreasing in general
        # (O3 typically decreases with height in stratosphere)
        # Check that the profile has expected characteristics
        assert prof.midpoint_values[0] > prof.midpoint_values[-1]
