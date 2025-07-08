import pytest
import musica

available = musica.backend.tuvx_available()
@pytest.mark.skipif(not available, reason="TUV-x backend is not available")

def test_tuvx_version():
    version = musica.TUVX.version()
    print(f"TUV-x version: {version}")
    assert version is not None
    assert isinstance(version, str)

if __name__ == '__main__':
    pytest.main([__file__])
