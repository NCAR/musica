# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Python wrappers for the aerosol configuration types defined in
# mechanism_configuration/types/aerosol.hpp.
#
# Design: like the gas-phase reaction wrappers (e.g. Arrhenius), these accept
# Species / Phase / representation objects and resolve them down to the names
# the C++ layer stores. Passing objects (rather than bare strings) lets typos be
# caught at construction time and keeps the aerosol API consistent with the rest
# of mechanism_configuration.
from typing import Any, Callable, Dict, List, Optional, Tuple, Union

from ... import backend
from ..._base import CppWrapper, CppField, _unwrap, _unwrap_list, _wrap_list
from ..species import Phase
from ..species import Species
from ..reactions import ReactionComponent
from ..utils import _convert_components

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration


def _name(obj) -> str:
    """Resolve an object with a ``name`` attribute (Species, Phase, ...) to its name.

    Strings pass through unchanged so callers may still supply bare names.
    """
    if obj is None:
        return ""
    return obj.name if hasattr(obj, "name") else obj


def _names(objs) -> List[str]:
    """Resolve a list of Phase/Species/str to a list of names."""
    return [_name(o) for o in (objs or [])]


# -- Rate constants ----------------------------------------------------------

RateConstantType = Union["Equilibrium", Callable[[float], float]]


class Equilibrium(CppWrapper):
    """Reference-temperature Arrhenius rate constant.

    f(T) = A * exp( C * (1/T0 - 1/T) )     (C = +Ea/R)

    Attributes:
        A: Value at the reference temperature T0 [units vary by use].
        C: Temperature-dependence parameter [K] (C = +Ea/R).
        T0: Reference temperature [K].
    """

    A = CppField()
    C = CppField()
    T0 = CppField()

    def __init__(
        self,
        A: Optional[float] = None,
        C: Optional[float] = None,
        T0: Optional[float] = None,
    ):
        self._cpp = _mc._Equilibrium()
        self.A = A if A is not None else self.A
        self.C = C if C is not None else self.C
        self.T0 = T0 if T0 is not None else self.T0


class HenryLawConstant(CppWrapper):
    """Henry's law constant: HLC(T) = HLC_ref * exp( C * (1/T - 1/T0) ).

    Attributes:
        HLC_ref: Reference HLC at T0 [mol m-3 Pa-1].
        C: Temperature-dependence parameter [K].
        T0: Reference temperature [K].
    """

    HLC_ref = CppField()
    C = CppField()
    T0 = CppField()

    def __init__(
        self,
        HLC_ref: Optional[float] = None,
        C: Optional[float] = None,
        T0: Optional[float] = None,
    ):
        self._cpp = _mc._HenryLawConstant()
        self.HLC_ref = HLC_ref if HLC_ref is not None else self.HLC_ref
        self.C = C if C is not None else self.C
        self.T0 = T0 if T0 is not None else self.T0


def _convert_rate_constant(rc):
    """Unwrap a rate constant (wrapper object) or pass a callable through."""
    return _unwrap(rc)


def _convert_rate_constant_map(mapping: Optional[Dict]) -> Dict:
    """Convert a {representation: rate_constant} mapping to {name: cpp_rate_constant}.

    Keys may be representation objects (resolved to their name) or strings. Each
    key must name one of the aerosol's declared representations; the model applies
    a single rate constant per reaction, so at most one entry is expected.
    """
    if not mapping:
        return {}
    return {_name(key): _convert_rate_constant(value) for key, value in mapping.items()}


# -- Representations ----------------------------------------------------------


class UniformSection(CppWrapper):
    """A uniform (sectional) aerosol representation.

    Attributes:
        name: The name of the representation.
        phases: Names of the phases carried by this section.
        min_radius: Minimum section radius [m].
        max_radius: Maximum section radius [m].
    """

    name = CppField()
    min_radius = CppField()
    max_radius = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        phases: Optional[List[Union[Phase, str]]] = None,
        min_radius: Optional[float] = None,
        max_radius: Optional[float] = None,
    ):
        self._cpp = _mc._UniformSection()
        self.name = name if name is not None else self.name
        self.phases = phases if phases is not None else []
        self.min_radius = min_radius if min_radius is not None else self.min_radius
        self.max_radius = max_radius if max_radius is not None else self.max_radius

    @property
    def phases(self) -> List[str]:
        return self._cpp.phases

    @phases.setter
    def phases(self, value):
        self._cpp.phases = _names(value)

    def set_default_parameters(self, state):
        """Set this section's default radius bounds on a MICM state (as rate parameters)."""
        state.set_user_defined_rate_parameters(
            {
                f"{self.name}.MIN_RADIUS": self.min_radius,
                f"{self.name}.MAX_RADIUS": self.max_radius,
            }
        )


