# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Unit tests for musica.miam Python data classes (constants, representations,
# processes, constraints, model) — no solver needed.

import math
import pytest
from dataclasses import FrozenInstanceError

from musica.miam import (
    HenrysLawConstant,
    EquilibriumConstant,
    ArrheniusRateConstant,
    UniformSection,
    SingleMomentMode,
    TwoMomentMode,
    DissolvedReaction,
    DissolvedReversibleReaction,
    HenryLawPhaseTransfer,
    HenryLawEquilibriumConstraint,
    DissolvedEquilibriumConstraint,
    LinearConstraint,
    LinearConstraintTerm,
    Model,
)
from musica.mechanism_configuration import Species, Phase


# ═══ Constants ═══════════════════════════════════════════════════════════════

class TestHenrysLawConstant:
    def test_construction(self):
        hlc = HenrysLawConstant(hlc_ref=1.23, c=3120.0)
        assert hlc.hlc_ref == 1.23
        assert hlc.c == 3120.0

    def test_default_c(self):
        hlc = HenrysLawConstant(hlc_ref=1.0)
        assert hlc.c == 0.0

    def test_frozen(self):
        hlc = HenrysLawConstant(hlc_ref=1.0, c=0.0)
        with pytest.raises(FrozenInstanceError):
            hlc.hlc_ref = 2.0

    def test_equality(self):
        a = HenrysLawConstant(hlc_ref=1.0, c=2.0)
        b = HenrysLawConstant(hlc_ref=1.0, c=2.0)
        assert a == b


class TestEquilibriumConstant:
    def test_construction(self):
        ec = EquilibriumConstant(a=1.7e-2, c=2090.0)
        assert ec.a == 1.7e-2
        assert ec.c == 2090.0

    def test_default_c(self):
        ec = EquilibriumConstant(a=1e-14)
        assert ec.c == 0.0

    def test_frozen(self):
        ec = EquilibriumConstant(a=1.0)
        with pytest.raises(FrozenInstanceError):
            ec.a = 2.0


class TestArrheniusRateConstant:
    def test_construction(self):
        arc = ArrheniusRateConstant(a=7.45e7, c=4430.0)
        assert arc.a == 7.45e7
        assert arc.c == 4430.0

    def test_default_c(self):
        arc = ArrheniusRateConstant(a=1.0)
        assert arc.c == 0.0

    def test_frozen(self):
        arc = ArrheniusRateConstant(a=1.0)
        with pytest.raises(FrozenInstanceError):
            arc.c = 99.0


# ═══ Representations ═════════════════════════════════════════════════════════

class TestUniformSection:
    def test_construction(self):
        us = UniformSection(name="CLOUD", phase_names=["AQUEOUS"],
                            min_radius=1e-6, max_radius=1e-5)
        assert us.name == "CLOUD"
        assert us.phase_names == ["AQUEOUS"]
        assert us.min_radius == 1e-6
        assert us.max_radius == 1e-5

    def test_no_defaults_for_radius(self):
        # min_radius and max_radius are required
        with pytest.raises(TypeError):
            UniformSection(name="CLOUD", phase_names=["AQ"])

    def test_set_default_parameters(self):
        us = UniformSection(name="CLOUD", phase_names=["AQUEOUS"],
                            min_radius=2e-6, max_radius=3e-5)

        class FakeState:
            def __init__(self):
                self._params = {}
            def set_user_defined_rate_parameters(self, params):
                self._params.update(params)

        state = FakeState()
        us.set_default_parameters(state)
        assert state._params["CLOUD.MIN_RADIUS"] == 2e-6
        assert state._params["CLOUD.MAX_RADIUS"] == 3e-5


class TestSingleMomentMode:
    def test_construction(self):
        smm = SingleMomentMode(
            name="AEROSOL", phase_names=["AQ"],
            geometric_mean_radius=1e-7, geometric_standard_deviation=1.5
        )
        assert smm.name == "AEROSOL"
        assert smm.geometric_mean_radius == 1e-7
        assert smm.geometric_standard_deviation == 1.5

    def test_set_default_parameters(self):
        smm = SingleMomentMode(
            name="MODE1", phase_names=["AQ"],
            geometric_mean_radius=1e-7, geometric_standard_deviation=1.8
        )

        class FakeState:
            def __init__(self):
                self._params = {}
            def set_user_defined_rate_parameters(self, params):
                self._params.update(params)

        state = FakeState()
        smm.set_default_parameters(state)
        assert state._params["MODE1.GEOMETRIC_MEAN_RADIUS"] == 1e-7
        assert state._params["MODE1.GEOMETRIC_STANDARD_DEVIATION"] == 1.8


