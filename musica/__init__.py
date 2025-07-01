from ._version import version as __version__
from ._backend_loader import get_backend, tuvx_available

# Get backend and export symbols for backward compatibility
_backend = get_backend()

# Export core symbols
from ._backend_loader import get_backend
_backend = get_backend()
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

# Helper to re-export names from a module
def _export_all(module, names, globals_):
    for name in names:
        globals_[name] = getattr(module, name)

# Export backend symbols for backward compatibility
_export_all(_backend._core, _core_names, globals())
_export_all(_backend._mechanism_configuration, _mechanism_names, globals())

# Import user-facing classes and functions
from .types import MICM, SolverType, State, Conditions

# Set up exports based on TUV-x availability
if tuvx_available():
    from .tuvx import TUVX, create_tuvx, TUVXNotAvailableError
    __all__ = ["MICM", "SolverType", "State", "Conditions",
               "TUVX", "create_tuvx", "TUVXNotAvailableError"]
else:
    from .tuvx import TUVXNotAvailableError
    __all__ = ["MICM", "SolverType", "State",
               "Conditions", "TUVXNotAvailableError"]