class SingleMomentMode(CppWrapper):
    """A single-moment modal aerosol representation.

    Attributes:
        name: The name of the representation.
        phases: Names of the phases carried by this mode.
        geometric_mean_radius: Geometric mean radius [m].
        geometric_standard_deviation: Geometric standard deviation [-].
    """

    name = CppField()
    geometric_mean_radius = CppField()
    geometric_standard_deviation = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        phases: Optional[List[Union[Phase, str]]] = None,
        geometric_mean_radius: Optional[float] = None,
        geometric_standard_deviation: Optional[float] = None,
    ):
        self._cpp = _mc._SingleMomentMode()
        self.name = name if name is not None else self.name
        self.phases = phases if phases is not None else []
        self.geometric_mean_radius = (
            geometric_mean_radius if geometric_mean_radius is not None else self.geometric_mean_radius
        )
        self.geometric_standard_deviation = (
            geometric_standard_deviation
            if geometric_standard_deviation is not None
            else self.geometric_standard_deviation
        )

    @property
    def phases(self) -> List[str]:
        return self._cpp.phases

    @phases.setter
    def phases(self, value):
        self._cpp.phases = _names(value)

    def set_default_parameters(self, state):
        """Set this mode's default size parameters on a MICM state (as rate parameters)."""
        state.set_user_defined_rate_parameters(
            {
                f"{self.name}.GEOMETRIC_MEAN_RADIUS": self.geometric_mean_radius,
                f"{self.name}.GEOMETRIC_STANDARD_DEVIATION": self.geometric_standard_deviation,
            }
        )


class TwoMomentMode(CppWrapper):
    """A two-moment modal aerosol representation.

    Attributes:
        name: The name of the representation.
        phases: Names of the phases carried by this mode.
        geometric_standard_deviation: Geometric standard deviation [-].
    """

    name = CppField()
    geometric_standard_deviation = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        phases: Optional[List[Union[Phase, str]]] = None,
        geometric_standard_deviation: Optional[float] = None,
    ):
        self._cpp = _mc._TwoMomentMode()
        self.name = name if name is not None else self.name
        self.phases = phases if phases is not None else []
        self.geometric_standard_deviation = (
            geometric_standard_deviation
            if geometric_standard_deviation is not None
            else self.geometric_standard_deviation
        )

    @property
    def phases(self) -> List[str]:
        return self._cpp.phases

    @phases.setter
    def phases(self, value):
        self._cpp.phases = _names(value)

    def set_default_parameters(self, state):
        """Set this mode's default size parameter on a MICM state (as rate parameters)."""
        state.set_user_defined_rate_parameters(
            {
                f"{self.name}.GEOMETRIC_STANDARD_DEVIATION": self.geometric_standard_deviation,
            }
        )


ComponentType = Union[Species, Tuple[float, Species], ReactionComponent]


# -- Processes ----------------------------------------------------------------


