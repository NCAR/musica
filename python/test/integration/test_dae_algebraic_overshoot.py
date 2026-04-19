"""
Test demonstrating that algebraic variables excluded from the Rosenbrock
error estimator can overshoot through zero in DAE systems.

Issue
-----
In ``NormalizedError()`` (rosenbrock.inl), variables with
``upper_left_identity_diagonal_[i] == 0`` (algebraic) are skipped when
computing the step-acceptance error norm.  When a fast kinetic reaction
converts species that participate in a conservation constraint, the
differential product can overshoot the mass budget.  The algebraic
"balance" variable absorbs the overshoot and goes negative, but this
is never detected because its error is excluded from the norm.

Minimal chemistry (H2O2-pathway sulfate production)
---------------------------------------------------
Gas:      SO2, H2O2
Aqueous:  H2O, SO2_aq, H2O2_aq, HSO3m, Hp, OHm, SO4mm, SO2OOHm

Reactions (aqueous, dissolved):
  R1a: HSO3m + H2O2_aq ⇌ SO2OOHm + H2O   (reversible, fast)
  R1b: SO2OOHm + Hp  →  SO4mm              (irreversible, fast)

Equilibrium constraints:
  Henry's law:   SO2  ↔ SO2_aq,  H2O2 ↔ H2O2_aq
  Dissociation:  SO2_aq → HSO3m + Hp,  H2O → Hp + OHm

Conservation constraints (LinearConstraint, diagnose_from_state=True):
  Total S:    SO2 + SO2_aq + HSO3m + SO2OOHm + SO4mm = C_s   (SO2 algebraic)
  Total H2O2: H2O2 + H2O2_aq = C_h                           (H2O2 algebraic)
  Charge:     Hp - OHm - HSO3m - 2·SO4mm - SO2OOHm = 0       (Hp algebraic)

Expected behaviour
------------------
The continuous system can never produce [SO2] < 0 because the kinetic
rates → 0 as S(IV) → 0.  With the default error estimator (algebraic
variables excluded), the solver takes ~13 internal steps for a 30 s
interval and SO2(g) goes to ≈ −5.6e-8.  With very tight tolerances
(atol ~ 1e-20) the solver is forced to take ~700k steps, tracks the
kinetic deceleration, and SO2(g) stays positive.

Proposed fix
------------
Include algebraic variables in ``NormalizedError()``.  This causes the
error estimator to reject steps where the algebraic variable changes
sign or experiences a large relative change, without requiring
impractical tolerance settings.
"""

import math
import pytest

import musica.mechanism_configuration as mc
from musica.micm import MICM, SolverState, SolverType
from musica.micm.solver_parameters import RosenbrockSolverParameters
from musica.miam import (
    ArrheniusRateConstant,
    DissolvedEquilibriumConstraint,
    DissolvedReaction,
    DissolvedReversibleReaction,
    EquilibriumConstant,
    HenryLawEquilibriumConstraint,
    HenrysLawConstant,
    LinearConstraint,
    LinearConstraintTerm,
    Model,
    UniformSection,
)


# ── Physical constants ──────────────────────────────────────────────────
MW_H2O = 0.018           # kg/mol
RHO_H2O = 1000.0         # kg/m³
M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556         # mol H2O per litre

LWC = 0.3e-3             # kg water / m³ air
C_H2O = LWC / MW_H2O     # mol H2O / m³ air  ≈ 0.01667
f_v = C_H2O * MW_H2O / RHO_H2O   # volume fraction
AIR_TO_SOLUTION = f_v * 1000.0    # ≈ 3e-4

T = 280.0
P = 70000.0


