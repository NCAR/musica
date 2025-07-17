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

    # these fail to run due to missing solar zenith angle, but that's not required and these should be able to run
    # rates = tuvx.run()
    # print(f"Rates: {rates}")


def test_tuvx_initialization_errors():
    """Test error handling during TUVX initialization."""
    # Test with non-existent file
    with pytest.raises(FileNotFoundError):
        musica.TUVX("non_existent_config.json")


if __name__ == '__main__':
    pytest.main([__file__])
