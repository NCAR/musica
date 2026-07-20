# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Unit tests for the aerosol configuration types in musica.mechanism_configuration
# (constants, representations, processes, constraints, Aerosol container) —
# no solver needed.

import math
import pytest

from musica.mechanism_configuration import (
    Aerosol,
    Arrhenius,
    Equilibrium,
    DiagnoseFromState,
    DissolvedEquilibrium,
    DissolvedReaction,
    DissolvedReversibleReaction,
    FixedConstant,
    HenryLawConstant,
    HenryLawEquilibrium,
    HenryLawPhaseTransfer,
    LinearConstraint,
    LinearConstraintTerm,
    Phase,
    SingleMomentMode,
    Species,
    TwoMomentMode,
    UniformSection,
)


class FakeState:
    """Collects user-defined rate parameters like a MICM state."""

    def __init__(self):
        self._params = {}

    def set_user_defined_rate_parameters(self, params):
        self._params.update(params)


# ═══ Constants ═══════════════════════════════════════════════════════════════


class TestHenryLawConstant:
    def test_construction(self):
        hlc = HenryLawConstant(HLC_ref=1.23, C=3120.0)
        assert hlc.HLC_ref == 1.23
        assert hlc.C == 3120.0

    def test_default_c_and_t0(self):
        hlc = HenryLawConstant(HLC_ref=1.0)
        assert hlc.C == 0.0
        assert hlc.T0 == pytest.approx(298.15)

    def test_mutable(self):
        hlc = HenryLawConstant(HLC_ref=1.0, C=0.0)
        hlc.HLC_ref = 2.0
        assert hlc.HLC_ref == 2.0


class TestEquilibrium:
    """Covers rate constants and equilibrium constants: f(T) = A * exp(C*(1/T0 - 1/T))."""

    def test_construction(self):
        art = Equilibrium(A=7.45e7, C=4430.0)
        assert art.A == 7.45e7
        assert art.C == 4430.0

    def test_default_c_and_t0(self):
        art = Equilibrium(A=1.0)
        assert art.C == 0.0
        assert art.T0 == pytest.approx(298.15)

    def test_mutable(self):
        art = Equilibrium(A=1.0)
        art.C = 99.0
        assert art.C == 99.0


# ═══ Representations ═════════════════════════════════════════════════════════


class TestUniformSection:
    def test_construction(self):
        aq = Phase(name="AQUEOUS")
        us = UniformSection(name="CLOUD", phases=[aq],
                            min_radius=1e-6, max_radius=1e-5)
        assert us.name == "CLOUD"
        assert us.phases == ["AQUEOUS"]
        assert us.min_radius == 1e-6
        assert us.max_radius == 1e-5

    def test_phases_accept_names(self):
        us = UniformSection(name="CLOUD", phases=["AQUEOUS"],
                            min_radius=1e-6, max_radius=1e-5)
        assert us.phases == ["AQUEOUS"]

    def test_set_default_parameters(self):
        us = UniformSection(name="CLOUD", phases=["AQUEOUS"],
                            min_radius=2e-6, max_radius=3e-5)
        state = FakeState()
        us.set_default_parameters(state)
        assert state._params["CLOUD.MIN_RADIUS"] == 2e-6
        assert state._params["CLOUD.MAX_RADIUS"] == 3e-5


class TestSingleMomentMode:
    def test_construction(self):
        smm = SingleMomentMode(
            name="AEROSOL", phases=["AQ"],
            geometric_mean_radius=1e-7, geometric_standard_deviation=1.5,
        )
        assert smm.name == "AEROSOL"
        assert smm.geometric_mean_radius == 1e-7
        assert smm.geometric_standard_deviation == 1.5

    def test_set_default_parameters(self):
        smm = SingleMomentMode(
            name="MODE1", phases=["AQ"],
            geometric_mean_radius=1e-7, geometric_standard_deviation=1.8,
        )
        state = FakeState()
        smm.set_default_parameters(state)
        assert state._params["MODE1.GEOMETRIC_MEAN_RADIUS"] == 1e-7
        assert state._params["MODE1.GEOMETRIC_STANDARD_DEVIATION"] == 1.8


