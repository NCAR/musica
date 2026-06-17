"""
musica.miam - MIAM aerosol/cloud model for use with MICM.

Provides Python classes for configuring aerosol representations,
aqueous-phase processes, and equilibrium constraints that integrate
with the MICM chemical solver as external models.
"""

from .constants import HenryLawConstant, EquilibriumConstant, ArrheniusRateConstant
from .representations import UniformSection, SingleMomentMode, TwoMomentMode
from .processes import DissolvedReaction, DissolvedReversibleReaction, HenryLawPhaseTransfer
from .constraints import (
    HenryLawEquilibriumConstraint,
    DissolvedEquilibriumConstraint,
    LinearConstraint,
    LinearConstraintTerm,
)
from .model import Model
from .. import backend
_backend = backend.get_backend()

__version__ = _backend._miam._get_miam_version() if backend.miam_available() else None

__all__ = [
    "HenryLawConstant",
    "EquilibriumConstant",
    "ArrheniusRateConstant",
    "UniformSection",
    "SingleMomentMode",
    "TwoMomentMode",
    "DissolvedReaction",
    "DissolvedReversibleReaction",
    "HenryLawPhaseTransfer",
    "HenryLawEquilibriumConstraint",
    "DissolvedEquilibriumConstraint",
    "LinearConstraint",
    "LinearConstraintTerm",
    "Model",
]