def _build_system():
    """Build the minimal SO2/H2O2 cloud-chemistry system."""

    # ── Gas-phase species & mechanism ───────────────────────────────
    so2_g = mc.Species(name="SO2")
    h2o2_g = mc.Species(name="H2O2")
    gas = mc.Phase(name="gas", species=[so2_g, h2o2_g])
    gas_mechanism = mc.Mechanism(
        name="minimal_gas",
        species=[so2_g, h2o2_g],
        phases=[gas],
        reactions=[],
    )

    # ── Aqueous species ─────────────────────────────────────────────
    h2o = mc.Species(name="H2O")
    h2o.molecular_weight_kg_mol = MW_H2O
    h2o.density_kg_m3 = RHO_H2O

    so2_aq = mc.Species(name="SO2_aq")
    h2o2_aq = mc.Species(name="H2O2_aq")
    hp = mc.Species(name="Hp")
    ohm = mc.Species(name="OHm")
    hso3m = mc.Species(name="HSO3m")
    so4mm = mc.Species(name="SO4mm")
    so2oohm = mc.Species(name="SO2OOHm")

    aq_phase = mc.Phase(
        name="AQUEOUS",
        species=[h2o, so2_aq, h2o2_aq, hp, ohm, hso3m, so4mm, so2oohm],
    )
    cloud = UniformSection(
        name="CLOUD", phase_names=["AQUEOUS"],
        min_radius=1e-6, max_radius=1e-5,
    )

    all_species = [so2_g, h2o2_g, so2_aq, h2o2_aq, hp, ohm, hso3m, so4mm, so2oohm, h2o]

    # ── Kinetic reactions (H2O2 pathway only) ───────────────────────
    r1a = DissolvedReversibleReaction(
        phase_name="AQUEOUS",
        reactant_names=["HSO3m", "H2O2_aq"],
        product_names=["SO2OOHm", "H2O"],
        solvent_name="H2O",
        forward_rate_constant=ArrheniusRateConstant(
            a=C_H2O_M * (7.45e7 / 13.0), c=4430.0),
        equilibrium_constant=EquilibriumConstant(a=1725.0),
    )
    r1b = DissolvedReaction(
        phase_name="AQUEOUS",
        reactant_names=["SO2OOHm", "Hp"],
        product_names=["SO4mm"],
        solvent_name="H2O",
        rate_constant=ArrheniusRateConstant(a=C_H2O_M * 2.4e6, c=4430.0),
    )

    # ── Equilibrium constraints ─────────────────────────────────────
    constraints = []

    # Henry's Law
    for gas_name, aq_name, hlc, c_val in [
        ("SO2",  "SO2_aq",  1.23,   3120.0),
        ("H2O2", "H2O2_aq", 7.4e4,  6621.0),
    ]:
        constraints.append(HenryLawEquilibriumConstraint(
            gas_species_name=gas_name,
            condensed_species_name=aq_name,
            solvent_name="H2O",
            condensed_phase_name="AQUEOUS",
            henrys_law_constant=HenrysLawConstant(
                hlc_ref=hlc * M_ATM_TO_MOL_M3_PA, c=c_val),
            mw_solvent=MW_H2O,
            rho_solvent=RHO_H2O,
        ))

    # Dissociation
    constraints.append(DissolvedEquilibriumConstraint(
        phase_name="AQUEOUS",
        reactant_names=["H2O"], product_names=["Hp", "OHm"],
        algebraic_species_name="OHm", solvent_name="H2O",
        equilibrium_constant=EquilibriumConstant(
            a=1e-14 / (C_H2O_M * C_H2O_M), c=0.0),
    ))
    constraints.append(DissolvedEquilibriumConstraint(
        phase_name="AQUEOUS",
        reactant_names=["SO2_aq"], product_names=["HSO3m", "Hp"],
        algebraic_species_name="HSO3m", solvent_name="H2O",
        equilibrium_constant=EquilibriumConstant(
            a=1.7e-2 / C_H2O_M, c=2090.0),
    ))

    # ── Conservation constraints ────────────────────────────────────
    # Total sulfur (SO2 is algebraic — computed from budget)
    constraints.append(LinearConstraint(
        algebraic_phase_name="gas",
        algebraic_species_name="SO2",
        terms=[
            LinearConstraintTerm("gas", "SO2", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO2_aq", 1.0),
            LinearConstraintTerm("AQUEOUS", "HSO3m", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO2OOHm", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO4mm", 1.0),
        ],
        diagnose_from_state=True,
    ))

    # Total H2O2
    constraints.append(LinearConstraint(
        algebraic_phase_name="gas",
        algebraic_species_name="H2O2",
        terms=[
            LinearConstraintTerm("gas", "H2O2", 1.0),
            LinearConstraintTerm("AQUEOUS", "H2O2_aq", 1.0),
        ],
        diagnose_from_state=True,
    ))

    # Charge balance (Hp is algebraic)
    constraints.append(LinearConstraint(
        algebraic_phase_name="AQUEOUS",
        algebraic_species_name="Hp",
        terms=[
            LinearConstraintTerm("AQUEOUS", "Hp", 1.0),
            LinearConstraintTerm("AQUEOUS", "OHm", -1.0),
            LinearConstraintTerm("AQUEOUS", "HSO3m", -1.0),
            LinearConstraintTerm("AQUEOUS", "SO4mm", -2.0),
            LinearConstraintTerm("AQUEOUS", "SO2OOHm", -1.0),
        ],
        constant=0.0,
    ))

    # ── Assemble ────────────────────────────────────────────────────
    miam_model = Model(
        name="minimal_cloud",
        species=all_species,
        condensed_phases=[aq_phase],
        representations=[cloud],
        processes=[r1a, r1b],
        constraints=constraints,
    )

    micm = MICM(
        mechanism=gas_mechanism,
        solver_type=SolverType.rosenbrock_dae4_standard_order,
        external_models=[miam_model],
    )

    return micm, miam_model


def _create_initial_state(micm, miam_model, so4_init=1e-6):
    """Create a state with physically reasonable initial conditions."""
    state = micm.create_state()
    miam_model.set_default_parameters(state)
    state.set_conditions(temperatures=T, pressures=P)

    so2_init = 3.01e-7     # mol/m³ air
    h2o2_init = 3.01e-7
    hp_init = 2.0 * so4_init  # approximate charge balance

    state.set_concentrations({
        "SO2": so2_init,
        "H2O2": h2o2_init,
        "CLOUD.AQUEOUS.H2O": C_H2O,
        "CLOUD.AQUEOUS.SO2_aq": 1e-12,
        "CLOUD.AQUEOUS.H2O2_aq": 1e-12,
        "CLOUD.AQUEOUS.Hp": hp_init,
        "CLOUD.AQUEOUS.OHm": 1e-14,
        "CLOUD.AQUEOUS.HSO3m": 1e-12,
        "CLOUD.AQUEOUS.SO4mm": so4_init,
        "CLOUD.AQUEOUS.SO2OOHm": 1e-30,
    })
    return state


def _total_sulfur(concs):
    """Sum all sulfur-containing species."""
    return (
        concs["SO2"][0]
        + concs["CLOUD.AQUEOUS.SO2_aq"][0]
        + concs["CLOUD.AQUEOUS.HSO3m"][0]
        + concs["CLOUD.AQUEOUS.SO2OOHm"][0]
        + concs["CLOUD.AQUEOUS.SO4mm"][0]
    )


class TestDAEAlgebraicOvershoot:
    """Demonstrate that algebraic variables can go negative when excluded
    from the Rosenbrock error estimator."""

    def test_negative_so2_with_default_tolerances(self):
        """With default tolerances, SO2(g) goes negative after a 30 s solve.

        The solver takes only ~13 internal steps.  The error on the
        differential SO4²⁻ is well within tolerance, so every step is
        accepted.  But SO4²⁻ overshoots the total-S budget, and the
        algebraic SO2(g) = C − SO4²⁻ − ... goes negative.
        """
        micm, miam_model = _build_system()
        ordering = micm.create_state().get_species_ordering()

        # Standard tolerances: atol=1e-14 aqueous, 1e-3 gas, rtol=1e-6
        abs_tols = [1e-3] * len(ordering)
        for name, idx in ordering.items():
            if "CLOUD.AQUEOUS." in name:
                abs_tols[idx] = 1e-14

        micm.set_solver_parameters(RosenbrockSolverParameters(
            absolute_tolerances=abs_tols,
            constraint_init_max_iterations=100,
            constraint_init_tolerance=1e-8,
            max_number_of_steps=50000,
        ))

        state = _create_initial_state(micm, miam_model, so4_init=1e-6)
        initial_total_s = _total_sulfur(state.get_concentrations())

        result = micm.solve(state, time_step=30.0)
        concs = state.get_concentrations()

        assert result.state == SolverState.Converged

        so2_final = concs["SO2"][0]
        final_total_s = _total_sulfur(concs)

        # Conservation is maintained (constraint works)
        assert abs(final_total_s - initial_total_s) / initial_total_s < 1e-6, (
            f"Total S not conserved: {initial_total_s:.6e} → {final_total_s:.6e}"
        )

        # BUG: SO2(g) is negative despite conservation being satisfied
        assert so2_final < 0, (
            f"Expected SO2 < 0 with default tolerances, got {so2_final:.4e}. "
            f"If this test fails, the bug may have been fixed!"
        )

        # The solver took very few steps (algebraic error not checked)
        assert result.stats.accepted < 100, (
            f"Expected few steps with default tolerances, got {result.stats.accepted}"
        )

    def test_positive_so2_with_extreme_tolerances(self):
        """With extreme tolerances (atol=1e-20, rtol=1e-14), SO2(g)
        stays positive — but at the cost of ~700k internal steps.

        This confirms the physics is correct and the negative SO2 is
        purely a step-acceptance artifact.
        """
        micm, miam_model = _build_system()
        ordering = micm.create_state().get_species_ordering()

        abs_tols = [1e-20] * len(ordering)

        micm.set_solver_parameters(RosenbrockSolverParameters(
            relative_tolerance=1e-14,
            absolute_tolerances=abs_tols,
            constraint_init_max_iterations=100,
            constraint_init_tolerance=1e-8,
            max_number_of_steps=5000000,
        ))

        state = _create_initial_state(micm, miam_model, so4_init=1e-6)
        initial_total_s = _total_sulfur(state.get_concentrations())

        result = micm.solve(state, time_step=30.0)
        concs = state.get_concentrations()

        assert result.state == SolverState.Converged

        so2_final = concs["SO2"][0]
        final_total_s = _total_sulfur(concs)

        # Conservation still maintained
        assert abs(final_total_s - initial_total_s) / initial_total_s < 1e-6

        # With extreme tolerances, SO2 stays positive
        assert so2_final > 0, (
            f"Expected SO2 > 0 with extreme tolerances, got {so2_final:.4e}"
        )

        # But it costs hundreds of thousands of steps
        assert result.stats.accepted > 100000, (
            f"Expected >100k steps, got {result.stats.accepted}"
        )


if __name__ == "__main__":
    """Run as a standalone script with diagnostic output."""
    micm, miam_model = _build_system()
    ordering = micm.create_state().get_species_ordering()
    n = len(ordering)

    print(f"Species ({n}): {ordering}\n")

    # Count algebraic vs differential
    algebraic = {
        "SO2", "H2O2",  # LinearConstraint
        "CLOUD.AQUEOUS.SO2_aq", "CLOUD.AQUEOUS.H2O2_aq",  # Henry's Law
        "CLOUD.AQUEOUS.OHm", "CLOUD.AQUEOUS.HSO3m",  # Dissociation
        "CLOUD.AQUEOUS.Hp",  # Charge balance
    }
    differential = set(ordering.keys()) - algebraic
    print(f"Algebraic  ({len(algebraic)}): {sorted(algebraic)}")
    print(f"Differential ({len(differential)}): {sorted(differential)}")

    print(f"\n{'='*90}")
    print(f"{'Config':55s} | {'SO2(g)':>12s} | {'SO4²⁻':>12s} | {'Steps':>8s} | {'Reject':>6s}")
    print(f"{'-'*90}")

    configs = [
        ("default: atol=1e-14(aq)/1e-3(gas), rtol=1e-6", 1e-6, 1e-3, 1e-14),
        ("tight rtol: rtol=1e-12",                        1e-12, 1e-3, 1e-14),
        ("uniform atol=1e-14, rtol=1e-6",                 1e-6, 1e-14, 1e-14),
        ("uniform atol=1e-16, rtol=1e-12",                1e-12, 1e-16, 1e-16),
        ("uniform atol=1e-18, rtol=1e-12",                1e-12, 1e-18, 1e-18),
        ("uniform atol=1e-19, rtol=1e-12",                1e-12, 1e-19, 1e-19),
        ("uniform atol=1e-20, rtol=1e-14",                1e-14, 1e-20, 1e-20),
    ]

    for label, rtol, gas_atol, aq_atol in configs:
        tols = [gas_atol] * n
        for name, idx in ordering.items():
            if "CLOUD.AQUEOUS." in name:
                tols[idx] = aq_atol

        micm.set_solver_parameters(RosenbrockSolverParameters(
            relative_tolerance=rtol,
            absolute_tolerances=tols,
            constraint_init_max_iterations=100,
            constraint_init_tolerance=1e-8,
            max_number_of_steps=5000000,
        ))

        state = _create_initial_state(micm, miam_model, so4_init=1e-6)
        result = micm.solve(state, time_step=30.0)
        c = state.get_concentrations()

        so2 = c["SO2"][0]
        so4 = c["CLOUD.AQUEOUS.SO4mm"][0]
        steps = result.stats.accepted
        rejects = result.stats.rejected
        flag = " ← NEGATIVE" if so2 < 0 else ""

        print(f"  {label:53s} | {so2:+.4e} | {so4:.4e} | {steps:8d} | {rejects:6d}{flag}")

    print(f"\nConclusion: algebraic SO2(g) goes negative with practical")
    print(f"tolerances because NormalizedError() skips it. Only atol ≤ 1e-19")
    print(f"forces enough internal steps to track the kinetic deceleration.")