class TestTwoMomentMode:
    def test_construction(self):
        tmm = TwoMomentMode(
            name="AEROSOL", phases=["AQ"],
            geometric_standard_deviation=2.0,
        )
        assert tmm.name == "AEROSOL"
        assert tmm.geometric_standard_deviation == 2.0

    def test_set_default_parameters(self):
        tmm = TwoMomentMode(
            name="MODE2", phases=["AQ"],
            geometric_standard_deviation=1.6,
        )
        state = FakeState()
        tmm.set_default_parameters(state)
        assert state._params["MODE2.GEOMETRIC_STANDARD_DEVIATION"] == 1.6


# ═══ Processes ═══════════════════════════════════════════════════════════════


class TestDissolvedReaction:
    def test_with_arrhenius(self):
        hso3m = Species(name="HSO3m")
        h2o2_aq = Species(name="H2O2_aq")
        so4mm = Species(name="SO4mm")
        h2o = Species(name="H2O")
        hp = Species(name="Hp")
        aq = Phase(name="AQUEOUS")
        rxn = DissolvedReaction(
            phase=aq,
            reactants=[hso3m, h2o2_aq],
            products=[so4mm, h2o, hp],
            solvent=h2o,
            rate_constants=Equilibrium(A=7.45e7, C=4430.0),
        )
        assert rxn.phase == "AQUEOUS"
        assert len(rxn.reactants) == 2
        assert len(rxn.products) == 3
        assert rxn.rate_constants is not None

    def test_with_callable(self):
        def k_func(T):
            return 55.556 * 7.45e7 * math.exp(-4430.0 * (1.0 / T - 1.0 / 298.15))

        rxn = DissolvedReaction(
            phase="AQUEOUS",
            reactants=[Species(name="HSO3m"), Species(name="H2O2_aq")],
            products=[Species(name="SO4mm")],
            solvent="H2O",
            rate_constants=k_func,
        )
        rc = rxn.rate_constants
        assert callable(rc)
        # Verify the callable works
        assert rc(298.15) == pytest.approx(55.556 * 7.45e7, rel=1e-10)

    def test_solvent_floor_and_min_halflife(self):
        rxn = DissolvedReaction(
            phase="AQUEOUS",
            reactants=[Species(name="A")],
            products=[Species(name="B")],
            solvent="H2O",
            rate_constants=Equilibrium(A=1.0),
            solvent_floor=1e-18,
            min_halflife=1.0,
        )
        assert rxn.solvent_floor == 1e-18
        assert rxn.min_halflife == 1.0

    def test_defaults_unset(self):
        rxn = DissolvedReaction(phase="AQUEOUS", solvent="H2O")
        assert rxn.solvent_floor is None
        assert rxn.min_halflife is None


class TestDissolvedReversibleReaction:
    def test_with_forward_and_equilibrium(self):
        rxn = DissolvedReversibleReaction(
            phase="AQUEOUS",
            reactants=[Species(name="SO2_aq")],
            products=[Species(name="HSO3m"), Species(name="Hp")],
            solvent="H2O",
            forward_rate_constants=Arrhenius(A=1e6, C=0.0),
            equilibrium_constant=Equilibrium(A=1.7e-2, C=2090.0),
        )
        assert rxn.forward_rate_constants is not None
        assert rxn.reverse_rate_constants is None
        assert rxn.equilibrium_constant is not None

    def test_with_forward_and_reverse(self):
        rxn = DissolvedReversibleReaction(
            phase="AQUEOUS",
            reactants=[Species(name="A")],
            products=[Species(name="B")],
            solvent="H2O",
            forward_rate_constants=Arrhenius(A=100.0, C=0.0),
            reverse_rate_constants=Arrhenius(A=50.0, C=0.0),
        )
        assert rxn.forward_rate_constants is not None
        assert rxn.reverse_rate_constants is not None

    def test_defaults_are_empty(self):
        rxn = DissolvedReversibleReaction(
            phase="AQUEOUS",
            reactants=[Species(name="A")],
            products=[Species(name="B")],
            solvent="H2O",
        )
        assert rxn.forward_rate_constants is None
        assert rxn.reverse_rate_constants is None

    def test_solvent_floor(self):
        rxn = DissolvedReversibleReaction(
            phase="AQUEOUS", solvent="H2O", solvent_floor=1e-19,
        )
        assert rxn.solvent_floor == 1e-19


