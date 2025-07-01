from ._backend_loader import get_backend


def is_cuda_available() -> bool:
    """
    Check if CUDA is available.

    Returns:
        bool: True if CUDA is available, False otherwise.
    """
    _backend = get_backend()
    return _backend._core._is_cuda_available()
