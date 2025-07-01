import pytest
import musica
import tempfile
import os
import json
import numpy as np

def test_tuvx_from_file(monkeypatch):
    monkeypatch.chdir("src")
    file = "test/data/tuvx/fixed/config.json"
    tuvx = musica.TUVX(file)
    assert tuvx is not None

    heating_rates = tuvx.heating_rate_names
    photolysis_rates = tuvx.photolysis_rate_names
    print("Heating Rates:", heating_rates)
    print("Photolysis Rates:", photolysis_rates)
    rates = tuvx.run()
    print(rates)


# def test_tuvx_creation():
#     """Test that we can create a TUVX instance."""
#     # Create a minimal configuration for testing
#     config = {
#         "grids": {
#             "height": {
#                 "type": "uniform",
#                 "units": "km",
#                 "begin": 0.0,
#                 "end": 120.0,
#                 "number of sections": 120
#             },
#             "wavelength": {
#                 "type": "from file",
#                 "file path": "data/grids/wavelength/combined.grid",
#                 "interpolation method": "linear"
#             }
#         },
#         "profiles": {
#             "solar zenith angle": {
#                 "type": "uniform",
#                 "units": "degrees",
#                 "values": [0.0, 30.0, 60.0, 90.0]
#             },
#             "Earth-Sun distance": {
#                 "type": "uniform",
#                 "units": "AU",
#                 "values": [1.0, 1.0, 1.0, 1.0]
#             }
#         },
#         "cross sections": [],
#         "photolysis": [],
#         "radiators": {}
#     }

#     # Write config to temporary file
#     with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
#         json.dump(config, f, indent=2)
#         config_path = f.name

#     try:
#         # Test creating TUVX from file path
#         tuvx = musica.TUVX(config_path)
#         assert tuvx is not None

#         # Test getting orderings (might be empty for minimal config)
#         photo_ordering = tuvx.photolysis_rate_constants_ordering
#         heating_ordering = tuvx.heating_rates_ordering

#         assert isinstance(photo_ordering, dict)
#         assert isinstance(heating_ordering, dict)

#         # Clean up
#         del tuvx

#     except Exception as e:
#         # If TUV-x is not built with TUVX support, this test will fail
#         # That's expected in some build configurations
#         if "Error creating TUVX" in str(e):
#             pytest.skip(f"TUV-x support not available: {e}")
#         else:
#             raise e
#     finally:
#         # Clean up temporary file
#         if os.path.exists(config_path):
#             os.unlink(config_path)

if __name__ == '__main__':
    pytest.main([__file__])