class TestHenryLawPhaseTransfer:
    def test_construction(self):
        hlpt = HenryLawPhaseTransfer(
            gas_phase="gas",
            gas_species="SO2",
            condensed_phase="AQUEOUS",
            condensed_species="SO2_aq",
            solvent="H2O",
            henry_law_constant=HenryLawConstant(HLC_ref=1.23e-2, C=3120.0),
            diffusion_coefficient=1.28e-5,
            accommodation_coefficient=0.11,
        )
        assert hlpt.condensed_phase == "AQUEOUS"
        assert hlpt.gas_species == "SO2"
        assert hlpt.diffusion_coefficient == 1.28e-5
        assert hlpt.accommodation_coefficient == 0.11


# ═══ Constraints ═════════════════════════════════════════════════════════════


class TestLinearConstraintTerm:
    def test_construction(self):
        term = LinearConstraintTerm("AQUEOUS", "SO4mm", 1.0)
        assert term.phase == "AQUEOUS"
        assert term.species == "SO4mm"
        assert term.coefficient == 1.0

    def test_accepts_objects(self):
        aq = Phase(name="AQUEOUS")
        so4 = Species(name="SO4mm")
        term = LinearConstraintTerm(aq, so4, -2.0)
        assert term.phase == "AQUEOUS"
        assert term.species == "SO4mm"
        assert term.coefficient == -2.0


class TestHenryLawEquilibrium:
    def test_construction(self):
        hlec = HenryLawEquilibrium(
            gas_phase="gas",
            gas_species="SO2",
            condensed_phase="AQUEOUS",
            condensed_species="SO2_aq",
            solvent="H2O",
            henry_law_constant=HenryLawConstant(HLC_ref=1.23e-2, C=3120.0),
            solvent_molecular_weight=0.018,
            solvent_density=1000.0,
        )
        assert hlec.gas_species == "SO2"
        assert hlec.solvent_molecular_weight == 0.018
        assert hlec.solvent_density == 1000.0


class TestDissolvedEquilibrium:
    def test_construction(self):
        dec = DissolvedEquilibrium(
            phase="AQUEOUS",
            reactants=[Species(name="SO2_aq")],
            products=[Species(name="HSO3m"), Species(name="Hp")],
            algebraic_species="SO2_aq",
            solvent="H2O",
            equilibrium_constant=Equilibrium(A=1.7e-2, C=2090.0),
        )
        assert dec.phase == "AQUEOUS"
        assert dec.algebraic_species == "SO2_aq"
        assert len(dec.reactants) == 1
        assert len(dec.products) == 2

    def test_solvent_floor(self):
        dec = DissolvedEquilibrium(
            phase="AQ", solvent="W",
            equilibrium_constant=Equilibrium(A=1.0),
            solvent_floor=1e-20,
        )
        assert dec.solvent_floor == 1e-20


class TestLinearConstraint:
    def test_construction_fixed_constant(self):
        lc = LinearConstraint(
            algebraic_phase="gas",
            algebraic_species="SO2",
            terms=[
                LinearConstraintTerm("gas", "SO2", 1.0),
                LinearConstraintTerm("AQUEOUS", "SO2_aq", 1.0),
                LinearConstraintTerm("AQUEOUS", "HSO3m", 1.0),
            ],
            constant=FixedConstant(3e-8),
        )
        assert lc.algebraic_phase == "gas"
        assert lc.algebraic_species == "SO2"
        assert len(lc.terms) == 3
        assert lc.constant.value == 3e-8

    def test_diagnose_from_state(self):
        lc = LinearConstraint(
            algebraic_phase="gas",
            algebraic_species="X",
            terms=[LinearConstraintTerm("gas", "X", 1.0)],
            constant=DiagnoseFromState(),
        )
        assert "DiagnoseFromState" in type(lc.constant).__name__

    def test_default_constant_is_fixed_zero(self):
        lc = LinearConstraint(
            algebraic_phase="gas",
            algebraic_species="X",
            terms=[LinearConstraintTerm("gas", "X", 1.0)],
        )
        assert lc.constant.value == 0.0


