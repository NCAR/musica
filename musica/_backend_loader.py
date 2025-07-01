"""
Backend selection module for GPU/CPU backends.
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


# Backend will be loaded lazily
_backend = None


def get_backend():
    """Get the appropriate backend module."""
    global _backend
    if _backend is None:
        if _gpu_deps_installed():
            from . import _musica_gpu as _backend
        else:
            from . import _musica as _backend
    return _backend
