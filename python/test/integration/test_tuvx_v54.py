"""
Test against stand-alone TUV-x version 5.4 configuration
"""
import os
import subprocess
import pytest
import numpy as np
import xarray as xr
import musica
from musica.tuvx import v54

@pytest.mark.skipif(not musica.backend.tuvx_available(),
                    reason="TUV-x backend is not available")
@pytest.mark.skipif(os.getenv("TUVX_ROOT") is None,
                    reason="TUVX_ROOT environment variable is not set")

def test_v54_against_standalone_tuvx():
    """
    Compare Python interface results with standalone TUV-x executable.
    
    This test runs the standalone TUV-x binary from TUVX_ROOT and compares
    its NetCDF outputs with results from the Python interface using the same
    configuration.
    """
    tuvx_root = os.getenv("TUVX_ROOT")
    executable = os.path.join(tuvx_root, "build", "tuv-x")
    config_file = os.path.join(tuvx_root, "examples", "tuv_5_4.json")
    
    # Verify executable and config exist
    assert os.path.isfile(executable), f"TUV-x executable not found: {executable}"
    assert os.path.isfile(config_file), f"Config file not found: {config_file}"
    
    # Run standalone TUV-x executable
    result = subprocess.run(
        [executable, config_file],
        cwd=tuvx_root,
        capture_output=True,
        text=True,
        timeout=60
    )
    
    assert result.returncode == 0, f"TUV-x executable failed:\nstdout: {result.stdout}\nstderr: {result.stderr}"
    
    # Read standalone TUV-x output files
    photolysis_nc = os.path.join(tuvx_root, "photolysis_rate_constants.nc")
    dose_rates_nc = os.path.join(tuvx_root, "dose_rates.nc")
    
    assert os.path.isfile(photolysis_nc), f"Output file not found: {photolysis_nc}"
    assert os.path.isfile(dose_rates_nc), f"Output file not found: {dose_rates_nc}"
    
    standalone_photolysis = xr.open_dataset(photolysis_nc, decode_timedelta=False)
    standalone_dose = xr.open_dataset(dose_rates_nc, decode_timedelta=False)
    
    # Get solar zenith angles and Earth-Sun distances from standalone output
    sza_values = standalone_photolysis["solar zenith angle"].values
    earth_sun_dist_values = standalone_photolysis["Earth-Sun distance"].values
    n_times = len(sza_values)
    
    # Create Python interface TUV-x instance
    tuvx = v54.get_tuvx_calculator()
    
    # Get grids and profiles for comparison
    grids = tuvx.get_grid_map()
    profiles = tuvx.get_profile_map()
    height_grid = grids["height", "km"]
    wavelength_grid = grids["wavelength", "nm"]
    temperature_profile = profiles["temperature", "K"]
    
    # Variables that should be skipped in comparison (only time-varying metadata)
    skip_vars = {'time', 'solar zenith angle', 'Earth-Sun distance'}
    
    # Compare static grids and profiles (only need to do this once)
    # Compare altitude grid (standalone uses edges, which correspond to vertical_level dimension)
    standalone_altitude = standalone_photolysis["altitude"].values
    np.testing.assert_allclose(
        standalone_altitude, height_grid.edges,
        rtol=1e-10, atol=1e-10,
        err_msg="Altitude grid mismatch"
    )
    
    # Compare wavelength grid (standalone uses midpoints)
    standalone_wavelength = standalone_photolysis["wavelength"].values
    np.testing.assert_allclose(
        standalone_wavelength, wavelength_grid.midpoints,
        rtol=1e-10, atol=1e-10,
        err_msg="Wavelength grid mismatch"
    )
    
    # Loop over each time step
    for time_idx in range(n_times):
        # Convert solar zenith angle from degrees to radians
        sza_rad = np.deg2rad(sza_values[time_idx])
        earth_sun_dist = earth_sun_dist_values[time_idx]
        
        # Run Python interface for this time step
        python_result = tuvx.run(sza_rad, earth_sun_dist)
        
        # Compare temperature profile for this time step
        standalone_temp = standalone_photolysis["temperature"].isel(time=time_idx).values
        np.testing.assert_allclose(
            standalone_temp, temperature_profile.edge_values,
            rtol=1e-10, atol=1e-10,
            err_msg=f"Temperature profile mismatch at time={time_idx}"
        )
        
        # Compare radiation arrays (actinic flux)
        # Standalone has: direct radiation, upward radiation, downward radiation
        # Python has: actinic_flux with components [direct, upwelling, downwelling]
        standalone_direct = standalone_photolysis["direct radiation"].isel(time=time_idx).values
        standalone_upward = standalone_photolysis["upward radiation"].isel(time=time_idx).values
        standalone_downward = standalone_photolysis["downward radiation"].isel(time=time_idx).values
        
        python_actinic = python_result['actinic_flux'].values
        # python_actinic shape: (wavelength, vertical_level, 3)
        # standalone shape: (wavelength, vertical_level)
        
        np.testing.assert_allclose(
            standalone_direct, python_actinic[:, :, 0],
            rtol=3e-2, atol=1e-4,
            err_msg=f"Direct radiation mismatch at time={time_idx}"
        )
        np.testing.assert_allclose(
            standalone_upward, python_actinic[:, :, 1],
            rtol=3e-2, atol=1e-4,
            err_msg=f"Upward radiation mismatch at time={time_idx}"
        )
        np.testing.assert_allclose(
            standalone_downward, python_actinic[:, :, 2],
            rtol=3e-2, atol=1e-4,
            err_msg=f"Downward radiation mismatch at time={time_idx}"
        )
        
        # Compare photolysis rate constants
        for var_name in standalone_photolysis.data_vars:
            if var_name in skip_vars:
                continue
            
            # Skip variables we've already compared
            if var_name in {'altitude', 'wavelength', 'temperature', 
                           'direct radiation', 'upward radiation', 'downward radiation',
                           'vertical_level'}:
                continue
            
            if var_name.startswith("cross section") or var_name.startswith("quantum yield"):
                # Skip cross sections and quantum yields - they're diagnostic outputs
                continue
            
            # Get standalone data for this time slice
            standalone_data = standalone_photolysis[var_name].isel(time=time_idx).values
            
            # Get Python data - reactions are stored as coordinates
            if var_name in python_result.coords.get('reaction', []):
                python_data = python_result['photolysis_rate_constants'].sel(reaction=var_name).values
            else:
                pytest.fail(f"Reaction {var_name} not found in Python output at time={time_idx}")
            
            # Compare shapes
            assert standalone_data.shape == python_data.shape, \
                f"Shape mismatch for {var_name} at time={time_idx}: {standalone_data.shape} vs {python_data.shape}"
            
            # Use relative tolerance for non-zero values
            mask = np.abs(standalone_data) > 1e-30
            if np.any(mask):
                rel_diff = np.abs((standalone_data[mask] - python_data[mask]) / standalone_data[mask])
                max_rel_diff = np.max(rel_diff)
                assert max_rel_diff < 3e-2, \
                    f"Relative difference for {var_name} at time={time_idx} exceeds tolerance: {max_rel_diff:.2e}"
            
            # Use absolute tolerance for near-zero values
            mask_zero = np.abs(standalone_data) <= 1e-30
            if np.any(mask_zero):
                abs_diff = np.abs(standalone_data[mask_zero] - python_data[mask_zero])
                max_abs_diff = np.max(abs_diff)
                assert max_abs_diff < 1e-30, \
                    f"Absolute difference for {var_name} (near-zero values) at time={time_idx} exceeds tolerance: {max_abs_diff:.2e}"
        
        # Compare dose rates
        for var_name in standalone_dose.data_vars:
            if var_name in skip_vars:
                continue
            
            # Skip variables we've already compared
            if var_name in {'altitude', 'wavelength', 'temperature',
                           'direct radiation', 'upward radiation', 'downward radiation',
                           'vertical_level'}:
                continue
            
            if var_name.startswith("cross section") or var_name.startswith("quantum yield"):
                continue
            
            # Get standalone data for this time slice
            standalone_data = standalone_dose[var_name].isel(time=time_idx).values
            
            # Get Python data - dose rates are stored as coordinates
            # NetCDF sanitizes variable names (e.g., "/" becomes "_"), so we need to check both
            python_dose_rates = python_result.coords.get('dose_rate', [])
            if var_name in python_dose_rates:
                python_data = python_result['dose_rates'].sel(dose_rate=var_name).values
            else:
                # Try with common character replacements (NetCDF sanitization)
                normalized_name = var_name.replace('_', '/')
                if normalized_name in python_dose_rates:
                    python_data = python_result['dose_rates'].sel(dose_rate=normalized_name).values
                else:
                    # Skip dose rates not found in Python output
                    continue
            
            # Compare shapes
            assert standalone_data.shape == python_data.shape, \
                f"Shape mismatch for {var_name} at time={time_idx}: {standalone_data.shape} vs {python_data.shape}"
            
            # Use relative tolerance for non-zero values
            mask = np.abs(standalone_data) > 1e-30
            if np.any(mask):
                rel_diff = np.abs((standalone_data[mask] - python_data[mask]) / standalone_data[mask])
                max_rel_diff = np.max(rel_diff)
                assert max_rel_diff < 3e-2, \
                    f"Relative difference for {var_name} at time={time_idx} exceeds tolerance: {max_rel_diff:.2e}"
            
            # Use absolute tolerance for near-zero values
            mask_zero = np.abs(standalone_data) <= 1e-30
            if np.any(mask_zero):
                abs_diff = np.abs(standalone_data[mask_zero] - python_data[mask_zero])
                max_abs_diff = np.max(abs_diff)
                assert max_abs_diff < 1e-30, \
                    f"Absolute difference for {var_name} (near-zero values) at time={time_idx} exceeds tolerance: {max_abs_diff:.2e}"
    
    # Clean up
    standalone_photolysis.close()
    standalone_dose.close()