# ═══ Aerosol container ═══════════════════════════════════════════════════════


class TestAerosol:
    def _make_minimal_aerosol(self):
        """Create a minimal valid aerosol section for testing."""
        aq_phase = Phase(name="AQUEOUS", species=[Species(name="H2O"), Species(name="SO2_aq")])
        return Aerosol(
            representations=[
                UniformSection(name="CLOUD", phases=[aq_phase], min_radius=1e-6, max_radius=1e-5)
            ],
        )

    def test_construction(self):
        aerosol = self._make_minimal_aerosol()
        assert len(aerosol.representations) == 1
        assert len(aerosol.processes) == 0
        assert len(aerosol.constraints) == 0

    def test_set_default_parameters(self):
        aerosol = self._make_minimal_aerosol()
        state = FakeState()
        aerosol.set_default_parameters(state)
        assert "CLOUD.MIN_RADIUS" in state._params
        assert "CLOUD.MAX_RADIUS" in state._params

    def test_set_default_parameters_multiple_representations(self):
        aerosol = Aerosol(
            representations=[
                UniformSection(name="SEC1", phases=["P"], min_radius=1e-7, max_radius=1e-6),
                SingleMomentMode(name="MODE1", phases=["P"],
                                 geometric_mean_radius=1e-7, geometric_standard_deviation=1.5),
                TwoMomentMode(name="MODE2", phases=["P"], geometric_standard_deviation=2.0),
            ],
        )
        state = FakeState()
        aerosol.set_default_parameters(state)
        assert "SEC1.MIN_RADIUS" in state._params
        assert "SEC1.MAX_RADIUS" in state._params
        assert "MODE1.GEOMETRIC_MEAN_RADIUS" in state._params
        assert "MODE1.GEOMETRIC_STANDARD_DEVIATION" in state._params
        assert "MODE2.GEOMETRIC_STANDARD_DEVIATION" in state._params

    def test_empty_aerosol(self):
        aerosol = Aerosol()
        assert len(aerosol.representations) == 0
        assert len(aerosol.processes) == 0
        assert len(aerosol.constraints) == 0

    def test_aerosol_with_all_process_types(self):
        aerosol = Aerosol(
            representations=[UniformSection(name="SEC", phases=["AQ"],
                                            min_radius=1e-6, max_radius=1e-5)],
            processes=[
                DissolvedReaction(
                    phase="AQ",
                    reactants=[Species(name="A")],
                    products=[Species(name="B")],
                    solvent="W",
                    rate_constants=Equilibrium(A=1.0, C=0.0),
                ),
                DissolvedReversibleReaction(
                    phase="AQ",
                    reactants=[Species(name="A")],
                    products=[Species(name="B")],
                    solvent="W",
                    equilibrium_constant=Equilibrium(A=1.0),
                ),
                HenryLawPhaseTransfer(
                    gas_phase="gas",
                    gas_species="A",
                    condensed_phase="AQ",
                    condensed_species="B",
                    solvent="W",
                    henry_law_constant=HenryLawConstant(HLC_ref=1.0),
                    diffusion_coefficient=1e-5,
                    accommodation_coefficient=0.1,
                ),
            ],
        )
        assert len(aerosol.processes) == 3

    def test_aerosol_with_all_constraint_types(self):
        aerosol = Aerosol(
            representations=[UniformSection(name="SEC", phases=["AQ"],
                                            min_radius=1e-6, max_radius=1e-5)],
            constraints=[
                HenryLawEquilibrium(
                    gas_phase="gas",
                    gas_species="A",
                    condensed_phase="AQ",
                    condensed_species="B",
                    solvent="W",
                    henry_law_constant=HenryLawConstant(HLC_ref=1.0, C=100.0),
                    solvent_molecular_weight=0.018,
                    solvent_density=1000.0,
                ),
                DissolvedEquilibrium(
                    phase="AQ",
                    reactants=[Species(name="A")],
                    products=[Species(name="B")],
                    algebraic_species="A",
                    solvent="W",
                    equilibrium_constant=Equilibrium(A=1.0),
                ),
                LinearConstraint(
                    algebraic_phase="AQ",
                    algebraic_species="A",
                    terms=[
                        LinearConstraintTerm("AQ", "A", 1.0),
                        LinearConstraintTerm("AQ", "B", 1.0),
                    ],
                    constant=FixedConstant(1e-6),
                ),
            ],
        )
        assert len(aerosol.constraints) == 3