class TestTwoMomentMode:
    def test_construction(self):
        tmm = TwoMomentMode(
            name="AEROSOL", phase_names=["AQ"],
            geometric_standard_deviation=2.0
        )
        assert tmm.name == "AEROSOL"
        assert tmm.geometric_standard_deviation == 2.0

    def test_set_default_parameters(self):
        tmm = TwoMomentMode(
            name="MODE2", phase_names=["AQ"],
            geometric_standard_deviation=1.6
        )

        class FakeState:
            def __init__(self):
                self._params = {}
            def set_user_defined_rate_parameters(self, params):
                self._params.update(params)

        state = FakeState()
        tmm.set_default_parameters(state)
        assert state._params["MODE2.GEOMETRIC_STANDARD_DEVIATION"] == 1.6


# ═══ Processes ═══════════════════════════════════════════════════════════════

class TestDissolvedReaction:
    def test_with_arrhenius(self):
        rxn = DissolvedReaction(
            phase_name="AQUEOUS",
            reactant_names=["HSO3m", "H2O2_aq"],
            product_names=["SO4mm", "H2O", "Hp"],
            solvent_name="H2O",
            rate_constant=ArrheniusRateConstant(a=7.45e7, c=4430.0),
        )
        assert rxn.phase_name == "AQUEOUS"
        assert len(rxn.reactant_names) == 2
        assert len(rxn.product_names) == 3
        assert isinstance(rxn.rate_constant, ArrheniusRateConstant)

    def test_with_callable(self):
        def k_func(T):
            return 55.556 * 7.45e7 * math.exp(-4430.0 * (1.0 / T - 1.0 / 298.15))

        rxn = DissolvedReaction(
            phase_name="AQUEOUS",
            reactant_names=["HSO3m", "H2O2_aq"],
            product_names=["SO4mm"],
            solvent_name="H2O",
            rate_constant=k_func,
        )
        assert callable(rxn.rate_constant)
        # Verify the callable works
        k_298 = rxn.rate_constant(298.15)
        assert k_298 == pytest.approx(55.556 * 7.45e7, rel=1e-10)


class TestDissolvedReversibleReaction:
    def test_with_forward_and_equilibrium(self):
        rxn = DissolvedReversibleReaction(
            phase_name="AQUEOUS",
            reactant_names=["SO2_aq"],
            product_names=["HSO3m", "Hp"],
            solvent_name="H2O",
            forward_rate_constant=ArrheniusRateConstant(a=1e6, c=0.0),
            equilibrium_constant=EquilibriumConstant(a=1.7e-2, c=2090.0),
        )
        assert rxn.forward_rate_constant is not None
        assert rxn.reverse_rate_constant is None
        assert rxn.equilibrium_constant is not None

    def test_with_forward_and_reverse(self):
        rxn = DissolvedReversibleReaction(
            phase_name="AQUEOUS",
            reactant_names=["A"],
            product_names=["B"],
            solvent_name="H2O",
            forward_rate_constant=ArrheniusRateConstant(a=100.0, c=0.0),
            reverse_rate_constant=ArrheniusRateConstant(a=50.0, c=0.0),
        )
        assert rxn.forward_rate_constant is not None
        assert rxn.reverse_rate_constant is not None
        assert rxn.equilibrium_constant is None

    def test_defaults_are_none(self):
        rxn = DissolvedReversibleReaction(
            phase_name="AQUEOUS",
            reactant_names=["A"],
            product_names=["B"],
            solvent_name="H2O",
        )
        assert rxn.forward_rate_constant is None
        assert rxn.reverse_rate_constant is None
        assert rxn.equilibrium_constant is None


class TestHenryLawPhaseTransfer:
    def test_construction(self):
        hlpt = HenryLawPhaseTransfer(
            condensed_phase_name="AQUEOUS",
            gas_species_name="SO2",
            condensed_species_name="SO2_aq",
            solvent_name="H2O",
            henrys_law_constant=HenrysLawConstant(hlc_ref=1.23e-2, c=3120.0),
            diffusion_coefficient=1.28e-5,
            accommodation_coefficient=0.11,
        )
        assert hlpt.condensed_phase_name == "AQUEOUS"
        assert hlpt.gas_species_name == "SO2"
        assert hlpt.diffusion_coefficient == 1.28e-5
        assert hlpt.accommodation_coefficient == 0.11


# ═══ Constraints ═════════════════════════════════════════════════════════════

