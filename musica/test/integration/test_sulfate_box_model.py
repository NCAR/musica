import pytest
import musica

available = musica.backend.carma_available()
pytestmark = pytest.mark.skipif(
    not available, reason="CARMA backend is not available")

def test_sulfate_box_model():
    """Test the sulfate box model implementation."""
    from musica.examples import sulfate_box_model
    concentrations, times, sulfate_data = sulfate_box_model.run_box_model()

    # Basic assertions to verify the simulation ran successfully
    assert concentrations is not None, "Concentrations should not be None"
    assert len(concentrations) > 0, "Should have concentration data"
    assert times is not None, "Times should not be None"
    assert len(times) > 0, "Should have time data"
    assert sulfate_data is not None, "CARMA sulfate data should not be None"

    # Check that we have the expected species
    expected_species = ["HO2", "H2O2", "SO2", "SO3", "H2SO4", "H2O"]
    for species in expected_species:
        assert species in concentrations.columns, f"Missing species: {species}"

    # Check that CARMA data has expected structure
    assert hasattr(sulfate_data, "mass_mixing_ratio"), "CARMA data should have mass_mixing_ratio"
    assert "time" in sulfate_data.dims, "CARMA data should have time dimension"
    assert "bin" in sulfate_data.dims, "CARMA data should have bin dimension"

    print(f"✅ Test passed! Simulated {len(times)} time steps over {times[-1]:.2f} hours")
    print(f"   Chemical species tracked: {list(concentrations.columns)}")
    print(f"   CARMA bins: {len(sulfate_data.bin)}")
    print(f"   Vertical levels: {len(sulfate_data.vertical_center)}")

