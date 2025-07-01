"""
MUSICA: A Python library for atmospheric chemistry simulations.
"""

from ._version import version as __version__
from .types import MICM, SolverType, State, Conditions
from . import mechanism_configuration

# Import TUV-x directly since it's now available on all platforms
from .tuvx import TUVX, create_tuvx

__all__ = [
    "MICM", "SolverType", "State", "Conditions", "mechanism_configuration",
    "TUVX", "create_tuvx"
]
