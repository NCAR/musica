"""
MUSICA: A Python library for atmospheric chemistry simulations.
"""

from ._version import version as __version__
from .types import MICM, SolverType, State, Conditions
from . import mechanism_configuration

__all__ = [
    "MICM", "SolverType", "State", "Conditions", "mechanism_configuration"
]

from ._backend_loader import tuvx_available
if tuvx_available():
    from .tuvx import TUVX, create_tuvx
    __all__.extend([
        "TUVX", "create_tuvx"
    ])
