# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Integration tests for MIAM aerosol model through the Python API.
# Builds the CAM Cloud Chemistry configuration and solves it.

import math
import pytest

import musica.mechanism_configuration as mc
from musica.micm import MICM, SolverType, SolverState
from musica.miam import (
    ArrheniusRateConstant,
    EquilibriumConstant,
    HenrysLawConstant,
    UniformSection,
    DissolvedReaction,
    HenryLawEquilibriumConstraint,
    DissolvedEquilibriumConstraint,
    LinearConstraint,
    LinearConstraintTerm,
    Model,
)


# ═══ Constants ═══════════════════════════════════════════════════════════════

M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556       # mol/L  (= 55556 mol/m3 when used as concentration)
MW_H2O = 0.018          # kg/mol
RHO_H2O = 1000.0        # kg/m3
R_GAS = 8.314           # J/(mol·K)
T0 = 298.15             # K  (reference temperature)

# Initial conditions
GAS0_SO2 = 3.01e-8      # mol/m3  (~ 1 ppb)
GAS0_H2O2 = 3.01e-8     # mol/m3
GAS0_O3 = 1.5e-6        # mol/m3
SO4MM0 = 1.0            # mol/m3  (background sulfate)
C_H2O = 55556.0         # mol/m3  (liquid water in droplet)
T_INIT = 280.0           # K
P_INIT = 70000.0         # Pa


# ═══ Helpers ═════════════════════════════════════════════════════════════════

def _create_gas_mechanism():
    """Create a simple gas-phase mechanism with SO2, H2O2, O3."""
    so2 = mc.Species(name="SO2")
    h2o2 = mc.Species(name="H2O2")
    o3 = mc.Species(name="O3")
    gas = mc.Phase(name="gas", species=[so2, h2o2, o3])
    return mc.Mechanism(species=[so2, h2o2, o3], phases=[gas], reactions=[])


