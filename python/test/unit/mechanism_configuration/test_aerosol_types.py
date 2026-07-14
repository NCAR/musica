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
    ArrheniusReferenceTemperature,
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


class TestArrheniusReferenceTemperature:
    """Covers rate constants and equilibrium constants: f(T) = A * exp(C*(1/T0 - 1/T))."""

    def test_construction(self):
        art = ArrheniusReferenceTemperature(A=7.45e7, C=4430.0)
        assert art.A == 7.45e7
        assert art.C == 4430.0

    def test_default_c_and_t0(self):
        art = ArrheniusReferenceTemperature(A=1.0)
        assert art.C == 0.0
        assert art.T0 == pytest.approx(298.15)

    def test_mutable(self):
        art = ArrheniusReferenceTemperature(A=1.0)
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
        cloud = UniformSection(name="CLOUD", phases=[aq],
                               min_radius=1e-6, max_radius=1e-5)
        rxn = DissolvedReaction(
            phase=aq,
            reactants=[hso3m, h2o2_aq],
            products=[so4mm, h2o, hp],
            solvent=h2o,
            rate_constants={cloud: ArrheniusReferenceTemperature(A=7.45e7, C=4430.0)},
        )
        assert rxn.phase == "AQUEOUS"
        assert len(rxn.reactants) == 2
        assert len(rxn.products) == 3
        assert "CLOUD" in rxn.rate_constants

    def test_with_callable(self):
        def k_func(T):
            return 55.556 * 7.45e7 * math.exp(-4430.0 * (1.0 / T - 1.0 / 298.15))

        rxn = DissolvedReaction(
            phase="AQUEOUS",
            reactants=[Species(name="HSO3m"), Species(name="H2O2_aq")],
            products=[Species(name="SO4mm")],
            solvent="H2O",
            rate_constants={"CLOUD": k_func},
        )
        rc = rxn.rate_constants["CLOUD"]
        assert callable(rc)
        # Verify the callable works
        assert rc(298.15) == pytest.approx(55.556 * 7.45e7, rel=1e-10)

    def test_solvent_floor_and_min_halflife(self):
        rxn = DissolvedReaction(
            phase="AQUEOUS",
            reactants=[Species(name="A")],
            products=[Species(name="B")],
            solvent="H2O",
            rate_constants={"CLOUD": ArrheniusReferenceTemperature(A=1.0)},
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
            forward_rate_constants={"CLOUD": ArrheniusReferenceTemperature(A=1e6, C=0.0)},
            equilibrium_constant=ArrheniusReferenceTemperature(A=1.7e-2, C=2090.0),
        )
        assert "CLOUD" in rxn.forward_rate_constants
        assert len(rxn.reverse_rate_constants) == 0
        assert rxn.equilibrium_constant is not None

    def test_with_forward_and_reverse(self):
        rxn = DissolvedReversibleReaction(
            phase="AQUEOUS",
            reactants=[Species(name="A")],
            products=[Species(name="B")],
            solvent="H2O",
            forward_rate_constants={"CLOUD": ArrheniusReferenceTemperature(A=100.0, C=0.0)},
            reverse_rate_constants={"CLOUD": ArrheniusReferenceTemperature(A=50.0, C=0.0)},
        )
        assert "CLOUD" in rxn.forward_rate_constants
        assert "CLOUD" in rxn.reverse_rate_constants

    def test_defaults_are_empty(self):
        rxn = DissolvedReversibleReaction(
            phase="AQUEOUS",
            reactants=[Species(name="A")],
            products=[Species(name="B")],
            solvent="H2O",
        )
        assert len(rxn.forward_rate_constants) == 0
        assert len(rxn.reverse_rate_constants) == 0

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
            equilibrium_constant=ArrheniusReferenceTemperature(A=1.7e-2, C=2090.0),
        )
        assert dec.phase == "AQUEOUS"
        assert dec.algebraic_species == "SO2_aq"
        assert len(dec.reactants) == 1
        assert len(dec.products) == 2

    def test_solvent_floor(self):
        dec = DissolvedEquilibrium(
            phase="AQ", solvent="W",
            equilibrium_constant=ArrheniusReferenceTemperature(A=1.0),
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
                    rate_constants={"SEC": ArrheniusReferenceTemperature(A=1.0, C=0.0)},
                ),
                DissolvedReversibleReaction(
                    phase="AQ",
                    reactants=[Species(name="A")],
                    products=[Species(name="B")],
                    solvent="W",
                    equilibrium_constant=ArrheniusReferenceTemperature(A=1.0),
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
                    equilibrium_constant=ArrheniusReferenceTemperature(A=1.0),
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
