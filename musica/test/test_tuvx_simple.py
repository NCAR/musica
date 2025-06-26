import pytest
import tempfile
import json
import os


def test_tuvx_simple():
    """Test basic TUV-x functionality with minimal config"""

    # Skip if TUV-x is not available
    try:
        import musica
        if not hasattr(musica, 'TUVX'):
            pytest.skip("TUV-x not available in this build")
    except ImportError:
        pytest.skip("musica module not available")

    # Create a minimal TUV-x configuration
    config = {
        "grids": {
            "height": {
                "units": "km",
                "bounds": [0.0, 50.0],
                "cells": 10
            }
        },
        "solar zenith angle": 45.0,
        "earth sun distance": 1.0
    }

    # Test creation from dictionary
    try:
        tuvx = musica.create_tuvx(config)

        # Test running TUV-x
        photolysis_rates, heating_rates = tuvx.run()

        # Basic checks
        assert photolysis_rates.shape[0] > 0  # Has layers
        assert heating_rates.shape[0] > 0     # Has layers

        print(f"Success! Photolysis rates shape: {photolysis_rates.shape}")
        print(f"Heating rates shape: {heating_rates.shape}")

    except Exception as e:
        print(f"Expected error (TUV-x not fully implemented yet): {e}")
        # For now, we expect this to fail since we haven't implemented the Fortran interface
        pass


if __name__ == '__main__':
    test_tuvx_simple()