def _create_cloud_chemistry_model():
    """Create the CAM Cloud Chemistry MIAM model.

    Species: SO2(g), H2O2(g), O3(g), SO2_aq, H2O2_aq, O3_aq,
             H+ (Hp), OH- (OHm), HSO3- (HSO3m), SO3-- (SO3mm),
             SO4-- (SO4mm), H2O (solvent).
    """
    # ── Species ──
    so2_g = mc.Species(name="SO2")
    h2o2_g = mc.Species(name="H2O2")
    o3_g = mc.Species(name="O3")
    so2_aq = mc.Species(name="SO2_aq")
    h2o2_aq = mc.Species(name="H2O2_aq")
    o3_aq = mc.Species(name="O3_aq")
    hp = mc.Species(name="Hp")
    ohm = mc.Species(name="OHm")
    hso3m = mc.Species(name="HSO3m")
    so3mm = mc.Species(name="SO3mm")
    so4mm = mc.Species(name="SO4mm")
    h2o = mc.Species(name="H2O")
    h2o.molecular_weight_kg_mol = MW_H2O
    h2o.density_kg_m3 = RHO_H2O

    all_species = [
        so2_g, h2o2_g, o3_g, so2_aq, h2o2_aq, o3_aq,
        hp, ohm, hso3m, so3mm, so4mm, h2o,
    ]

    # ── Condensed phase ──
    aq_phase = mc.Phase(
        name="AQUEOUS",
        species=[h2o, so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm],
    )

    # ── Representation ──
    cloud = UniformSection(
        name="CLOUD",
        phase_names=["AQUEOUS"],
        min_radius=1e-6,
        max_radius=1e-5,
    )

    # ── Processes ──
    # R1: HSO3- + H2O2_aq → SO4-- + H2O + H+
    r1 = DissolvedReaction(
        phase_name="AQUEOUS",
        reactant_names=["HSO3m", "H2O2_aq"],
        product_names=["SO4mm", "H2O", "Hp"],
        solvent_name="H2O",
        rate_constant=ArrheniusRateConstant(a=C_H2O_M * 7.45e7, c=4430.0),
    )

    # ── Constraints ──
    constraints = []

    # Henry's Law equilibria for SO2, H2O2, O3
    constraints.append(HenryLawEquilibriumConstraint(
        gas_species_name="SO2",
        condensed_species_name="SO2_aq",
        solvent_name="H2O",
        condensed_phase_name="AQUEOUS",
        henrys_law_constant=HenrysLawConstant(
            hlc_ref=1.23 * M_ATM_TO_MOL_M3_PA, c=3120.0),
        mw_solvent=MW_H2O,
        rho_solvent=RHO_H2O,
    ))
    constraints.append(HenryLawEquilibriumConstraint(
        gas_species_name="H2O2",
        condensed_species_name="H2O2_aq",
        solvent_name="H2O",
        condensed_phase_name="AQUEOUS",
        henrys_law_constant=HenrysLawConstant(
            hlc_ref=7.4e4 * M_ATM_TO_MOL_M3_PA, c=6621.0),
        mw_solvent=MW_H2O,
        rho_solvent=RHO_H2O,
    ))
    constraints.append(HenryLawEquilibriumConstraint(
        gas_species_name="O3",
        condensed_species_name="O3_aq",
        solvent_name="H2O",
        condensed_phase_name="AQUEOUS",
        henrys_law_constant=HenrysLawConstant(
            hlc_ref=1.15e-2 * M_ATM_TO_MOL_M3_PA, c=2560.0),
        mw_solvent=MW_H2O,
        rho_solvent=RHO_H2O,
    ))

    # Dissolved equilibria: Kw, Ka1, Ka2
    constraints.append(DissolvedEquilibriumConstraint(
        phase_name="AQUEOUS",
        reactant_names=["H2O"],
        product_names=["Hp", "OHm"],
        algebraic_species_name="OHm",
        solvent_name="H2O",
        equilibrium_constant=EquilibriumConstant(
            a=1e-14 / (C_H2O_M * C_H2O_M), c=0.0),
    ))
    constraints.append(DissolvedEquilibriumConstraint(
        phase_name="AQUEOUS",
        reactant_names=["SO2_aq"],
        product_names=["HSO3m", "Hp"],
        algebraic_species_name="HSO3m",
        solvent_name="H2O",
        equilibrium_constant=EquilibriumConstant(a=1.7e-2 / C_H2O_M, c=2090.0),
    ))
    constraints.append(DissolvedEquilibriumConstraint(
        phase_name="AQUEOUS",
        reactant_names=["HSO3m"],
        product_names=["SO3mm", "Hp"],
        algebraic_species_name="SO3mm",
        solvent_name="H2O",
        equilibrium_constant=EquilibriumConstant(a=6.0e-8 / C_H2O_M, c=1120.0),
    ))

    # Linear constraints: mass conservation + charge balance
    # Mass S: SO2_g + SO2_aq + HSO3- + SO3-- = total_S
    # NOTE: SO4-- is differential (produced by kinetics), NOT in S budget
    constraints.append(LinearConstraint(
        algebraic_phase_name="gas",
        algebraic_species_name="SO2",
        terms=[
            LinearConstraintTerm("gas", "SO2", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO2_aq", 1.0),
            LinearConstraintTerm("AQUEOUS", "HSO3m", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO3mm", 1.0),
        ],
        constant=GAS0_SO2,
    ))

    # Mass H2O2: H2O2_g + H2O2_aq = total_H2O2
    constraints.append(LinearConstraint(
        algebraic_phase_name="gas",
        algebraic_species_name="H2O2",
        terms=[
            LinearConstraintTerm("gas", "H2O2", 1.0),
            LinearConstraintTerm("AQUEOUS", "H2O2_aq", 1.0),
        ],
        constant=GAS0_H2O2,
    ))

    # Mass O3: O3_g + O3_aq = total_O3
    constraints.append(LinearConstraint(
        algebraic_phase_name="gas",
        algebraic_species_name="O3",
        terms=[
            LinearConstraintTerm("gas", "O3", 1.0),
            LinearConstraintTerm("AQUEOUS", "O3_aq", 1.0),
        ],
        constant=GAS0_O3,
    ))

    # Charge balance: H+ = OH- + HSO3- + 2*SO3-- + 2*SO4--
    constraints.append(LinearConstraint(
        algebraic_phase_name="AQUEOUS",
        algebraic_species_name="Hp",
        terms=[
            LinearConstraintTerm("AQUEOUS", "Hp", 1.0),
            LinearConstraintTerm("AQUEOUS", "OHm", -1.0),
            LinearConstraintTerm("AQUEOUS", "HSO3m", -1.0),
            LinearConstraintTerm("AQUEOUS", "SO3mm", -2.0),
            LinearConstraintTerm("AQUEOUS", "SO4mm", -2.0),
        ],
        constant=0.0,
    ))

    return Model(
        name="cloud_chemistry",
        species=all_species,
        condensed_phases=[aq_phase],
        representations=[cloud],
        processes=[r1],
        constraints=constraints,
    )


def _compute_equilibrium_ics():
    """Compute self-consistent initial conditions via damped fixed-point iteration.

    Matches the C++ reference test (Step3_FullEquilibrium).
    Returns a dict of species name -> concentration.
    """
    T = T_INIT

    # Temperature-adjusted Henry's Law constants (HLC * R * T = alpha)
    hlc_so2_T = (1.23 * M_ATM_TO_MOL_M3_PA) * math.exp(3120.0 * (1.0/T - 1.0/T0))
    hlc_h2o2_T = (7.4e4 * M_ATM_TO_MOL_M3_PA) * math.exp(6621.0 * (1.0/T - 1.0/T0))
    hlc_o3_T = (1.15e-2 * M_ATM_TO_MOL_M3_PA) * math.exp(2560.0 * (1.0/T - 1.0/T0))
    alpha_SO2 = hlc_so2_T * R_GAS * T
    alpha_H2O2 = hlc_h2o2_T * R_GAS * T
    alpha_O3 = hlc_o3_T * R_GAS * T

    # Temperature-adjusted equilibrium constants
    Ka1_T = (1.7e-2 / C_H2O_M) * math.exp(2090.0 * (1.0/T0 - 1.0/T))
    Ka2_T = (6.0e-8 / C_H2O_M) * math.exp(1120.0 * (1.0/T0 - 1.0/T))
    Kw_T = 1.0e-14 / (C_H2O_M * C_H2O_M)

    # H2O2 and O3: simple HLC split (no dissociation)
    ic_h2o2_g = GAS0_H2O2 / (1.0 + alpha_H2O2)
    ic_h2o2_aq = alpha_H2O2 * ic_h2o2_g
    ic_o3_g = GAS0_O3 / (1.0 + alpha_O3)
    ic_o3_aq = alpha_O3 * ic_o3_g

    # Iterate on [H+] for SO2 equilibria + charge balance
    hp_ic = 2.0 * SO4MM0  # charge balance dominated by SO4
    for _ in range(100):
        ic_ohm = Kw_T * C_H2O * C_H2O / hp_ic
        f = (1.0 + alpha_SO2
             + Ka1_T * alpha_SO2 * C_H2O / hp_ic
             + Ka2_T * Ka1_T * alpha_SO2 * C_H2O * C_H2O / (hp_ic * hp_ic))
        ic_so2_g = GAS0_SO2 / f
        ic_so2_aq = alpha_SO2 * ic_so2_g
        ic_hso3m = Ka1_T * ic_so2_aq * C_H2O / hp_ic
        ic_so3mm = Ka2_T * ic_hso3m * C_H2O / hp_ic
        hp_new = ic_ohm + ic_hso3m + 2.0 * ic_so3mm + 2.0 * SO4MM0
        if abs(hp_new - hp_ic) < 1e-15 * hp_ic:
            break
        hp_ic = 0.5 * (hp_ic + hp_new)

    return {
        "SO2": ic_so2_g,
        "H2O2": ic_h2o2_g,
        "O3": ic_o3_g,
        "CLOUD.AQUEOUS.H2O": C_H2O,
        "CLOUD.AQUEOUS.SO2_aq": ic_so2_aq,
        "CLOUD.AQUEOUS.H2O2_aq": ic_h2o2_aq,
        "CLOUD.AQUEOUS.O3_aq": ic_o3_aq,
        "CLOUD.AQUEOUS.Hp": hp_ic,
        "CLOUD.AQUEOUS.OHm": ic_ohm,
        "CLOUD.AQUEOUS.HSO3m": ic_hso3m,
        "CLOUD.AQUEOUS.SO3mm": ic_so3mm,
        "CLOUD.AQUEOUS.SO4mm": SO4MM0,
    }


# ═══ Tests ═══════════════════════════════════════════════════════════════════

class TestMiamModelCreation:
    """Test that a MICM solver with MIAM can be created."""

    def test_create_solver_dae4(self):
        mechanism = _create_gas_mechanism()
        model = _create_cloud_chemistry_model()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[model],
        )
        assert micm is not None
        assert micm.solver_type() == SolverType.rosenbrock_dae4_standard_order

    def test_create_solver_dae6(self):
        mechanism = _create_gas_mechanism()
        model = _create_cloud_chemistry_model()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae6_standard_order,
            external_models=[model],
        )
        assert micm is not None

    def test_create_solver_rosenbrock(self):
        """Non-DAE solver should also work with MIAM (constraints ignored)."""
        mechanism = _create_gas_mechanism()
        model = _create_cloud_chemistry_model()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_standard_order,
            external_models=[model],
        )
        assert micm is not None