class TestLinearConstraintTerm:
    def test_construction(self):
        term = LinearConstraintTerm(
            phase_name="AQUEOUS", species_name="SO4mm", coefficient=1.0
        )
        assert term.phase_name == "AQUEOUS"
        assert term.species_name == "SO4mm"
        assert term.coefficient == 1.0

    def test_frozen(self):
        term = LinearConstraintTerm(phase_name="A", species_name="B", coefficient=1.0)
        with pytest.raises(FrozenInstanceError):
            term.coefficient = 2.0


class TestHenryLawEquilibriumConstraint:
    def test_construction(self):
        hlec = HenryLawEquilibriumConstraint(
            gas_species_name="SO2",
            condensed_species_name="SO2_aq",
            solvent_name="H2O",
            condensed_phase_name="AQUEOUS",
            henrys_law_constant=HenrysLawConstant(hlc_ref=1.23e-2, c=3120.0),
            mw_solvent=0.018,
            rho_solvent=1000.0,
        )
        assert hlec.gas_species_name == "SO2"
        assert hlec.mw_solvent == 0.018
        assert hlec.rho_solvent == 1000.0

    def test_frozen(self):
        hlec = HenryLawEquilibriumConstraint(
            gas_species_name="SO2",
            condensed_species_name="SO2_aq",
            solvent_name="H2O",
            condensed_phase_name="AQUEOUS",
            henrys_law_constant=HenrysLawConstant(hlc_ref=1.23e-2, c=3120.0),
            mw_solvent=0.018,
            rho_solvent=1000.0,
        )
        with pytest.raises(FrozenInstanceError):
            hlec.mw_solvent = 0.020


class TestDissolvedEquilibriumConstraint:
    def test_construction(self):
        dec = DissolvedEquilibriumConstraint(
            phase_name="AQUEOUS",
            reactant_names=["SO2_aq"],
            product_names=["HSO3m", "Hp"],
            algebraic_species_name="SO2_aq",
            solvent_name="H2O",
            equilibrium_constant=EquilibriumConstant(a=1.7e-2, c=2090.0),
        )
        assert dec.phase_name == "AQUEOUS"
        assert dec.algebraic_species_name == "SO2_aq"
        assert len(dec.reactant_names) == 1
        assert len(dec.product_names) == 2

    def test_frozen(self):
        dec = DissolvedEquilibriumConstraint(
            phase_name="AQ",
            reactant_names=["A"],
            product_names=["B"],
            algebraic_species_name="A",
            solvent_name="W",
            equilibrium_constant=EquilibriumConstant(a=1.0),
        )
        with pytest.raises(FrozenInstanceError):
            dec.phase_name = "OTHER"


class TestLinearConstraint:
    def test_construction(self):
        lc = LinearConstraint(
            algebraic_phase_name="gas",
            algebraic_species_name="SO2",
            terms=[
                LinearConstraintTerm("gas", "SO2", 1.0),
                LinearConstraintTerm("AQUEOUS", "SO2_aq", 1.0),
                LinearConstraintTerm("AQUEOUS", "HSO3m", 1.0),
            ],
            constant=3e-8,
        )
        assert lc.algebraic_phase_name == "gas"
        assert lc.algebraic_species_name == "SO2"
        assert len(lc.terms) == 3
        assert lc.constant == 3e-8

    def test_default_constant(self):
        lc = LinearConstraint(
            algebraic_phase_name="gas",
            algebraic_species_name="X",
            terms=[LinearConstraintTerm("gas", "X", 1.0)],
        )
        assert lc.constant == 0.0

    def test_frozen(self):
        lc = LinearConstraint(
            algebraic_phase_name="gas",
            algebraic_species_name="X",
            terms=[],
            constant=0.0,
        )
        with pytest.raises(FrozenInstanceError):
            lc.constant = 1.0


# ═══ Model ═══════════════════════════════════════════════════════════════════