class DissolvedReaction(CppWrapper):
    """Irreversible dissolved (aqueous-phase) reaction.

    Attributes:
        phase: Name of the condensed phase where the reaction occurs.
        solvent: Name of the solvent species.
        reactants: Reactant components.
        products: Product components.
        rate_constants: Rate constant keyed by aerosol representation name (e.g. the
            mechanism's ``UniformSection``). The model applies a single rate constant
            per reaction, so at most one entry is expected.
        solvent_floor: Floor added to the solvent concentration in the rate denominator [mol m-3].
        min_halflife: Minimum reactant half-life used to cap the rate [s].
    """

    phase = CppField()
    solvent = CppField()
    solvent_floor = CppField()
    min_halflife = CppField()

    def __init__(
        self,
        phase: Optional[Union[Phase, str]] = None,
        solvent: Optional[Union[Species, str]] = None,
        reactants: Optional[List[ComponentType]] = None,
        products: Optional[List[ComponentType]] = None,
        rate_constants: Optional[Dict[Any, RateConstantType]] = None,
        solvent_floor: Optional[float] = None,
        min_halflife: Optional[float] = None,
    ):
        self._cpp = _mc._DissolvedReaction()
        self.phase = _name(phase)
        self.solvent = _name(solvent)
        self.reactants = reactants if reactants is not None else []
        self.products = products if products is not None else []
        self.rate_constants = rate_constants
        if solvent_floor is not None:
            self.solvent_floor = solvent_floor
        if min_halflife is not None:
            self.min_halflife = min_halflife

    @property
    def reactants(self) -> List[ReactionComponent]:
        return _wrap_list(ReactionComponent, self._cpp.reactants)

    @reactants.setter
    def reactants(self, value):
        self._cpp.reactants = _unwrap_list(_convert_components(value))

    @property
    def products(self) -> List[ReactionComponent]:
        return _wrap_list(ReactionComponent, self._cpp.products)

    @products.setter
    def products(self, value):
        self._cpp.products = _unwrap_list(_convert_components(value))

    @property
    def rate_constants(self) -> Dict:
        return self._cpp.rate_constants

    @rate_constants.setter
    def rate_constants(self, value):
        self._cpp.rate_constants = _convert_rate_constant_map(value)


class DissolvedReversibleReaction(CppWrapper):
    """Reversible dissolved (aqueous-phase) reaction.

    Supply two of {forward, reverse, equilibrium} per representation; the third
    is derived by the model.

    Attributes:
        phase: Name of the condensed phase where the reaction occurs.
        solvent: Name of the solvent species.
        reactants: Reactant components.
        products: Product components.
        forward_rate_constants: Per-representation forward rate constants.
        reverse_rate_constants: Per-representation reverse rate constants.
        equilibrium_constant: Shared intrinsic equilibrium constant (not per representation).
        solvent_floor: Floor added to the solvent concentration in the rate denominator [mol m-3].
    """

    phase = CppField()
    solvent = CppField()
    solvent_floor = CppField()

    def __init__(
        self,
        phase: Optional[Union[Phase, str]] = None,
        solvent: Optional[Union[Species, str]] = None,
        reactants: Optional[List[ComponentType]] = None,
        products: Optional[List[ComponentType]] = None,
        forward_rate_constants: Optional[Dict[Any, RateConstantType]] = None,
        reverse_rate_constants: Optional[Dict[Any, RateConstantType]] = None,
        equilibrium_constant: Optional[Equilibrium] = None,
        solvent_floor: Optional[float] = None,
    ):
        self._cpp = _mc._DissolvedReversibleReaction()
        self.phase = _name(phase)
        self.solvent = _name(solvent)
        self.reactants = reactants if reactants is not None else []
        self.products = products if products is not None else []
        self.forward_rate_constants = forward_rate_constants
        self.reverse_rate_constants = reverse_rate_constants
        if equilibrium_constant is not None:
            self._cpp.equilibrium_constant = _unwrap(equilibrium_constant)
        if solvent_floor is not None:
            self.solvent_floor = solvent_floor

    @property
    def reactants(self) -> List[ReactionComponent]:
        return _wrap_list(ReactionComponent, self._cpp.reactants)

    @reactants.setter
    def reactants(self, value):
        self._cpp.reactants = _unwrap_list(_convert_components(value))

    @property
    def products(self) -> List[ReactionComponent]:
        return _wrap_list(ReactionComponent, self._cpp.products)

    @products.setter
    def products(self, value):
        self._cpp.products = _unwrap_list(_convert_components(value))

    @property
    def forward_rate_constants(self) -> Dict:
        return self._cpp.forward_rate_constants

    @forward_rate_constants.setter
    def forward_rate_constants(self, value):
        self._cpp.forward_rate_constants = _convert_rate_constant_map(value)

    @property
    def reverse_rate_constants(self) -> Dict:
        return self._cpp.reverse_rate_constants

    @reverse_rate_constants.setter
    def reverse_rate_constants(self, value):
        self._cpp.reverse_rate_constants = _convert_rate_constant_map(value)

    @property
    def equilibrium_constant(self) -> Optional[Equilibrium]:
        cpp = self._cpp.equilibrium_constant
        return Equilibrium._from_cpp(cpp) if cpp is not None else None

    @equilibrium_constant.setter
    def equilibrium_constant(self, value):
        self._cpp.equilibrium_constant = _unwrap(value) if value is not None else None


