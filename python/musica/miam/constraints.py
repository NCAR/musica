"""Equilibrium and algebraic constraints for MIAM."""

from dataclasses import dataclass, field
from typing import List

from .constants import EquilibriumConstant, HenrysLawConstant


@dataclass(frozen=True)
class LinearConstraintTerm:
    """A single term in a linear algebraic constraint.

    Args:
        phase_name: Name of the phase containing the species.
        species_name: Name of the species.
        coefficient: Stoichiometric coefficient (dimensionless).
    """
    phase_name: str
    species_name: str
    coefficient: float


@dataclass(frozen=True)
class HenryLawEquilibriumConstraint:
    """Algebraic constraint enforcing Henry's Law equilibrium.

    Constrains the dissolved concentration of a gas-phase species
    to satisfy Henry's Law at equilibrium.

    Args:
        gas_species_name: Name of the gas-phase species.
        condensed_species_name: Name of the dissolved species.
        solvent_name: Name of the solvent species.
        condensed_phase_name: Name of the condensed phase.
        henrys_law_constant: Henry's Law constant.
        mw_solvent: Molecular weight of solvent [kg mol-1].
        rho_solvent: Density of solvent [kg m-3].
    """
    gas_species_name: str
    condensed_species_name: str
    solvent_name: str
    condensed_phase_name: str
    henrys_law_constant: HenrysLawConstant
    mw_solvent: float
    rho_solvent: float


@dataclass(frozen=True)
class DissolvedEquilibriumConstraint:
    """Algebraic constraint enforcing dissolved equilibrium.

    Constrains one species concentration so that the aqueous-phase
    equilibrium condition is satisfied.

    Args:
        phase_name: Name of the condensed phase.
        reactant_names: Names of reactant species.
        product_names: Names of product species.
        algebraic_species_name: Species whose concentration is constrained.
        solvent_name: Name of the solvent species.
        equilibrium_constant: Equilibrium constant.
        solvent_damping_epsilon: Regularization parameter to prevent
            singularity as solvent concentration approaches zero.
    """
    phase_name: str
    reactant_names: List[str]
    product_names: List[str]
    algebraic_species_name: str
    solvent_name: str
    equilibrium_constant: EquilibriumConstant
    solvent_damping_epsilon: float = 1.0e-10


@dataclass(frozen=True)
class LinearConstraint:
    """Linear algebraic constraint on species concentrations.

    Enforces: sum(coefficient_i * [species_i]) = constant,
    where the algebraic species is the one whose ODE is replaced
    by this algebraic equation.

    Args:
        algebraic_phase_name: Phase of the algebraically constrained species.
        algebraic_species_name: Species whose ODE is replaced.
        terms: List of ``LinearConstraintTerm``s.
        constant: Right-hand side constant [mol m-3]. Ignored when
            ``diagnose_from_state`` is True.
        diagnose_from_state: If True, compute the constant from the current
            state at the beginning of each solve step as
            C = sum(c_i * [species_i]).  This is useful for mass conservation
            constraints where the total varies by grid cell and may change
            between solve steps due to emissions, transport, or deposition.
    """
    algebraic_phase_name: str
    algebraic_species_name: str
    terms: List[LinearConstraintTerm]
    constant: float = 0.0
    diagnose_from_state: bool = False