class TestModel:
    def _make_minimal_model(self):
        """Create a minimal valid model for testing."""
        h2o = Species(name="H2O")
        h2o.molecular_weight_kg_mol = 0.018
        h2o.density_kg_m3 = 1000.0

        so2 = Species(name="SO2")
        so2_aq = Species(name="SO2_aq")

        aq_phase = Phase(name="AQUEOUS", species=[h2o, so2_aq])

        return Model(
            name="test_model",
            species=[so2, h2o, so2_aq],
            condensed_phases=[aq_phase],
            representations=[
                UniformSection("CLOUD", ["AQUEOUS"], 1e-6, 1e-5)
            ],
        )

    def test_construction(self):
        model = self._make_minimal_model()
        assert model.name == "test_model"
        assert len(model.species) == 3
        assert len(model.condensed_phases) == 1
        assert len(model.representations) == 1
        assert len(model.processes) == 0
        assert len(model.constraints) == 0

    def test_set_default_parameters(self):
        model = self._make_minimal_model()

        class FakeState:
            def __init__(self):
                self._params = {}
            def set_user_defined_rate_parameters(self, params):
                self._params.update(params)

        state = FakeState()
        model.set_default_parameters(state)
        assert "CLOUD.MIN_RADIUS" in state._params
        assert "CLOUD.MAX_RADIUS" in state._params

    def test_set_default_parameters_multiple_representations(self):
        model = Model(
            name="multi_repr",
            species=[Species(name="A")],
            condensed_phases=[Phase(name="P", species=[Species(name="A")])],
            representations=[
                UniformSection("SEC1", ["P"], 1e-7, 1e-6),
                SingleMomentMode("MODE1", ["P"], 1e-7, 1.5),
                TwoMomentMode("MODE2", ["P"], 2.0),
            ],
        )

        class FakeState:
            def __init__(self):
                self._params = {}
            def set_user_defined_rate_parameters(self, params):
                self._params.update(params)

        state = FakeState()
        model.set_default_parameters(state)
        assert "SEC1.MIN_RADIUS" in state._params
        assert "SEC1.MAX_RADIUS" in state._params
        assert "MODE1.GEOMETRIC_MEAN_RADIUS" in state._params
        assert "MODE1.GEOMETRIC_STANDARD_DEVIATION" in state._params
        assert "MODE2.GEOMETRIC_STANDARD_DEVIATION" in state._params

    def test_empty_model(self):
        model = Model(name="empty")
        assert model.species == []
        assert model.condensed_phases == []
        assert model.representations == []
        assert model.processes == []
        assert model.constraints == []

    def test_model_with_all_process_types(self):
        model = Model(
            name="all_processes",
            species=[Species(name="A"), Species(name="B"), Species(name="W")],
            condensed_phases=[Phase(name="AQ", species=[Species(name="A"), Species(name="B"), Species(name="W")])],
            representations=[UniformSection("SEC", ["AQ"], 1e-6, 1e-5)],
            processes=[
                DissolvedReaction(
                    phase_name="AQ",
                    reactant_names=["A"],
                    product_names=["B"],
                    solvent_name="W",
                    rate_constant=ArrheniusRateConstant(a=1.0, c=0.0),
                ),
                DissolvedReversibleReaction(
                    phase_name="AQ",
                    reactant_names=["A"],
                    product_names=["B"],
                    solvent_name="W",
                    equilibrium_constant=EquilibriumConstant(a=1.0),
                ),
                HenryLawPhaseTransfer(
                    condensed_phase_name="AQ",
                    gas_species_name="A",
                    condensed_species_name="B",
                    solvent_name="W",
                    henrys_law_constant=HenrysLawConstant(hlc_ref=1.0),
                    diffusion_coefficient=1e-5,
                    accommodation_coefficient=0.1,
                ),
            ],
        )
        assert len(model.processes) == 3

    def test_model_with_all_constraint_types(self):
        model = Model(
            name="all_constraints",
            species=[Species(name="A"), Species(name="B"), Species(name="W")],
            condensed_phases=[Phase(name="AQ", species=[Species(name="A"), Species(name="B"), Species(name="W")])],
            representations=[UniformSection("SEC", ["AQ"], 1e-6, 1e-5)],
            constraints=[
                HenryLawEquilibriumConstraint(
                    gas_species_name="A",
                    condensed_species_name="B",
                    solvent_name="W",
                    condensed_phase_name="AQ",
                    henrys_law_constant=HenrysLawConstant(hlc_ref=1.0, c=100.0),
                    mw_solvent=0.018,
                    rho_solvent=1000.0,
                ),
                DissolvedEquilibriumConstraint(
                    phase_name="AQ",
                    reactant_names=["A"],
                    product_names=["B"],
                    algebraic_species_name="A",
                    solvent_name="W",
                    equilibrium_constant=EquilibriumConstant(a=1.0),
                ),
                LinearConstraint(
                    algebraic_phase_name="AQ",
                    algebraic_species_name="A",
                    terms=[
                        LinearConstraintTerm("AQ", "A", 1.0),
                        LinearConstraintTerm("AQ", "B", 1.0),
                    ],
                    constant=1e-6,
                ),
            ],
        )
        assert len(model.constraints) == 3