class HenryLawPhaseTransfer(CppWrapper):
    """Henry's law gas-to-aqueous phase transfer.

    Attributes:
        gas_phase: Name of the gas phase.
        gas_species: Name of the gas-phase species.
        condensed_phase: Name of the condensed phase receiving the species.
        condensed_species: Name of the dissolved species in the condensed phase.
        solvent: Name of the solvent species in the condensed phase.
        henry_law_constant: Henry's law constant with temperature dependence.
        diffusion_coefficient: Gas-phase diffusion coefficient [m2 s-1].
        accommodation_coefficient: Mass accommodation coefficient [-].
    """

    gas_phase = CppField()
    gas_species = CppField()
    condensed_phase = CppField()
    condensed_species = CppField()
    solvent = CppField()
    diffusion_coefficient = CppField()
    accommodation_coefficient = CppField()

    def __init__(
        self,
        gas_phase: Optional[Union[Phase, str]] = None,
        gas_species: Optional[Union[Species, str]] = None,
        condensed_phase: Optional[Union[Phase, str]] = None,
        condensed_species: Optional[Union[Species, str]] = None,
        solvent: Optional[Union[Species, str]] = None,
        henry_law_constant: Optional[HenryLawConstant] = None,
        diffusion_coefficient: Optional[float] = None,
        accommodation_coefficient: Optional[float] = None,
    ):
        self._cpp = _mc._HenryLawPhaseTransfer()
        self.gas_phase = _name(gas_phase)
        self.gas_species = _name(gas_species)
        self.condensed_phase = _name(condensed_phase)
        self.condensed_species = _name(condensed_species)
        self.solvent = _name(solvent)
        if henry_law_constant is not None:
            self._cpp.henry_law_constant = _unwrap(henry_law_constant)
        self.diffusion_coefficient = (
            diffusion_coefficient if diffusion_coefficient is not None else self.diffusion_coefficient
        )
        self.accommodation_coefficient = (
            accommodation_coefficient if accommodation_coefficient is not None else self.accommodation_coefficient
        )

    @property
    def henry_law_constant(self) -> HenryLawConstant:
        return HenryLawConstant._from_cpp(self._cpp.henry_law_constant)

    @henry_law_constant.setter
    def henry_law_constant(self, value):
        self._cpp.henry_law_constant = _unwrap(value)


# -- Constraints --------------------------------------------------------------


class HenryLawEquilibrium(CppWrapper):
    """Henry's law equilibrium constraint.

    Attributes:
        gas_phase: Name of the gas phase.
        gas_species: Name of the gas-phase species.
        condensed_phase: Name of the condensed phase.
        condensed_species: Name of the condensed-phase species.
        solvent: Name of the solvent species.
        henry_law_constant: Henry's law constant.
        solvent_molecular_weight: Solvent molecular weight [kg mol-1].
        solvent_density: Solvent density [kg m-3].
    """

    gas_phase = CppField()
    gas_species = CppField()
    condensed_phase = CppField()
    condensed_species = CppField()
    solvent = CppField()
    solvent_molecular_weight = CppField()
    solvent_density = CppField()

    def __init__(
        self,
        gas_phase: Optional[Union[Phase, str]] = None,
        gas_species: Optional[Union[Species, str]] = None,
        condensed_phase: Optional[Union[Phase, str]] = None,
        condensed_species: Optional[Union[Species, str]] = None,
        solvent: Optional[Union[Species, str]] = None,
        henry_law_constant: Optional[HenryLawConstant] = None,
        solvent_molecular_weight: Optional[float] = None,
        solvent_density: Optional[float] = None,
    ):
        self._cpp = _mc._HenryLawEquilibrium()
        self.gas_phase = _name(gas_phase)
        self.gas_species = _name(gas_species)
        self.condensed_phase = _name(condensed_phase)
        self.condensed_species = _name(condensed_species)
        self.solvent = _name(solvent)
        if henry_law_constant is not None:
            self._cpp.henry_law_constant = _unwrap(henry_law_constant)
        self.solvent_molecular_weight = (
            solvent_molecular_weight if solvent_molecular_weight is not None else self.solvent_molecular_weight
        )
        self.solvent_density = solvent_density if solvent_density is not None else self.solvent_density

    @property
    def henry_law_constant(self) -> HenryLawConstant:
        return HenryLawConstant._from_cpp(self._cpp.henry_law_constant)

    @henry_law_constant.setter
    def henry_law_constant(self, value):
        self._cpp.henry_law_constant = _unwrap(value)


