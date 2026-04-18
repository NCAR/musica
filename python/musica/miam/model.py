"""MIAM Model — the top-level aerosol/cloud model configuration."""

from dataclasses import dataclass, field
from typing import List, Union

from .constants import ArrheniusRateConstant, EquilibriumConstant, HenrysLawConstant
from .representations import UniformSection, SingleMomentMode, TwoMomentMode
from .processes import DissolvedReaction, DissolvedReversibleReaction, HenryLawPhaseTransfer
from .constraints import (
    HenryLawEquilibriumConstraint,
    DissolvedEquilibriumConstraint,
    LinearConstraint,
    LinearConstraintTerm,
)
from ..mechanism_configuration import Species, Phase

RepresentationType = Union[UniformSection, SingleMomentMode, TwoMomentMode]
ProcessType = Union[DissolvedReaction, DissolvedReversibleReaction, HenryLawPhaseTransfer]
ConstraintType = Union[
    HenryLawEquilibriumConstraint, DissolvedEquilibriumConstraint, LinearConstraint
]


@dataclass
class Model:
    """MIAM aerosol/cloud model configuration.

    Collects representations, processes, and constraints that define
    an aerosol or cloud system.  Pass as an external model to
    ``MICM(external_models=[model])``.

    Args:
        name: Model name (e.g. "cloud_chemistry").
        species: All species referenced by this model (gas + condensed).
        condensed_phases: Condensed-phase definitions.
        representations: Aerosol/cloud representations.
        processes: Aqueous-phase processes.
        constraints: Algebraic equilibrium constraints.
    """
    name: str
    species: List[Species] = field(default_factory=list)
    condensed_phases: List[Phase] = field(default_factory=list)
    representations: List[RepresentationType] = field(default_factory=list)
    processes: List[ProcessType] = field(default_factory=list)
    constraints: List[ConstraintType] = field(default_factory=list)

    def set_default_parameters(self, state):
        """Set default representation parameters on a MICM state.

        Delegates to each representation's ``set_default_parameters``.

        Args:
            state: A ``musica.micm.State`` object.
        """
        for repr_ in self.representations:
            repr_.set_default_parameters(state)

    def _to_config(self):
        """Convert to the pybind11 ``_ModelConfig`` struct.

        Returns:
            A ``_musica._miam._ModelConfig`` instance.
        """
        from .._musica import _miam as m

        config = m._ModelConfig()
        config.name = self.name

        # Species
        config.species = [
            m._SpeciesDef(
                name=s.name,
                molecular_weight=s.molecular_weight_kg_mol if s.molecular_weight_kg_mol is not None else None,
                density=s.density_kg_m3,
            )
            for s in self.species
        ]

        # Condensed phases
        config.condensed_phases = [
            m._PhaseDef(name=p.name, species_names=[s.name for s in p.species])
            for p in self.condensed_phases
        ]

        # Representations
        repr_list = []
        for r in self.representations:
            if isinstance(r, UniformSection):
                repr_list.append(
                    m._UniformSection(r.name, r.phase_names, r.min_radius, r.max_radius)
                )
            elif isinstance(r, SingleMomentMode):
                repr_list.append(
                    m._SingleMomentMode(
                        r.name,
                        r.phase_names,
                        r.geometric_mean_radius,
                        r.geometric_standard_deviation,
                    )
                )
            elif isinstance(r, TwoMomentMode):
                repr_list.append(
                    m._TwoMomentMode(
                        r.name, r.phase_names, r.geometric_standard_deviation
                    )
                )
        config.representations = repr_list

        # Processes
        proc_list = []
        for p in self.processes:
            if isinstance(p, DissolvedReaction):
                proc_list.append(
                    m._DissolvedReaction(
                        p.phase_name,
                        p.reactant_names,
                        p.product_names,
                        p.solvent_name,
                        _convert_rate_constant(m, p.rate_constant),
                        solvent_damping_epsilon=p.solvent_damping_epsilon,
                    )
                )
            elif isinstance(p, DissolvedReversibleReaction):
                proc_list.append(
                    m._DissolvedReversibleReaction(
                        p.phase_name,
                        p.reactant_names,
                        p.product_names,
                        p.solvent_name,
                        forward_rate_constant=(
                            _convert_rate_constant(m, p.forward_rate_constant)
                            if p.forward_rate_constant is not None
                            else None
                        ),
                        reverse_rate_constant=(
                            _convert_rate_constant(m, p.reverse_rate_constant)
                            if p.reverse_rate_constant is not None
                            else None
                        ),
                        equilibrium_constant=(
                            m._EquilibriumConstant(p.equilibrium_constant.a, p.equilibrium_constant.c)
                            if p.equilibrium_constant is not None
                            else None
                        ),
                        solvent_damping_epsilon=p.solvent_damping_epsilon,
                    )
                )
            elif isinstance(p, HenryLawPhaseTransfer):
                proc_list.append(
                    m._HenryLawPhaseTransfer(
                        p.condensed_phase_name,
                        p.gas_species_name,
                        p.condensed_species_name,
                        p.solvent_name,
                        m._HenrysLawConstant(
                            p.henrys_law_constant.hlc_ref, p.henrys_law_constant.c
                        ),
                        p.diffusion_coefficient,
                        p.accommodation_coefficient,
                    )
                )
        config.processes = proc_list

        # Constraints
        con_list = []
        for c in self.constraints:
            if isinstance(c, HenryLawEquilibriumConstraint):
                con_list.append(
                    m._HenryLawEquilibriumConstraint(
                        c.gas_species_name,
                        c.condensed_species_name,
                        c.solvent_name,
                        c.condensed_phase_name,
                        m._HenrysLawConstant(
                            c.henrys_law_constant.hlc_ref, c.henrys_law_constant.c
                        ),
                        c.mw_solvent,
                        c.rho_solvent,
                    )
                )
            elif isinstance(c, DissolvedEquilibriumConstraint):
                con_list.append(
                    m._DissolvedEquilibriumConstraint(
                        c.phase_name,
                        c.reactant_names,
                        c.product_names,
                        c.algebraic_species_name,
                        c.solvent_name,
                        m._EquilibriumConstant(
                            c.equilibrium_constant.a, c.equilibrium_constant.c
                        ),
                        solvent_damping_epsilon=c.solvent_damping_epsilon,
                    )
                )
            elif isinstance(c, LinearConstraint):
                terms = [
                    m._LinearConstraintTerm(t.phase_name, t.species_name, t.coefficient)
                    for t in c.terms
                ]
                con_list.append(
                    m._LinearConstraint(
                        c.algebraic_phase_name,
                        c.algebraic_species_name,
                        terms,
                        c.constant,
                        c.diagnose_from_state,
                    )
                )
        config.constraints = con_list

        return config


def _convert_rate_constant(m, rc):
    """Convert a Python rate constant to the pybind11 type."""
    if isinstance(rc, ArrheniusRateConstant):
        return m._ArrheniusRateConstant(rc.a, rc.c)
    else:
        # callable f(T) -> k
        return rc
