import pytest
import musica

available = musica.backend.tuvx_available()


@pytest.mark.skipif(not available, reason="TUV-x backend is not available")
def test_tuvx_version():
    version = musica.tuvx.version
    assert version is not None
    assert isinstance(version, str)
    print(f"TUV-x version: {version}")


if __name__ == '__main__':
    pytest.main([__file__])
