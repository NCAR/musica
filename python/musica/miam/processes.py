"""Aqueous-phase processes for MIAM."""

from dataclasses import dataclass, field
from typing import Callable, List, Optional, Union

from .constants import ArrheniusRateConstant, EquilibriumConstant, HenrysLawConstant

RateConstantType = Union[ArrheniusRateConstant, Callable[[float], float]]


@dataclass
class DissolvedReaction:
    """Irreversible dissolved (aqueous-phase) reaction.

    rate = k(T) * [reactant1] * [reactant2] * ... / [solvent]^(n_reactants - 1)

    Args:
        phase_name: Name of the condensed phase where the reaction occurs.
        reactant_names: Species names of reactants.
        product_names: Species names of products.
        solvent_name: Name of the solvent species.
        rate_constant: An ``ArrheniusRateConstant`` or callable ``f(T) -> k``.
    """
    phase_name: str
    reactant_names: List[str]
    product_names: List[str]
    solvent_name: str
    rate_constant: RateConstantType


@dataclass
class DissolvedReversibleReaction:
    """Reversible dissolved (aqueous-phase) reaction.

    Provide either forward + reverse rate constants, or forward + equilibrium constant.

    Args:
        phase_name: Name of the condensed phase where the reaction occurs.
        reactant_names: Species names of reactants.
        product_names: Species names of products.
        solvent_name: Name of the solvent species.
        forward_rate_constant: Forward rate constant (optional).
        reverse_rate_constant: Reverse rate constant (optional).
        equilibrium_constant: Equilibrium constant (optional, used with forward rate).
    """
    phase_name: str
    reactant_names: List[str]
    product_names: List[str]
    solvent_name: str
    forward_rate_constant: Optional[RateConstantType] = None
    reverse_rate_constant: Optional[RateConstantType] = None
    equilibrium_constant: Optional[EquilibriumConstant] = None


@dataclass
class HenryLawPhaseTransfer:
    """Henry's Law gas-to-aqueous phase transfer.

    Models the mass transfer of a gas-phase species dissolving into a
    condensed phase according to Henry's Law.

    Args:
        condensed_phase_name: Name of the condensed phase receiving the species.
        gas_species_name: Name of the gas-phase species.
        condensed_species_name: Name of the dissolved species in the condensed phase.
        solvent_name: Name of the solvent species in the condensed phase.
        henrys_law_constant: Henry's Law constant with temperature dependence.
        diffusion_coefficient: Gas-phase diffusion coefficient [m2 s-1].
        accommodation_coefficient: Mass accommodation coefficient (dimensionless, 0-1).
    """
    condensed_phase_name: str
    gas_species_name: str
    condensed_species_name: str
    solvent_name: str
    henrys_law_constant: HenrysLawConstant
    diffusion_coefficient: float
    accommodation_coefficient: float