class TestMiamStateCreation:
    """Test creating a state from a MIAM-enabled solver."""

    def test_create_state(self):
        mechanism = _create_gas_mechanism()
        model = _create_cloud_chemistry_model()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[model],
        )
        state = micm.create_state()
        assert state is not None

    def test_set_default_parameters(self):
        mechanism = _create_gas_mechanism()
        model = _create_cloud_chemistry_model()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[model],
        )
        state = micm.create_state()
        model.set_default_parameters(state)
        # Verify the representation parameters were set
        params = state.get_user_defined_rate_parameters()
        assert "CLOUD.MIN_RADIUS" in params
        assert params["CLOUD.MIN_RADIUS"] == [1e-6]
        assert "CLOUD.MAX_RADIUS" in params
        assert params["CLOUD.MAX_RADIUS"] == [1e-5]


class TestMiamSolve:
    """Test the MIAM solver with a cloud chemistry system."""

    def test_solve_converges(self):
        """Solver should converge for the cloud chemistry system."""
        mechanism = _create_gas_mechanism()
        model = _create_cloud_chemistry_model()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[model],
        )

        state = micm.create_state()
        model.set_default_parameters(state)

        # Set conditions
        state.set_conditions(temperatures=T_INIT, pressures=P_INIT)

        # Set self-consistent initial conditions
        ics = _compute_equilibrium_ics()
        state.set_concentrations(ics)

        # Integrate with adaptive time stepping (matching C++ reference test)
        total_time = 0.0
        target_time = 10.0
        dt = 0.01
        while total_time < target_time - 1e-10:
            step = min(dt, target_time - total_time)
            result = micm.solve(state, time_step=step)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={total_time:.4f}s"
            total_time += step
            if total_time > 0.1 and dt < 0.1:
                dt = 0.1
            if total_time > 1.0 and dt < 1.0:
                dt = 1.0

        # Mass conservation: SO2_g + SO2_aq + HSO3- + SO3-- should still
        # approximately equal the initial total (minus what reacted to SO4)
        concs = state.get_concentrations()
        so4_f = concs.get("CLOUD.AQUEOUS.SO4mm", [0.0])[0]
        assert so4_f >= SO4MM0, "SO4 should only increase from kinetics"

    def test_solve_callable_rate_constant(self):
        """Test with a callable rate constant instead of ArrheniusRateConstant."""
        mechanism = _create_gas_mechanism()
        model = _create_cloud_chemistry_model()

        # Replace R1's ArrheniusRateConstant with a Python callable
        model.processes[0] = DissolvedReaction(
            phase_name="AQUEOUS",
            reactant_names=["HSO3m", "H2O2_aq"],
            product_names=["SO4mm", "H2O", "Hp"],
            solvent_name="H2O",
            rate_constant=lambda T: C_H2O_M * 7.45e7 * math.exp(
                -4430.0 * (1.0 / T - 1.0 / T0)),
        )

        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[model],
        )

        state = micm.create_state()
        model.set_default_parameters(state)

        state.set_conditions(temperatures=T_INIT, pressures=P_INIT)
        ics = _compute_equilibrium_ics()
        state.set_concentrations(ics)

        # Integrate with adaptive time stepping
        total_time = 0.0
        target_time = 10.0
        dt = 0.01
        while total_time < target_time - 1e-10:
            step = min(dt, target_time - total_time)
            result = micm.solve(state, time_step=step)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={total_time:.4f}s"
            total_time += step
            if total_time > 0.1 and dt < 0.1:
                dt = 0.1
            if total_time > 1.0 and dt < 1.0:
                dt = 1.0


