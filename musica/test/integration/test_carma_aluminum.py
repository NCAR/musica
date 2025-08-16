import pytest
import musica

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")

def test_carma_aluminum():
    from musica.examples import carma_aluminum
    state = carma_aluminum.run_carma_aluminum_example()
    assert state is not None, "State should not be None"