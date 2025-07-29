import pytest
import musica

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")


def test_carma_version():
    version = musica.carma.version
    assert version is not None
    assert isinstance(version, str)
    print(f"CARMA version: {version}")


def test_carma_instance():
    # Test CARMA instance creation
    test_params = musica.CARMAParameters.create_aluminum_test_config()
    test_params.nstep = 560

    carma = musica.CARMA(test_params)
    assert carma is not None
    assert isinstance(carma, musica.CARMA)

    state = carma.create_state()

    assert state is not None
    assert isinstance(state, musica.CARMAState)

    carma.run()


if __name__ == '__main__':
    pytest.main([__file__])
