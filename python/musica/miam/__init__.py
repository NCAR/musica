"""
musica.miam - MIAM aerosol/cloud model for use with MICM.

Provides Python classes for configuring aerosol representations,
aqueous-phase processes, and equilibrium constraints that integrate
with the MICM chemical solver as external models.
"""

from .constants import HenrysLawConstant, EquilibriumConstant, ArrheniusRateConstant
from .representations import UniformSection, SingleMomentMode, TwoMomentMode
from .processes import DissolvedReaction, DissolvedReversibleReaction, HenryLawPhaseTransfer
from .constraints import (
    HenryLawEquilibriumConstraint,
    DissolvedEquilibriumConstraint,
    LinearConstraint,
    LinearConstraintTerm,
)
from .model import Model

__all__ = [
    "HenrysLawConstant",
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
