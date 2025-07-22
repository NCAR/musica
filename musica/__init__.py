"""
MUSICA: A Python library for atmospheric chemistry simulations.
"""

try:
    from ._version import version as __version__
except ImportError:
    __version__ = "0.12.0"
from .types import MICM, SolverType, State, Conditions
from . import mechanism_configuration
from .tuvx import TUVX
from .carma import CARMA

__all__ = [
    "MICM", "SolverType", "State", "Conditions", "mechanism_configuration", "TUVX", "CARMA"
]