# ═══ Serialization ═══════════════════════════════════════════════════════════


class TestRateConstantSerialize:
    def test_equilibrium(self):
        assert Equilibrium(A=1725.0, C=0.0, T0=298.15).serialize() == {
            "type": "EQUILIBRIUM",
            "A": 1725.0,
            "C [K]": 0.0,
            "T0 [K]": 298.15,
        }

    def test_henry_law_constant(self):
        assert HenryLawConstant(HLC_ref=1e-2, C=3000.0).serialize() == {
            "HLC_ref [mol m-3 Pa-1]": 1e-2,
            "C [K]": 3000.0,
            "T0 [K]": pytest.approx(298.15),
        }


class TestRepresentationSerialize:
    def test_uniform_section(self):
        assert UniformSection(name="CLOUD", phases=["AQ"], min_radius=1e-6, max_radius=1e-5).serialize() == {
            "type": "UNIFORM_SECTION",
            "name": "CLOUD",
            "phases": ["AQ"],
            "minimum radius [m]": 1e-6,
            "maximum radius [m]": 1e-5,
        }

    def test_single_moment_mode(self):
        assert SingleMomentMode(
            name="aitken", phases=["AQ"], geometric_mean_radius=1e-6, geometric_standard_deviation=1.5
        ).serialize() == {
            "type": "SINGLE_MOMENT_MODE",
            "name": "aitken",
            "phases": ["AQ"],
            "geometric mean radius [m]": 1e-6,
            "geometric standard deviation": 1.5,
        }

    def test_two_moment_mode(self):
        assert TwoMomentMode(name="fine", phases=["AQ"], geometric_standard_deviation=1.6).serialize() == {
            "type": "TWO_MOMENT_MODE",
            "name": "fine",
            "phases": ["AQ"],
            "geometric standard deviation": 1.6,
        }


class TestProcessSerialize:
    def test_dissolved_reaction_with_arrhenius(self):
        d = DissolvedReaction(
            phase="AQ", solvent="H2O",
            reactants=[Species(name="A")], products=[Species(name="B")],
            rate_constants=Arrhenius(A=1e3, C=100.0),
        ).serialize()
        assert d["type"] == "DISSOLVED_REACTION"
        assert d["condensed phase"] == "AQ"
        assert d["solvent"] == "H2O"
        assert d["reactants"] == [{"name": "A", "coefficient": 1.0}]
        assert d["products"] == [{"name": "B", "coefficient": 1.0}]
        assert d["rate constants"] == {"type": "ARRHENIUS", "A": 1e3, "C": 100.0}
        # Internal-only fields have no configuration key and must not be emitted.
        assert "solvent floor" not in d and "min halflife" not in d

    def test_dissolved_reaction_callable_raises(self):
        rxn = DissolvedReaction(phase="AQ", solvent="H2O", rate_constants=lambda T: 1.0)
        with pytest.raises(TypeError):
            rxn.serialize()

    def test_dissolved_reversible_reaction(self):
        d = DissolvedReversibleReaction(
            phase="AQ", solvent="H2O",
            reactants=[Species(name="A")], products=[Species(name="B")],
            forward_rate_constants=Arrhenius(A=1e3, C=100.0),
            equilibrium_constant=Equilibrium(A=1725.0),
        ).serialize()
        assert d["type"] == "DISSOLVED_REVERSIBLE_REACTION"
        assert d["forward rate constants"] == {"type": "ARRHENIUS", "A": 1e3, "C": 100.0}
        assert d["equilibrium constant"]["type"] == "EQUILIBRIUM"
        # Unset optional rate constants are omitted entirely.
        assert "reverse rate constants" not in d

    def test_henry_law_phase_transfer(self):
        d = HenryLawPhaseTransfer(
            gas_phase="gas", gas_species="A", condensed_phase="AQ",
            condensed_species="A", solvent="H2O",
            henry_law_constant=HenryLawConstant(HLC_ref=1e-2, C=3000.0),
            diffusion_coefficient=1.5e-5, accommodation_coefficient=0.1,
        ).serialize()
        assert d["type"] == "HENRY_LAW_PHASE_TRANSFER"
        assert d["gas-phase species"] == "A"
        assert d["condensed-phase species"] == "A"
        assert d["accommodation coefficient"] == 0.1
        assert d["Henry's law constant"]["HLC_ref [mol m-3 Pa-1]"] == 1e-2
        # Diffusion coefficient is derived from the species definition, not serialized.
        assert "diffusion coefficient [m2 s-1]" not in d