class DissolvedEquilibrium(CppWrapper):
    """Dissolved (aqueous-phase) equilibrium constraint.

    Attributes:
        phase: Name of the condensed phase.
        algebraic_species: Name of the species solved for algebraically.
        solvent: Name of the solvent species.
        reactants: Reactant components.
        products: Product components.
        equilibrium_constant: Equilibrium constant.
        solvent_floor: Floor added to the solvent concentration in the rate denominator [mol m-3].
    """

    phase = CppField()
    algebraic_species = CppField()
    solvent = CppField()
    solvent_floor = CppField()

    def __init__(
        self,
        phase: Optional[Union[Phase, str]] = None,
        algebraic_species: Optional[Union[Species, str]] = None,
        solvent: Optional[Union[Species, str]] = None,
        reactants: Optional[List[ComponentType]] = None,
        products: Optional[List[ComponentType]] = None,
        equilibrium_constant: Optional[Equilibrium] = None,
        solvent_floor: Optional[float] = None,
    ):
        self._cpp = _mc._DissolvedEquilibrium()
        self.phase = _name(phase)
        self.algebraic_species = _name(algebraic_species)
        self.solvent = _name(solvent)
        self.reactants = reactants if reactants is not None else []
        self.products = products if products is not None else []
        if equilibrium_constant is not None:
            self._cpp.equilibrium_constant = _unwrap(equilibrium_constant)
        if solvent_floor is not None:
            self.solvent_floor = solvent_floor

    @property
    def reactants(self) -> List[ReactionComponent]:
        return _wrap_list(ReactionComponent, self._cpp.reactants)

    @reactants.setter
    def reactants(self, value):
        self._cpp.reactants = _unwrap_list(_convert_components(value))

    @property
    def products(self) -> List[ReactionComponent]:
        return _wrap_list(ReactionComponent, self._cpp.products)

    @products.setter
    def products(self, value):
        self._cpp.products = _unwrap_list(_convert_components(value))

    @property
    def equilibrium_constant(self) -> Equilibrium:
        return Equilibrium._from_cpp(self._cpp.equilibrium_constant)

    @equilibrium_constant.setter
    def equilibrium_constant(self, value):
        self._cpp.equilibrium_constant = _unwrap(value)


class LinearConstraintTerm(CppWrapper):
    """A single term of a LinearConstraint: coefficient * [species] in a phase.

    Attributes:
        phase: Name of the phase.
        species: Name of the species.
        coefficient: Term coefficient.
    """

    phase = CppField()
    coefficient = CppField()

    def __init__(
        self,
        phase: Optional[Union[Phase, str]] = None,
        species: Optional[Union[Species, str]] = None,
        coefficient: Optional[float] = None,
    ):
        self._cpp = _mc._LinearConstraintTerm()
        self.phase = _name(phase)
        self.species = _name(species)
        self.coefficient = coefficient if coefficient is not None else self.coefficient

    @property
    def species(self) -> str:
        # The C++ field is named `name`.
        return self._cpp.name

    @species.setter
    def species(self, value):
        self._cpp.name = _name(value)


class FixedConstant(CppWrapper):
    """Fixed RHS constant of a LinearConstraint, shared by all instances.

    Attributes:
        value: Fixed value [mol m-3].
    """

    value = CppField()

    def __init__(self, value: Optional[float] = None):
        self._cpp = _mc._FixedConstant()
        self.value = value if value is not None else self.value


class DiagnoseFromState(CppWrapper):
    """RHS constant of a LinearConstraint computed per representation instance."""

    def __init__(self):
        self._cpp = _mc._DiagnoseFromState()


