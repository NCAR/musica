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


def test_carma_parameters():
    # Test default parameters
    params = musica.carma.CARMAParameters()
    assert params.max_bins == 100
    assert params.nbin == 5
    assert params.dtime == 1800.0

    # Test custom parameters
    custom_params = musica.carma.CARMAParameters(
        nbin=10,
        dtime=900.0,
        nstep=200
    )
    assert custom_params.nbin == 10
    assert custom_params.dtime == 900.0
    assert custom_params.nstep == 200

    # Test to_dict and from_dict
    params_dict = custom_params.to_dict()
    assert isinstance(params_dict, dict)
    assert params_dict['nbin'] == 10

    restored_params = musica.carma.CARMAParameters.from_dict(params_dict)
    assert restored_params.nbin == custom_params.nbin
    assert restored_params.dtime == custom_params.dtime


def test_carma_test_configurations():
    # Test aluminum test parameters
    aluminum_params = musica.CARMA.get_aluminum_test_parameters()
    assert isinstance(aluminum_params, musica.carma.CARMAParameters)
    assert aluminum_params.nbin == 5

    # Test fractal optics test parameters
    fractal_params = musica.CARMA.get_fractal_optics_test_parameters()
    assert isinstance(fractal_params, musica.carma.CARMAParameters)
    assert fractal_params.nbin == 5

    # Test sulfate test parameters
    sulfate_params = musica.CARMA.get_sulfate_test_parameters()
    assert isinstance(sulfate_params, musica.carma.CARMAParameters)
    assert sulfate_params.nbin == 22
    assert sulfate_params.ngas == 2


def test_carma_instance():
    # Test CARMA instance creation
    carma = musica.CARMA()
    assert carma is not None

    # Test running with default parameters
    params = musica.carma.CARMAParameters(nstep=1)  # Short run for testing
    carma.run(params)

    # Test running with test configuration
    test_params = musica.CARMA.get_aluminum_test_parameters()
    test_params.nstep = 560
    print(test_params.to_dict())
    carma.run(test_params)


if __name__ == '__main__':
    pytest.main([__file__])
