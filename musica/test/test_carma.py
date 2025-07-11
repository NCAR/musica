import pytest
import musica

available = musica.backend.carma_available()


@pytest.mark.skipif(not available, reason="CARMA backend is not available")
def test_carma_version():
    version = musica.carma.version
    assert version is not None
    assert isinstance(version, str)
    print(f"CARMA version: {version}")


if __name__ == '__main__':
    pytest.main([__file__])
    