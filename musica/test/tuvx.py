import pytest
import musica
import tempfile
import os
import json
import numpy as np


def test_tuvx_creation():
    """Test that we can create a TUVX instance."""
    # Create a minimal configuration for testing
    config = {
        "grids": {
            "height": {
                "type": "uniform",
                "units": "km",
                "begin": 0.0,
                "end": 120.0,
                "number of sections": 120
            },
            "wavelength": {
                "type": "from file",
                "file path": "data/grids/wavelength/combined.grid",
                "interpolation method": "linear"
            }
        },
        "profiles": {
            "solar zenith angle": {
                "type": "uniform",
                "units": "degrees",
                "values": [0.0, 30.0, 60.0, 90.0]
            },
            "Earth-Sun distance": {
                "type": "uniform",
                "units": "AU",
                "values": [1.0, 1.0, 1.0, 1.0]
            }
        },
        "cross sections": [],
        "photolysis": [],
        "radiators": {}
    }

    # Write config to temporary file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
        json.dump(config, f, indent=2)
        config_path = f.name

    try:
        # Test creating TUVX from file path
        tuvx = musica.TUVX(config_path)
        assert tuvx is not None

        # Test getting orderings (might be empty for minimal config)
        photo_ordering = tuvx.photolysis_rate_constants_ordering
        heating_ordering = tuvx.heating_rates_ordering

        assert isinstance(photo_ordering, dict)
        assert isinstance(heating_ordering, dict)

        # Clean up
        del tuvx

    except Exception as e:
        # If TUV-x is not built with TUVX support, this test will fail
        # That's expected in some build configurations
        if "Error creating TUVX" in str(e):
            pytest.skip(f"TUV-x support not available: {e}")
        else:
            raise e
    finally:
        # Clean up temporary file
        if os.path.exists(config_path):
            os.unlink(config_path)


def test_tuvx_create_config_from_dict():
    """Test creating config from dictionary."""
    config_dict = {
        "grids": {},
        "profiles": {},
        "cross sections": [],
        "photolysis": [],
        "radiators": {}
    }

    with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
        output_path = f.name

    try:
        result_path = musica.TUVX.create_config_from_dict(
            config_dict, output_path)
        assert result_path == output_path
        assert os.path.exists(output_path)

        # Verify content
        with open(output_path, 'r') as f:
            loaded_config = json.load(f)
        assert loaded_config == config_dict

    finally:
        if os.path.exists(output_path):
            os.unlink(output_path)


def test_tuvx_create_config_from_json_string():
    """Test creating config from JSON string."""
    config_dict = {
        "grids": {},
        "profiles": {},
        "cross sections": [],
        "photolysis": [],
        "radiators": {}
    }
    json_string = json.dumps(config_dict)

    with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
        output_path = f.name

    try:
        result_path = musica.TUVX.create_config_from_json_string(
            json_string, output_path)
        assert result_path == output_path
        assert os.path.exists(output_path)

        # Verify content
        with open(output_path, 'r') as f:
            loaded_config = json.load(f)
        assert loaded_config == config_dict

    finally:
        if os.path.exists(output_path):
            os.unlink(output_path)


def test_create_tuvx_convenience_function():
    """Test the convenience create_tuvx function."""
    config_dict = {
        "grids": {},
        "profiles": {},
        "cross sections": [],
        "photolysis": [],
        "radiators": {}
    }

    try:
        # Test with dictionary
        tuvx = musica.create_tuvx(config_dict)
        assert tuvx is not None
        del tuvx

        # Test with JSON string
        json_string = json.dumps(config_dict)
        tuvx = musica.create_tuvx(json_string)
        assert tuvx is not None
        del tuvx

    except Exception as e:
        if "Error creating TUVX" in str(e):
            pytest.skip(f"TUV-x support not available: {e}")
        else:
            raise e


if __name__ == '__main__':
    pytest.main([__file__])