class LinearConstraint(CppWrapper):
    """A linear algebraic constraint: G = sum(coeff_i * [species_i]) - C = 0.

    Attributes:
        algebraic_phase: Name of the phase of the algebraic species.
        algebraic_species: Name of the species solved for algebraically.
        terms: The constraint terms.
        constant: A FixedConstant or DiagnoseFromState (defaults to FixedConstant(0)).
    """

    algebraic_phase = CppField()
    algebraic_species = CppField()

    def __init__(
        self,
        algebraic_phase: Optional[Union[Phase, str]] = None,
        algebraic_species: Optional[Union[Species, str]] = None,
        terms: Optional[List[LinearConstraintTerm]] = None,
        constant: Optional[Union[FixedConstant, DiagnoseFromState]] = None,
    ):
        self._cpp = _mc._LinearConstraint()
        self.algebraic_phase = _name(algebraic_phase)
        self.algebraic_species = _name(algebraic_species)
        self.terms = terms if terms is not None else []
        if constant is not None:
            self._cpp.constant = _unwrap(constant)

    @property
    def terms(self) -> List[LinearConstraintTerm]:
        return _wrap_list(LinearConstraintTerm, self._cpp.terms)

    @terms.setter
    def terms(self, value):
        self._cpp.terms = _unwrap_list(value)

    @property
    def constant(self):
        return self._cpp.constant

    @constant.setter
    def constant(self, value):
        self._cpp.constant = _unwrap(value)


# -- Container ----------------------------------------------------------------

RepresentationType = Union[UniformSection, SingleMomentMode, TwoMomentMode]
ProcessType = Union[DissolvedReaction, DissolvedReversibleReaction, HenryLawPhaseTransfer]
ConstraintType = Union[HenryLawEquilibrium, DissolvedEquilibrium, LinearConstraint]

_REPRESENTATION_WRAPPERS = {
    _mc._UniformSection: UniformSection,
    _mc._SingleMomentMode: SingleMomentMode,
    _mc._TwoMomentMode: TwoMomentMode,
}
_PROCESS_WRAPPERS = {
    _mc._DissolvedReaction: DissolvedReaction,
    _mc._DissolvedReversibleReaction: DissolvedReversibleReaction,
    _mc._HenryLawPhaseTransfer: HenryLawPhaseTransfer,
}
_CONSTRAINT_WRAPPERS = {
    _mc._HenryLawEquilibrium: HenryLawEquilibrium,
    _mc._DissolvedEquilibrium: DissolvedEquilibrium,
    _mc._LinearConstraint: LinearConstraint,
}


def _wrap_variant_list(cpp_items, wrapper_map):
    """Wrap a list of C++ variant objects using the appropriate wrapper per type."""
    wrapped = []
    for item in cpp_items:
        wrapper = wrapper_map.get(type(item))
        wrapped.append(wrapper._from_cpp(item) if wrapper is not None else item)
    return wrapped


class Aerosol(CppWrapper):
    """Aerosol configuration attached to a Mechanism.

    Collects the representations, processes, and constraints that define an
    aerosol or cloud system. Species and phases are defined on the owning
    Mechanism; aerosol entries reference them by object (resolved to names).

    Attributes:
        representations: Aerosol/cloud representations.
        processes: Aqueous-phase processes.
        constraints: Algebraic equilibrium constraints.
    """

    def __init__(
        self,
        representations: Optional[List[RepresentationType]] = None,
        processes: Optional[List[ProcessType]] = None,
        constraints: Optional[List[ConstraintType]] = None,
    ):
        self._cpp = _mc._Aerosol()
        self.representations = representations if representations is not None else []
        self.processes = processes if processes is not None else []
        self.constraints = constraints if constraints is not None else []

    @property
    def representations(self) -> List[RepresentationType]:
        return _wrap_variant_list(self._cpp.representations, _REPRESENTATION_WRAPPERS)

    @representations.setter
    def representations(self, value):
        self._cpp.representations = _unwrap_list(value)

    @property
    def processes(self) -> List[ProcessType]:
        return _wrap_variant_list(self._cpp.processes, _PROCESS_WRAPPERS)

    @processes.setter
    def processes(self, value):
        self._cpp.processes = _unwrap_list(value)

    @property
    def constraints(self) -> List[ConstraintType]:
        return _wrap_variant_list(self._cpp.constraints, _CONSTRAINT_WRAPPERS)

    @constraints.setter
    def constraints(self, value):
        self._cpp.constraints = _unwrap_list(value)

    def set_default_parameters(self, state):
        """Set each representation's default parameters on a MICM state.

        Delegates to every representation's ``set_default_parameters``.

        Args:
            state: A ``musica.micm.State`` object.
        """
        for representation in self.representations:
            representation.set_default_parameters(state)
