from . import backend

_backend = backend.get_backend()


def is_cuda_available() -> bool:
    """
    Check if CUDA is available.

    Returns:
        bool: True if CUDA is available, False otherwise.
    """
    return _backend._core._is_cuda_available()
