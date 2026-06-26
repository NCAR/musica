import pytest
import musica.backend

try:
    import ussa1976  # noqa: F401
    ussa1976_available = True
except ImportError:
    ussa1976_available = False

pytestmark = pytest.mark.skipif(
    not musica.backend.tuvx_available() or not ussa1976_available,
    reason="TUV-x backend is not available" if not musica.backend.tuvx_available()
    else "ussa1976 module is not available"
)


def test_ts1_box_model():
    from musica.examples.ts1_box_model import main
    ds = main(plot=False)
    assert len(ds.time) > 0, "Dataset should have time steps"
    assert ds.sizes["grid_cell"] > 0, "Dataset should have grid cells"
