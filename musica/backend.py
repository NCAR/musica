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
        from . import _musica_gpu as backend
    else:
        from . import _musica as backend
    return backend


# Load backend and expose symbols at module level
_backend = get_backend()

# Core symbols
Conditions = _backend._core._Conditions
SolverType = _backend._core._SolverType
Solver = _backend._core._Solver
State = _backend._core._State
create_solver = _backend._core._create_solver
create_solver_from_mechanism = _backend._core._create_solver_from_mechanism
create_state = _backend._core._create_state
micm_solve = _backend._core._micm_solve
vector_size = _backend._core._vector_size
species_ordering = _backend._core._species_ordering
user_defined_rate_parameters_ordering = _backend._core._user_defined_rate_parameters_ordering

# Mechanism configuration
mechanism_configuration = _backend._mechanism_configuration

_tuvx = _backend._tuvx

_is_cuda_available = _backend._core._is_cuda_available
