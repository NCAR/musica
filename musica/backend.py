"""
Backend selection and symbol loading for MUSICA.
"""
import importlib.util


def _safe_find_spec(name):
    try:
        return importlib.util.find_spec(name)
    except ModuleNotFoundError:
        return None


def _gpu_deps_installed():
    return (
        _safe_find_spec("nvidia.cublas") is not None or
        _safe_find_spec("nvidia_cuda_runtime") is not None or
        _safe_find_spec("nvidia-cublas-cu12") is not None or
        _safe_find_spec("nvidia-cuda-runtime-cu12") is not None
    )


def get_backend():
    """Get the appropriate backend module."""
    if _gpu_deps_installed():
        import musica._musica_gpu as backend
    else:
        import musica._musica as backend
    return backend


def tuvx_available():
    """Check if the TUVX backend is available."""
    _backend = get_backend()
    return hasattr(_backend._tuvx, "_get_tuvx_version")


def carma_available():
    """Check if the CARMA backend is available."""
    _backend = get_backend()
    return hasattr(_backend._carma, "_get_carma_version")
