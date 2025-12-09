import musica
import pytest


def test_musica_version():
    version = musica.__version__
    print(f"MUSICA version: {version}")
    assert isinstance(version, str)
    assert len(version) > 0


def test_micm_version():
    micm_version = musica.micm.__version__
    print(f"MICM version: {micm_version}")
    assert isinstance(micm_version, str)
    assert len(micm_version) > 0


@pytest.mark.skipif(not musica.backend.tuvx_available(), reason="TUV-x backend is not available")
def test_tuvx_version():
    tuvx_version = musica.tuvx.__version__
    print(f"TUV-x version: {tuvx_version}")
    assert isinstance(tuvx_version, str)
    assert len(tuvx_version) > 0


@pytest.mark.skipif(not musica.backend.carma_available(), reason="CARMA backend is not available")
def test_carma_version():
    carma_version = musica.carma.__version__
    print(f"CARMA version: {carma_version}")
    assert isinstance(carma_version, str)
    assert len(carma_version) > 0
