import pytest

scipy_available = pytest.importorskip("scipy", reason="scipy is required for Latin Hypercube sampling")


def test_ts1_latin_hypercube():
    from musica.examples.ts1_latin_hypercube import main
    ds = main(plot=False)
    assert len(ds.time) > 0, "Dataset should have time steps"
    assert ds.sizes["grid_cell"] == 100, "Dataset should have 100 grid cells"
