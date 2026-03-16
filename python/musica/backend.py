"""
Backend selection and symbol loading for MUSICA.

With the type-erased solver architecture, there is a single _musica module
that handles both CPU and CUDA solvers. CUDA support is detected at runtime
via the CudaLoader, which uses dlopen to load libmusica_cuda.so when available.
"""


# Global backend instance to ensure it's only loaded once
_backend_instance = None


def get_backend():
    """Get the MUSICA backend module.

    Returns the single _musica module which handles both CPU and CUDA solvers.
    CUDA support is detected and loaded at runtime when a CUDA solver is requested.
    """

    global _backend_instance

    if _backend_instance is not None:
        return _backend_instance

    try:
        import musica._musica as backend
        _backend_instance = backend
        return _backend_instance

    except ImportError as e:
        raise ImportError(
            f"Failed to import MUSICA backend. Make sure the package is properly built and installed. Error: {e}")


def cuda_available():
    """Check if CUDA solvers are available.

    This checks if:
    1. The CUDA plugin library (libmusica_cuda.so) can be loaded
    2. CUDA devices are present on the system

    Returns:
        bool: True if CUDA solvers can be used, False otherwise.
    """
    _backend = get_backend()
    return _backend._micm._is_cuda_available()


def tuvx_available():
    """Check if the TUVX backend is available."""
    _backend = get_backend()
    return hasattr(_backend._tuvx, "_get_tuvx_version")


def carma_available():
    """Check if the CARMA backend is available."""
    _backend = get_backend()
    return hasattr(_backend._carma, "_get_carma_version")
