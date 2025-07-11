import pytest
import musica

available = musica.backend.tuvx_available()


@pytest.mark.skipif(not available, reason="TUV-x backend is not available")
def test_tuvx_version():
    version = musica.tuvx.version
    assert version is not None
    assert isinstance(version, str)
    print(f"TUV-x version: {version}")

def test_full_tuvx(monkeypatch):
    monkeypatch.chdir("configs/tuvx")
    file = "tuv_5_4.json"
    tuvx = musica.TUVX(file)
    assert tuvx is not None

    heating_rates = tuvx.heating_rate_names
    photolysis_rates = tuvx.photolysis_rate_names
    print(f"Heating rates: {heating_rates}")
    print(f"Photolysis rates: {photolysis_rates}")
    rates = tuvx.run()
    print(f"Rates: {rates}")

    assert len(rates) > 0, "No photolysis rates found"
    assert len(photolysis_rates) > 0, "No photolysis rates found"


def test_fixed_tuvx(monkeypatch):
    monkeypatch.chdir("src")
    file = "test/data/tuvx/fixed/config.json"
    tuvx = musica.TUVX(file)
    assert tuvx is not None

    heating_rates = tuvx.heating_rate_names
    photolysis_rates = tuvx.photolysis_rate_names
    print(f"Heating rates: {heating_rates}")
    print(f"Photolysis rates: {photolysis_rates}")

    assert len(heating_rates) > 0, "No heating rates found"
    assert len(photolysis_rates) > 0, "No photolysis rates found"

    # rates = tuvx.run()
    # print(f"Rates: {rates}")

if __name__ == '__main__':
    pytest.main([__file__])
