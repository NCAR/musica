"""
Backend selection module to avoid circular imports.
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


# Set up backend selection
if _gpu_deps_installed():
    from . import _musica_gpu as backend
else:
    from . import _musica as backend


def get_backend():
    """Get the appropriate backend module."""
    return backend


def tuvx_available():
    """Check if TUV-x is available in the backend."""
    try:
        return hasattr(backend, '_tuvx')
    except AttributeError:
        return False
