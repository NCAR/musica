import pytest
import musica
import json

available = musica.backend.tuvx_available()
pytestmark = pytest.mark.skipif(not available, reason="TUV-x backend is not available")

def test_tuvx_version():
    version = musica.tuvx.version
    assert version is not None
    assert isinstance(version, str)


def test_full_tuvx(monkeypatch):
    monkeypatch.chdir("configs/tuvx")
    file = "tuv_5_4.json"
    tuvx = musica.TUVX(file)
    assert tuvx is not None

    heating_rates = tuvx.heating_rate_names
    photolysis_rates = tuvx.photolysis_rate_names
    rates = tuvx.run()

    assert len(rates) > 0, "No photolysis rates found"
    assert len(heating_rates) == 0, "Heating rates should be empty for this config"
    assert len(photolysis_rates) > 0, "No photolysis rates found"


def test_fixed_tuvx(monkeypatch):
    monkeypatch.chdir("src")
    file = "test/data/tuvx/fixed/config.json"
    tuvx = musica.TUVX(file)
    assert tuvx is not None

    # Access properties multiple times
    photolysis_names_1 = tuvx.photolysis_rate_names
    photolysis_names_2 = tuvx.photolysis_rate_names
    heating_names_1 = tuvx.heating_rate_names
    heating_names_2 = tuvx.heating_rate_names
    
    # Verify they return the same object (cached)
    assert photolysis_names_1 is photolysis_names_2
    assert heating_names_1 is heating_names_2

    assert len(heating_names_1) > 0, "No heating rates found"
    assert len(photolysis_names_1) > 0, "No photolysis rates found"

    # rates = tuvx.run()
    # print(f"Rates: {rates}")


# def test_create_config_from_dict():
#     """Test creating a TUVX instance from a dictionary."""
#     config_dict = {
#         "__description": "Test configuration",
#         "grids": [
#             {
#                 "name": "height",
#                 "units": "km",
#                 "type": "from config file",
#                 "values": [0.0, 1.0, 2.0]
#             }
#         ],
#         "profiles": [
#             {
#                 "name": "temperature",
#                 "units": "K",
#                 "type": "from config file",
#                 "grid": {"name": "height", "units": "km"},
#                 "values": [300, 275, 260]
#             }
#         ]
#     }
    
#     # Test the method
#     tuvx_instance = musica.TUVX.create_config_from_dict(config_dict)
    
#     # Verify the method returns a TUVX instance
#     assert isinstance(tuvx_instance, musica.TUVX)
#     assert hasattr(tuvx_instance, 'photolysis_rate_names')
#     assert hasattr(tuvx_instance, 'heating_rate_names')


# def test_create_config_from_json_string():
#     """Test creating a TUVX instance from a JSON string."""
#     config_dict = {
#         "__description": "Test configuration from JSON string",
#         "grids": [
#             {
#                 "name": "wavelength",
#                 "units": "nm",
#                 "type": "from config file",
#                 "values": [300, 400, 500]
#             }
#         ]
#     }
    
#     json_string = json.dumps(config_dict, indent=2)
    
#     # Test the method
#     tuvx_instance = musica.TUVX.create_config_from_json_string(json_string)
    
#     # Verify the method returns a TUVX instance
#     assert isinstance(tuvx_instance, musica.TUVX)
#     assert hasattr(tuvx_instance, 'photolysis_rate_names')
#     assert hasattr(tuvx_instance, 'heating_rate_names')


def test_tuvx_initialization_errors():
    """Test error handling during TUVX initialization."""
    # Test with non-existent file
    with pytest.raises(FileNotFoundError):
        musica.TUVX("non_existent_config.json")

if __name__ == '__main__':
    pytest.main([__file__])
