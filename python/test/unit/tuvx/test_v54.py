"""
Tests for TUV-x v5.4 configuration module.

This module tests the grid creation and profile loading functionality
for the v5.4 configuration of TUV-x.
"""

import os
import pytest
import numpy as np
from decimal import Decimal, getcontext
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

    def test_profile_load_all(self):
        """Test loading all available v5.4 profiles."""
        # Test atmospheric profiles with height grid
        height_grid = v54.height_grid()

        o2_prof = v54.profile("O2", height_grid)
        assert o2_prof.name == "O2"
        assert o2_prof.units == "molecule cm-3"

        o3_prof = v54.profile("O3", height_grid)
        assert o3_prof.name == "O3"
        assert o3_prof.units == "molecule cm-3"

        air_prof = v54.profile("air", height_grid)
        assert air_prof.name == "air"
        assert air_prof.units == "molecule cm-3"

        temp_prof = v54.profile("temperature", height_grid)
        assert temp_prof.name == "temperature"
        assert temp_prof.units == "K"

        # Test solar profiles with wavelength grid
        wavelength_grid = v54.wavelength_grid()

        albedo_prof = v54.profile("surface albedo", wavelength_grid)
        assert albedo_prof.name == "surface albedo"
        assert albedo_prof.units == "none"

        flux_prof = v54.profile("extraterrestrial flux", wavelength_grid)
        assert flux_prof.name == "extraterrestrial flux"
        assert flux_prof.units == "photon cm-2 s-1"

    def test_profile_invalid_name(self):
        """Test that invalid profile name raises error."""
        grid = v54.height_grid()

        with pytest.raises(ValueError, match="Profile 'INVALID' not found"):
            v54.profile("INVALID", grid)

    def test_profile_exact_reproduction(self):
        """Test that profile can exactly reproduce the data file values."""
        # Test all available profiles
        profiles_to_test = [
            ("O2", v54.height_grid()),
            ("O3", v54.height_grid()),
            ("air", v54.height_grid()),
            ("temperature", v54.height_grid()),
            ("surface albedo", v54.wavelength_grid()),
            ("extraterrestrial flux", v54.wavelength_grid())
        ]

        for profile_name, grid in profiles_to_test:
            self._test_single_profile_reproduction(profile_name, grid)

    def _test_single_profile_reproduction(self, profile_name: str, grid):
        """Helper method to test exact reproduction of a single profile."""
        # Load the profile
        prof = v54.profile(profile_name, grid)

        # Get the file path
        from musica.tuvx.v54 import profile_data_files
        filepath = profile_data_files[profile_name]
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
                if 'mid-point' in stripped:
                    current_section = 'midpoint'
                elif 'edge' in stripped:
                    current_section = 'edge'
                continue

            # Skip footer lines that start with '---'
            parts = stripped.split(',')
            if parts[0].strip() == '---':
                continue

            if current_section == 'midpoint':
                file_midpoint_lines.append(stripped)
            elif current_section == 'edge':
                file_edge_lines.append(stripped)

        # Set decimal precision high enough for 20 digits
        getcontext().prec = 50

        # Helper function to format numbers matching the file format
        def format_scientific(value):
            """Format a number in scientific notation matching the .dat file format."""
            # Convert to Decimal for high precision
            d = Decimal(str(value))

            # Get the scientific notation components
            # Format: d.ddddddddddddddddE±nnn (17 significant figures: 1 before decimal + 16 after)
            sign, digits, exponent = d.as_tuple()

            # Convert digits tuple to string
            digits_str = ''.join(str(d) for d in digits)

            # Pad or truncate to exactly 17 digits
            if len(digits) < 17:
                digits_str = digits_str + '0' * (17 - len(digits))
            else:
                digits_str = digits_str[:17]

            # Calculate the exponent for scientific notation
            # In Decimal.as_tuple(), exponent is the power of 10 to multiply by
            # For scientific notation d.ddd...E+nnn, we need exponent relative to first digit
            actual_exp = exponent + len(digits) - 1

            # Format as d.dddddddddddddddd (first digit, then decimal, then 16 more digits)
            mantissa = digits_str[0] + '.' + digits_str[1:17]

            # Format exponent with sign and 3 digits
            if actual_exp >= 0:
                exp_str = f"+{actual_exp:03d}"
            else:
                exp_str = f"-{abs(actual_exp):03d}"

            # Add negative sign if needed
            result = mantissa + 'E' + exp_str
            if sign:
                result = '-' + result

            return result

        # Check midpoint lines
        assert len(prof.midpoint_values) == len(file_midpoint_lines), \
            f"{profile_name}: Number of midpoints mismatch: {len(prof.midpoint_values)} vs {len(file_midpoint_lines)}"

        for i, (grid_val, prof_val, layer_density) in enumerate(
                zip(grid.midpoints, prof.midpoint_values, prof.layer_densities)):
            file_line_parts = file_midpoint_lines[i].split(',')

            # Column 0: grid coordinate at midpoint
            file_grid_str = file_line_parts[0].strip()
            file_grid = Decimal(file_grid_str)
            grid_decimal = Decimal(str(grid_val))
            # Compare with tolerance appropriate for float64 (about 15-16 significant digits)
            rel_error = abs((grid_decimal - file_grid) / file_grid) if file_grid != 0 else abs(grid_decimal)
            assert rel_error < Decimal('1e-15'), \
                f"{profile_name}: Grid midpoint mismatch at index {i}:\n  Expected: {file_grid_str}\n  Got: {format_scientific(grid_val)}\n  Relative error: {rel_error}"

            # Column 1: profile value at midpoint
            file_prof_str = file_line_parts[1].strip()
            file_prof = Decimal(file_prof_str)
            prof_decimal = Decimal(str(prof_val))
            # Compare with tolerance appropriate for float64
            rel_error = abs((prof_decimal - file_prof) / file_prof) if file_prof != 0 else abs(prof_decimal)
            assert rel_error < Decimal('1e-15'), \
                f"{profile_name}: Profile midpoint value mismatch at index {i}:\n  Expected: {file_prof_str}\n  Got: {format_scientific(prof_val)}\n  Relative error: {rel_error}"

            # Column 3: layer density
            file_layer_density = file_line_parts[3].strip()
            if file_layer_density != '---':
                file_ld = Decimal(file_layer_density)
                ld_decimal = Decimal(str(layer_density))
                rel_error = abs((ld_decimal - file_ld) / file_ld) if file_ld != 0 else abs(ld_decimal)
                assert rel_error < Decimal('1e-15'), \
                    f"{profile_name}: Layer density mismatch at index {i}:\n  Expected: {file_layer_density}\n  Got: {format_scientific(layer_density)}\n  Relative error: {rel_error}"

        # Check edge lines if they exist
        if file_edge_lines:
            assert len(prof.edge_values) == len(file_edge_lines), \
                f"{profile_name}: Number of edges mismatch: {len(prof.edge_values)} vs {len(file_edge_lines)}"

            for i, (grid_val, prof_val) in enumerate(zip(grid.edges, prof.edge_values)):
                file_line_parts = file_edge_lines[i].split(',')

                # Column 0: grid coordinate at edge
                file_grid_str = file_line_parts[0].strip()
                file_grid = Decimal(file_grid_str)
                grid_decimal = Decimal(str(grid_val))
                rel_error = abs((grid_decimal - file_grid) / file_grid) if file_grid != 0 else abs(grid_decimal)
                assert rel_error < Decimal('1e-15'), \
                    f"{profile_name}: Grid edge mismatch at index {i}:\n  Expected: {file_grid_str}\n  Got: {format_scientific(grid_val)}\n  Relative error: {rel_error}"

                # Column 1: profile value at edge
                file_prof_str = file_line_parts[1].strip()
                file_prof = Decimal(file_prof_str)
                prof_decimal = Decimal(str(prof_val))
                rel_error = abs((prof_decimal - file_prof) / file_prof) if file_prof != 0 else abs(prof_decimal)
                assert rel_error < Decimal('1e-15'), \
                    f"{profile_name}: Profile edge value mismatch at index {i}:\n  Expected: {file_prof_str}\n  Got: {format_scientific(prof_val)}\n  Relative error: {rel_error}"

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
