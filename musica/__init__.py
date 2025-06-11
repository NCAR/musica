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


if _gpu_deps_installed():
    from . import _musica_gpu as _backend
else:
    from . import _musica as _backend

# Helper to re-export names from a module
def _export_all(module, names, globals_):
    for name in names:
        globals_[name] = getattr(module, name)


_core_names = [
    "_Conditions", "_SolverType", "_Solver", "_State", "_create_solver",
    "_create_solver_from_mechanism", "_create_state", "_micm_solve", "_vector_size",
    "_species_ordering", "_user_defined_rate_parameters_ordering",
]
_mechanism_names = [
    "_ReactionType", "_Species", "_Phase", "_ReactionComponent", "_Arrhenius",
    "_CondensedPhaseArrhenius", "_Troe", "_Branched", "_Tunneling", "_Surface",
    "_Photolysis", "_CondensedPhasePhotolysis", "_Emission", "_FirstOrderLoss",
    "_AqueousEquilibrium", "_WetDeposition", "_HenrysLaw", "_SimpolPhaseTransfer",
    "_UserDefined", "_Reactions", "_ReactionsIterator", "_Mechanism", "_Version", "_Parser"
]

# this allows us to use the same symbols in both the GPU and CPU versionspp
_export_all(_backend._core, _core_names, globals())
_export_all(_backend._mechanism_configuration, _mechanism_names, globals())

__all__ = _core_names + _mechanism_names

from .types import MICM, SolverType, State, Conditions
from ._version import version as __version__
