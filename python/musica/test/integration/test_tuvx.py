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

    photolysis_rates, heating_rates, dose_rates = tuvx.run()

    assert len(tuvx.heating_rate_names) == 0, "Heating rate names should be empty for this config"
    assert len(tuvx.photolysis_rate_names) > 0, "No photolysis rate names found"
    assert len(tuvx.dose_rate_names) > 0, "No dose rate names found"
    assert len(heating_rates) == 2, "Number of solar zenith angles should be 2 for heating rates"
    assert len(photolysis_rates) == 2, "Number of solar zenith angles should be 2 for photolysis rates"
    assert len(dose_rates) == 2, "Number of solar zenith angles should be 2 for dose rates"
    assert heating_rates.shape[1] == 121, "Number of layers should be 121 for heating rates"
    assert photolysis_rates.shape[1] == 121, "Number of layers should be 121 for photolysis rates"
    assert dose_rates.shape[1] == 121, "Number of layers should be 121 for dose rates"
    assert heating_rates.shape[2] == 0, "Should be no heating rates for this config"
    assert photolysis_rates.shape[2] == len(tuvx.photolysis_rate_names), "Photolysis rates shape mismatch"
    assert dose_rates.shape[2] == len(tuvx.dose_rate_names), "Dose rates shape mismatch"


def test_fixed_tuvx_from_file(monkeypatch):
    monkeypatch.chdir("src")
    file = "test/data/tuvx/fixed/config.json"
    tuvx = musica.TUVX(config_path=file)
    assert tuvx is not None

    # Access properties multiple times
    photolysis_names_1 = tuvx.photolysis_rate_names
    photolysis_names_2 = tuvx.photolysis_rate_names
    heating_names_1 = tuvx.heating_rate_names
    heating_names_2 = tuvx.heating_rate_names
    dose_names_1 = tuvx.dose_rate_names
    dose_names_2 = tuvx.dose_rate_names

    # Verify they return the same object (cached)
    assert photolysis_names_1 is photolysis_names_2
    assert heating_names_1 is heating_names_2
    assert dose_names_1 is dose_names_2

    assert len(photolysis_names_1) == 3, "Unexpected number of photolysis rates found"
    assert len(heating_names_1) == 2, "Unexpected number of heating rates found"
    assert len(dose_names_1) == 3, "Unexpected number of dose rates found"

    photolysis_rates, heating_rates, dose_rates = tuvx.run()

    assert len(photolysis_rates) == 5, "Unexpected number of solar zenith angles for photolysis rates"
    assert len(heating_rates) == 5, "Unexpected number of solar zenith angles for heating rates"
    assert len(dose_rates) == 5, "Unexpected number of solar zenith angles for dose rates"
    assert photolysis_rates.shape[1] == 4, "Unexpected number of layers for photolysis rates"
    assert heating_rates.shape[1] == 4, "Unexpected number of layers for heating rates"
    assert dose_rates.shape[1] == 4, "Unexpected number of layers for dose rates"
    assert photolysis_rates.shape[2] == len(photolysis_names_1), "Photolysis rates shape mismatch"
    assert heating_rates.shape[2] == len(heating_names_1), "Heating rates shape mismatch"
    assert dose_rates.shape[2] == len(dose_names_1), "Dose rates shape mismatch"


def test_fixed_tuvx_from_string(monkeypatch):
    monkeypatch.chdir("src")
    file = "test/data/tuvx/fixed/config.json"
    with open(file, 'r') as f:
        config_str = f.read()
    tuvx = musica.TUVX(config_string=config_str)
    assert tuvx is not None

    # Access properties multiple times
    photolysis_names_1 = tuvx.photolysis_rate_names
    photolysis_names_2 = tuvx.photolysis_rate_names
    heating_names_1 = tuvx.heating_rate_names
    heating_names_2 = tuvx.heating_rate_names
    dose_names_1 = tuvx.dose_rate_names
    dose_names_2 = tuvx.dose_rate_names

    # Verify they return the same object (cached)
    assert photolysis_names_1 is photolysis_names_2
    assert heating_names_1 is heating_names_2
    assert dose_names_1 is dose_names_2

    assert len(photolysis_names_1) == 3, "Unexpected number of photolysis rates found"
    assert len(heating_names_1) == 2, "Unexpected number of heating rates found"
    assert len(dose_names_1) == 3, "Unexpected number of dose rates found"

    photolysis_rates, heating_rates, dose_rates = tuvx.run()

    assert len(photolysis_rates) == 5, "Unexpected number of solar zenith angles for photolysis rates"
    assert len(heating_rates) == 5, "Unexpected number of solar zenith angles for heating rates"
    assert len(dose_rates) == 5, "Unexpected number of solar zenith angles for dose rates"
    assert photolysis_rates.shape[1] == 4, "Unexpected number of layers for photolysis rates"
    assert heating_rates.shape[1] == 4, "Unexpected number of layers for heating rates"
    assert dose_rates.shape[1] == 4, "Unexpected number of layers for dose rates"
    assert photolysis_rates.shape[2] == len(photolysis_names_1), "Photolysis rates shape mismatch"
    assert heating_rates.shape[2] == len(heating_names_1), "Heating rates shape mismatch"
    assert dose_rates.shape[2] == len(dose_names_1), "Dose rates shape mismatch"


def test_tuvx_initialization_errors():
    """Test error handling during TUVX initialization."""
    # Test with non-existent file
    with pytest.raises(FileNotFoundError):
        musica.TUVX("non_existent_config.json")


if __name__ == '__main__':
    pytest.main([__file__])