class TestConstraintSerialize:
    def test_henry_law_equilibrium(self):
        d = HenryLawEquilibrium(
            gas_phase="gas", gas_species="A", condensed_phase="AQ",
            condensed_species="A", solvent="H2O",
            henry_law_constant=HenryLawConstant(HLC_ref=1e-2, C=3000.0),
            solvent_molecular_weight=0.018, solvent_density=1000.0,
        ).serialize()
        assert d["type"] == "HENRY_LAW_EQUILIBRIUM"
        assert d["gas-phase species"] == "A"
        # Solvent molecular weight/density are derived from the species definition.
        assert "solvent molecular weight [kg mol-1]" not in d
        assert "solvent density [kg m-3]" not in d

    def test_dissolved_equilibrium(self):
        d = DissolvedEquilibrium(
            phase="AQ", solvent="H2O",
            reactants=[Species(name="A")], products=[Species(name="B")],
            algebraic_species="A", equilibrium_constant=Equilibrium(A=1.0),
        ).serialize()
        assert d["type"] == "DISSOLVED_EQUILIBRIUM"
        assert d["condensed phase"] == "AQ"
        assert d["algebraic species"] == "A"
        assert d["equilibrium constant"]["type"] == "EQUILIBRIUM"

    def test_linear_constraint_term(self):
        assert LinearConstraintTerm("AQ", "SO4mm", 2.0).serialize() == {
            "phase": "AQ",
            "name": "SO4mm",
            "coefficient": 2.0,
        }

    def test_linear_constraint_fixed_constant(self):
        d = LinearConstraint(
            algebraic_phase="AQ", algebraic_species="A",
            terms=[LinearConstraintTerm("AQ", "A", 1.0)],
            constant=FixedConstant(3e-8),
        ).serialize()
        assert d["type"] == "LINEAR_CONSTRAINT"
        assert d["constant [mol m-3]"] == 3e-8
        assert "diagnose from state" not in d
        assert d["terms"] == [{"phase": "AQ", "name": "A", "coefficient": 1.0}]

    def test_linear_constraint_diagnose_from_state(self):
        d = LinearConstraint(
            algebraic_phase="AQ", algebraic_species="A",
            terms=[LinearConstraintTerm("AQ", "A", 1.0)],
            constant=DiagnoseFromState(),
        ).serialize()
        assert d["diagnose from state"] is True
        assert "constant [mol m-3]" not in d


class TestAerosolContainerSerialize:
    def test_processes_and_constraints_share_processes_section(self):
        aerosol = Aerosol(
            representations=[
                SingleMomentMode(name="aitken", phases=["AQ"],
                                 geometric_mean_radius=1e-6, geometric_standard_deviation=1.5),
            ],
            processes=[
                DissolvedReaction(phase="AQ", solvent="H2O",
                                  reactants=[Species(name="A")], products=[Species(name="B")],
                                  rate_constants=Arrhenius(A=1e3, C=100.0)),
            ],
            constraints=[
                LinearConstraint(algebraic_phase="AQ", algebraic_species="A",
                                 terms=[LinearConstraintTerm("AQ", "A", 1.0)]),
            ],
        )
        d = aerosol.serialize()
        assert set(d.keys()) == {"aerosol representations", "aerosol processes"}
        assert len(d["aerosol representations"]) == 1
        # Processes come first, then constraints, in the shared section.
        assert len(d["aerosol processes"]) == 2
        assert d["aerosol processes"][0]["type"] == "DISSOLVED_REACTION"
        assert d["aerosol processes"][1]["type"] == "LINEAR_CONSTRAINT"
