"""
Test demonstrating the Rosenbrock DAE solver's step-change error estimate
for algebraic variables and its effect on conservation-constraint overshoot.

Background
----------
MICM's Rosenbrock DAE solver now uses **step-change error estimation** for
algebraic variables: after computing the embedded error, the solver replaces
each algebraic entry with ``Yerror[a] = Ynew[a] - Y[a]``. This means
algebraic tolerances directly control step acceptance.

With loose tolerances, the solver accepts steps where differential products
overshoot a conservation budget, forcing the algebraic balance variable
negative. With tight algebraic tolerances, the solver rejects such steps
and takes smaller ones that track the kinetic deceleration.

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
With loose algebraic tolerances (atol ≥ 1e-8), the solver accepts large
steps and SO₂(g) goes negative (~−6.5e-8). With tight algebraic tolerances
(atol ≤ 1e-12), the solver takes enough steps to prevent overshoot and
SO₂(g) stays positive — but at a cost of ~130k+ internal steps.
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


ALGEBRAIC_NAMES = {
    "SO2", "H2O2",
    "CLOUD.AQUEOUS.SO2_aq", "CLOUD.AQUEOUS.H2O2_aq",
    "CLOUD.AQUEOUS.OHm", "CLOUD.AQUEOUS.HSO3m", "CLOUD.AQUEOUS.Hp",
}


class TestDAEAlgebraicOvershoot:
    """Test the step-change error estimate for algebraic variables.

    The Rosenbrock DAE solver now uses ``Yerror[a] = Ynew[a] - Y[a]`` for
    algebraic variables, making their tolerances control step acceptance.
    See dae_algebraic_tolerance_guide.md for the full specification.
    """

    def test_algebraic_tolerance_sensitivity(self):
        """Changing atol for algebraic species from 1e-3 to 1e-10
        must change step count — proving the error estimator sees them.
        """
        micm, miam_model = _build_system()
        ordering = micm.create_state().get_species_ordering()
        n = len(ordering)

        results = {}
        for alg_atol in [1e-3, 1e-10]:
            abs_tols = [1e-14] * n  # differential species
            for name, idx in ordering.items():
                if name in ALGEBRAIC_NAMES:
                    abs_tols[idx] = alg_atol

            micm.set_solver_parameters(RosenbrockSolverParameters(
                absolute_tolerances=abs_tols,
                constraint_init_max_iterations=100,
                constraint_init_tolerance=1e-8,
                max_number_of_steps=500000,
            ))

            state = _create_initial_state(micm, miam_model, so4_init=1e-6)
            result = micm.solve(state, time_step=30.0)
            assert result.state == SolverState.Converged
            results[alg_atol] = result.stats.accepted

        # Tight algebraic tolerance should require significantly more steps
        assert results[1e-10] > 10 * results[1e-3], (
            f"Expected tight atol to need many more steps: "
            f"loose={results[1e-3]}, tight={results[1e-10]}"
        )

    def test_negative_so2_with_loose_algebraic_tolerances(self):
        """With loose algebraic tolerances (1e-3), SO2(g) goes negative.

        The step-change error for algebraic variables is within their
        generous tolerance, so the solver accepts large steps.
        """
        micm, miam_model = _build_system()
        ordering = micm.create_state().get_species_ordering()
        n = len(ordering)

        abs_tols = [1e-14] * n  # differential species
        for name, idx in ordering.items():
            if name in ALGEBRAIC_NAMES:
                abs_tols[idx] = 1e-3

        micm.set_solver_parameters(RosenbrockSolverParameters(
            absolute_tolerances=abs_tols,
            constraint_init_max_iterations=100,
            constraint_init_tolerance=1e-8,
            max_number_of_steps=500000,
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

        # With loose algebraic tolerances, SO2(g) goes negative
        assert so2_final < 0, (
            f"Expected SO2 < 0 with loose algebraic atol, got {so2_final:.4e}"
        )

    def test_positive_so2_with_tight_algebraic_tolerances(self):
        """With tight algebraic tolerances (1e-12), SO2(g) stays positive.

        The step-change error estimate forces the solver to reject steps
        where the algebraic balance variable changes too much, preventing
        overshoot of the conservation budget.
        """
        micm, miam_model = _build_system()
        ordering = micm.create_state().get_species_ordering()
        n = len(ordering)

        abs_tols = [1e-14] * n  # differential species
        for name, idx in ordering.items():
            if name in ALGEBRAIC_NAMES:
                abs_tols[idx] = 1e-12

        micm.set_solver_parameters(RosenbrockSolverParameters(
            absolute_tolerances=abs_tols,
            constraint_init_max_iterations=100,
            constraint_init_tolerance=1e-8,
            max_number_of_steps=500000,
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

        # With tight algebraic tolerances, SO2 stays positive
        assert so2_final > 0, (
            f"Expected SO2 > 0 with tight algebraic atol, got {so2_final:.4e}"
        )

        # Balance variable should not exceed -atol (per guide)
        assert so2_final >= -1e-12, (
            f"SO2 went more negative than -atol: {so2_final:.4e}"
        )


if __name__ == "__main__":
    """Run as a standalone script with diagnostic output."""
    micm, miam_model = _build_system()
    ordering = micm.create_state().get_species_ordering()
    n = len(ordering)

    print(f"Species ({n}): {ordering}\n")
    print(f"Algebraic: {sorted(ALGEBRAIC_NAMES)}")
    print(f"Differential: {sorted(set(ordering.keys()) - ALGEBRAIC_NAMES)}")

    print(f"\n{'='*90}")
    print(f"{'Algebraic atol':20s} | {'SO2(g)':>12s} | {'SO4²⁻':>12s} | {'Steps':>8s} | {'Reject':>6s}")
    print(f"{'-'*90}")

    for alg_atol in [1e-3, 1e-6, 1e-8, 1e-10, 1e-11, 1e-12, 1e-14]:
        tols = [1e-14] * n  # differential default
        for name, idx in ordering.items():
            if name in ALGEBRAIC_NAMES:
                tols[idx] = alg_atol

        micm.set_solver_parameters(RosenbrockSolverParameters(
            absolute_tolerances=tols,
            constraint_init_max_iterations=100,
            constraint_init_tolerance=1e-8,
            max_number_of_steps=500000,
        ))

        state = _create_initial_state(micm, miam_model, so4_init=1e-6)
        result = micm.solve(state, time_step=30.0)
        c = state.get_concentrations()

        so2 = c["SO2"][0]
        so4 = c["CLOUD.AQUEOUS.SO4mm"][0]
        steps = result.stats.accepted
        rejects = result.stats.rejected
        flag = " ← NEGATIVE" if so2 < 0 else ""

        print(f"  {alg_atol:.0e}{'':<13s} | {so2:+.4e} | {so4:.4e} | {steps:8d} | {rejects:6d}{flag}")

    print(f"\nConclusion: step-change error estimate makes algebraic tolerances")
    print(f"control step acceptance. Tight atol (≤ 1e-12) prevents overshoot.")