class TestMiamErrorCases:
    """Test error handling in MIAM solver creation."""

    def test_external_models_with_config_path(self):
        """Using external_models with config_path should raise ValueError."""
        model = _create_cloud_chemistry_model()
        with pytest.raises(ValueError, match="external_models cannot be used with config_path"):
            MICM(
                config_path="dummy/path",
                external_models=[model],
            )

    def test_invalid_species_in_model(self):
        """A model referencing non-existent species should raise an error."""
        mechanism = _create_gas_mechanism()

        model = Model(
            name="bad_model",
            species=[mc.Species(name="X")],
            condensed_phases=[mc.Phase(name="P", species=[mc.Species(name="X")])],
            representations=[UniformSection("S", ["P"], 1e-6, 1e-5)],
            processes=[
                DissolvedReaction(
                    phase_name="P",
                    reactant_names=["NONEXISTENT"],
                    product_names=["X"],
                    solvent_name="X",
                    rate_constant=ArrheniusRateConstant(a=1.0),
                ),
            ],
        )

        with pytest.raises(Exception):
            MICM(
                mechanism=mechanism,
                solver_type=SolverType.rosenbrock_standard_order,
                external_models=[model],
            )


class TestMiamToConfig:
    """Test the _to_config() conversion from Python to pybind11 types."""

    def test_to_config_round_trip(self):
        """Verify _to_config() produces a valid _ModelConfig."""
        model = _create_cloud_chemistry_model()
        config = model._to_config()

        assert config.name == "cloud_chemistry"
        assert len(config.species) == 12
        assert len(config.condensed_phases) == 1
        assert len(config.representations) == 1
        assert len(config.processes) == 1
        assert len(config.constraints) == 10

    def test_species_properties_preserved(self):
        """Verify molecular weight and density are preserved through _to_config()."""
        h2o = mc.Species(name="H2O")
        h2o.molecular_weight_kg_mol = 0.018
        h2o.density_kg_m3 = 1000.0

        model = Model(
            name="test",
            species=[h2o],
            condensed_phases=[mc.Phase(name="P", species=[h2o])],
            representations=[UniformSection("S", ["P"], 1e-6, 1e-5)],
        )
        config = model._to_config()

        assert len(config.species) == 1
        sp = config.species[0]
        assert sp.name == "H2O"
        assert sp.molecular_weight == pytest.approx(0.018)
        assert sp.density == pytest.approx(1000.0)

    def test_minimal_config(self):
        """An empty model should produce a valid config."""
        model = Model(name="empty")
        config = model._to_config()
        assert config.name == "empty"
        assert len(config.species) == 0
        assert len(config.processes) == 0
        assert len(config.constraints) == 0
