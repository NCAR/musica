import pytest
import musica

try:
    import ussa1976
    ussa1976_available = True
except ImportError:
    ussa1976_available = False

carma_available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not carma_available or not ussa1976_available,
    reason="CARMA backend is not available" if not carma_available else "ussa1976 module is not available. This likely means you are running on arm-windows and netcdf4 is not available for this platform so ussa1976 cannot be installed."
)


def test_carma_aluminum():
    from musica.examples import carma_aluminum
    state = carma_aluminum.run_carma_aluminum_example()
    assert state is not None, "State should not be None"
