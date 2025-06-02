from musica._musica._cuda import _try_load_cuda, _is_cuda_available

def is_cuda_available() -> bool:
    """
    Check if CUDA is available.

    Returns:
        bool: True if CUDA is available, False otherwise.
    """
    _try_load_cuda()
    return _is_cuda_available()

def _try_load_cuda():
    """
    Attempt to load CUDA-related libraries.
    """
    try:
        _try_load_cuda()
    except ImportError:
      raise ImportError(
          "CUDA is not available. Please ensure that you have the necessary CUDA libraries installed and configured correctly."
      )