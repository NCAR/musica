"""
Diagnostic script to investigate why the Rosenbrock DAE error estimator
doesn't prevent algebraic variable overshoot.

Hypothesis: The error vector (Yerror = K[3] for DAE4) should be non-zero
for algebraic variables, but it may be too small relative to atol + rtol*ymax
to trigger step rejection.
"""
import math
import numpy as np

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

# ── Physical constants ──
MW_H2O = 0.018
RHO_H2O = 1000.0
M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556
LWC = 0.3e-3
C_H2O = LWC / MW_H2O
f_v = C_H2O * MW_H2O / RHO_H2O
T = 280.0
P = 70000.0


def build_system():
    so2_g = mc.Species(name="SO2")
    h2o2_g = mc.Species(name="H2O2")
    gas = mc.Phase(name="gas", species=[so2_g, h2o2_g])
    gas_mechanism = mc.Mechanism(
        name="minimal_gas", species=[so2_g, h2o2_g], phases=[gas], reactions=[],
    )

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

    constraints = []
    for gas_name, aq_name, hlc, c_val in [
        ("SO2", "SO2_aq", 1.23, 3120.0),
        ("H2O2", "H2O2_aq", 7.4e4, 6621.0),
    ]:
        constraints.append(HenryLawEquilibriumConstraint(
            gas_species_name=gas_name, condensed_species_name=aq_name,
            solvent_name="H2O", condensed_phase_name="AQUEOUS",
            henrys_law_constant=HenrysLawConstant(hlc_ref=hlc * M_ATM_TO_MOL_M3_PA, c=c_val),
            mw_solvent=MW_H2O, rho_solvent=RHO_H2O,
        ))
    constraints.append(DissolvedEquilibriumConstraint(
        phase_name="AQUEOUS", reactant_names=["H2O"], product_names=["Hp", "OHm"],
        algebraic_species_name="OHm", solvent_name="H2O",
        equilibrium_constant=EquilibriumConstant(a=1e-14 / (C_H2O_M * C_H2O_M), c=0.0),
    ))
    constraints.append(DissolvedEquilibriumConstraint(
        phase_name="AQUEOUS", reactant_names=["SO2_aq"], product_names=["HSO3m", "Hp"],
        algebraic_species_name="HSO3m", solvent_name="H2O",
        equilibrium_constant=EquilibriumConstant(a=1.7e-2 / C_H2O_M, c=2090.0),
    ))
    constraints.append(LinearConstraint(
        algebraic_phase_name="gas", algebraic_species_name="SO2",
        terms=[
            LinearConstraintTerm("gas", "SO2", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO2_aq", 1.0),
            LinearConstraintTerm("AQUEOUS", "HSO3m", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO2OOHm", 1.0),
            LinearConstraintTerm("AQUEOUS", "SO4mm", 1.0),
        ],
        diagnose_from_state=True,
    ))
    constraints.append(LinearConstraint(
        algebraic_phase_name="gas", algebraic_species_name="H2O2",
        terms=[
            LinearConstraintTerm("gas", "H2O2", 1.0),
            LinearConstraintTerm("AQUEOUS", "H2O2_aq", 1.0),
        ],
        diagnose_from_state=True,
    ))
    constraints.append(LinearConstraint(
        algebraic_phase_name="AQUEOUS", algebraic_species_name="Hp",
        terms=[
            LinearConstraintTerm("AQUEOUS", "Hp", 1.0),
            LinearConstraintTerm("AQUEOUS", "OHm", -1.0),
            LinearConstraintTerm("AQUEOUS", "HSO3m", -1.0),
            LinearConstraintTerm("AQUEOUS", "SO4mm", -2.0),
            LinearConstraintTerm("AQUEOUS", "SO2OOHm", -1.0),
        ],
        constant=0.0,
    ))

    miam_model = Model(
        name="minimal_cloud", species=all_species, condensed_phases=[aq_phase],
        representations=[cloud], processes=[r1a, r1b], constraints=constraints,
    )
    micm = MICM(
        mechanism=gas_mechanism, solver_type=SolverType.rosenbrock_dae4_standard_order,
        external_models=[miam_model],
    )
    return micm, miam_model


def total_sulfur(concs):
    return (
        concs["SO2"][0]
        + concs["CLOUD.AQUEOUS.SO2_aq"][0]
        + concs["CLOUD.AQUEOUS.HSO3m"][0]
        + concs["CLOUD.AQUEOUS.SO2OOHm"][0]
        + concs["CLOUD.AQUEOUS.SO4mm"][0]
    )


def run_with_substeps(micm, miam_model, total_time, n_substeps, abs_tols, rtol=1e-6):
    """Run solver with sub-stepping to observe the trajectory."""
    ordering = micm.create_state().get_species_ordering()

    micm.set_solver_parameters(RosenbrockSolverParameters(
        relative_tolerance=rtol,
        absolute_tolerances=abs_tols,
        constraint_init_max_iterations=100,
        constraint_init_tolerance=1e-8,
        max_number_of_steps=5000000,
    ))

    state = micm.create_state()
    miam_model.set_default_parameters(state)
    state.set_conditions(temperatures=T, pressures=P)
    state.set_concentrations({
        "SO2": 3.01e-7, "H2O2": 3.01e-7,
        "CLOUD.AQUEOUS.H2O": C_H2O,
        "CLOUD.AQUEOUS.SO2_aq": 1e-12, "CLOUD.AQUEOUS.H2O2_aq": 1e-12,
        "CLOUD.AQUEOUS.Hp": 2e-6, "CLOUD.AQUEOUS.OHm": 1e-14,
        "CLOUD.AQUEOUS.HSO3m": 1e-12, "CLOUD.AQUEOUS.SO4mm": 1e-6,
        "CLOUD.AQUEOUS.SO2OOHm": 1e-30,
    })

    dt = total_time / n_substeps
    trajectory = []
    total_steps = 0
    total_rejected = 0

    for i in range(n_substeps):
        result = micm.solve(state, time_step=dt)
        c = state.get_concentrations()
        so2 = c["SO2"][0]
        so4 = c["CLOUD.AQUEOUS.SO4mm"][0]
        ts = total_sulfur(c)
        total_steps += result.stats.accepted
        total_rejected += result.stats.rejected
        trajectory.append({
            "t": (i + 1) * dt,
            "so2": so2,
            "so4": so4,
            "total_s": ts,
            "steps": result.stats.accepted,
            "rejected": result.stats.rejected,
            "converged": result.state == SolverState.Converged,
        })

    return trajectory, total_steps, total_rejected


if __name__ == "__main__":
    micm, miam_model = build_system()
    ordering = micm.create_state().get_species_ordering()
    n = len(ordering)

    print("=" * 100)
    print("DIAGNOSTIC: Why does removing the algebraic skip from NormalizedError not fix the overshoot?")
    print("=" * 100)

    algebraic = {
        "SO2", "H2O2",
        "CLOUD.AQUEOUS.SO2_aq", "CLOUD.AQUEOUS.H2O2_aq",
        "CLOUD.AQUEOUS.OHm", "CLOUD.AQUEOUS.HSO3m",
        "CLOUD.AQUEOUS.Hp",
    }
    print(f"\nSpecies ({n}):")
    for name, idx in sorted(ordering.items(), key=lambda x: x[1]):
        kind = "ALGEBRAIC" if name in algebraic else "DIFFERENTIAL"
        print(f"  [{idx:2d}] {name:40s}  {kind}")

    # ── Test 1: Single 30s solve with varying atol ──
    print(f"\n{'='*100}")
    print("Test 1: Single 30s solve — effect of atol on SO2(g)")
    print(f"{'='*100}")
    print(f"{'Config':60s} | {'SO2(g)':>12s} | {'SO4²⁻':>12s} | {'total_S':>12s} | {'Steps':>8s} | {'Reject':>6s}")
    print(f"{'-'*100}")

    configs = [
        ("gas_atol=1e-3, aq_atol=1e-14, rtol=1e-6 (default)",    1e-6,  1e-3,  1e-14),
        ("gas_atol=1e-9, aq_atol=1e-14, rtol=1e-6",              1e-6,  1e-9,  1e-14),
        ("gas_atol=1e-12, aq_atol=1e-14, rtol=1e-6",             1e-6,  1e-12, 1e-14),
        ("all_atol=1e-14, rtol=1e-6",                             1e-6,  1e-14, 1e-14),
        ("all_atol=1e-14, rtol=1e-10",                            1e-10, 1e-14, 1e-14),
        ("all_atol=1e-16, rtol=1e-12",                            1e-12, 1e-16, 1e-16),
        ("all_atol=1e-18, rtol=1e-12",                            1e-12, 1e-18, 1e-18),
    ]

    initial_total_s = None
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

        state = micm.create_state()
        miam_model.set_default_parameters(state)
        state.set_conditions(temperatures=T, pressures=P)
        state.set_concentrations({
            "SO2": 3.01e-7, "H2O2": 3.01e-7,
            "CLOUD.AQUEOUS.H2O": C_H2O,
            "CLOUD.AQUEOUS.SO2_aq": 1e-12, "CLOUD.AQUEOUS.H2O2_aq": 1e-12,
            "CLOUD.AQUEOUS.Hp": 2e-6, "CLOUD.AQUEOUS.OHm": 1e-14,
            "CLOUD.AQUEOUS.HSO3m": 1e-12, "CLOUD.AQUEOUS.SO4mm": 1e-6,
            "CLOUD.AQUEOUS.SO2OOHm": 1e-30,
        })

        if initial_total_s is None:
            initial_total_s = total_sulfur(state.get_concentrations())

        result = micm.solve(state, time_step=30.0)
        c = state.get_concentrations()
        so2 = c["SO2"][0]
        so4 = c["CLOUD.AQUEOUS.SO4mm"][0]
        ts = total_sulfur(c)
        flag = " ← NEGATIVE" if so2 < 0 else ""

        print(f"  {label:58s} | {so2:+.4e} | {so4:.4e} | {ts:.4e} | {result.stats.accepted:8d} | {result.stats.rejected:6d}{flag}")

    # ── Test 2: Sub-stepping to find where overshoot occurs ──
    print(f"\n{'='*100}")
    print("Test 2: Sub-stepping (10 × 3s) with default tolerances to see trajectory")
    print(f"{'='*100}")

    tols = [1e-3] * n
    for name, idx in ordering.items():
        if "CLOUD.AQUEOUS." in name:
            tols[idx] = 1e-14

    traj, total_steps, total_rej = run_with_substeps(micm, miam_model, 30.0, 10, tols)
    print(f"{'t (s)':>8s} | {'SO2(g)':>12s} | {'SO4²⁻':>12s} | {'total_S':>12s} | {'Steps':>8s} | {'Rej':>4s} | {'OK':>3s}")
    print(f"{'-'*80}")
    for pt in traj:
        flag = " ← NEG" if pt["so2"] < 0 else ""
        print(f"  {pt['t']:6.1f} | {pt['so2']:+.4e} | {pt['so4']:.4e} | {pt['total_s']:.4e} | {pt['steps']:8d} | {pt['rejected']:4d} | {'Y' if pt['converged'] else 'N'}{flag}")
    print(f"\nTotal steps: {total_steps}, Total rejected: {total_rej}")

    # ── Analysis ──
    print(f"\n{'='*100}")
    print("ANALYSIS")
    print(f"{'='*100}")
    print("""
The Rosenbrock DAE4 error estimator computes:
  Yerror = e[0]*K[0] + e[1]*K[1] + e[2]*K[2] + e[3]*K[3]

For DAE4, e = [0, 0, 0, 1], so Yerror = K[3].

K[3] for algebraic variables is determined by solving:
  -J_a * K[3] = f_a(Y + 2*K[0] + K[2])

where f_a is the constraint residual (forcing terms from constraints).

The key question is: what is f_a at Y + 2*K[0] + K[2]?

If the solver takes a large step (H ≈ 30s), the linear approximation 
Y + 2*K[0] + K[2] may be far from the true trajectory, and f_a could 
be large. But the linear solve redistributes this error across all 
variables through the Jacobian coupling.

The practical problem is that for this stiff system:
1. The Jacobian has very large entries for fast reactions
2. J_a^{-1} * f_a may produce SMALL K[3] values for SO2 even when 
   the actual SO2 overshoot is large
3. This is because the Jacobian tells the solver "a small change in 
   SO2 causes a large change in the fast reactions" — so the error 
   estimate is dominated by the fast reaction variables, not SO2

This is a known limitation of Rosenbrock error estimators for index-1 
DAEs with stiff coupling between differential and algebraic variables.
""")
