import pytest
import musica

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")

def test_carma_sulfate():
    from musica.examples import carma_sulfate
    env_state, gas_state, bin_state = carma_sulfate.run_carma_sulfate_example()
    # Basic assertions to verify the simulation ran successfully
    assert env_state is not None, "Environmental state should not be None"
    assert gas_state is not None, "Gas state should not be None"
    assert bin_state is not None, "Bin state should not be None"
    # Optionally, check expected dimensions or variables
    assert hasattr(bin_state, "mass_mixing_ratio"), "Bin state should have mass_mixing_ratio"