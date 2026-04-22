#!/usr/bin/env python3
"""Diagnostic script to trace total-S constraint behavior step by step.

Reproduces Tutorial 14's chemistry with the total-S constraint (including SO₄²⁻)
and prints the full state before/after each solve() call to diagnose why
SO₂(g) goes negative.
"""
import math
import numpy as np

import musica.mechanism_configuration as mc
from musica.micm import MICM, SolverState, SolverType
from musica.micm.solver_parameters import RosenbrockSolverParameters
from musica.miam import (
    HenrysLawConstant,
    EquilibriumConstant,
    ArrheniusRateConstant,
    UniformSection,
    DissolvedReaction,
    DissolvedReversibleReaction,
    HenryLawEquilibriumConstraint,
    DissolvedEquilibriumConstraint,
    LinearConstraint,
    LinearConstraintTerm,
    Model,
)

# ── Constants (matching tutorial exactly) ──
R_GAS = 8.314
T0 = 298.15
M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556
MW_H2O = 0.018
RHO_H2O = 1000.0

LWC = 0.3e-3
C_H2O = LWC / MW_H2O
f_v = C_H2O * MW_H2O / RHO_H2O
T_INIT = 280.0
P_INIT = 70000.0
AIR_TO_SOLUTION = f_v * 1000.0

GAS0_SO2 = 3.01e-7
GAS0_H2O2 = 3.01e-7
GAS0_O3 = 1.5e-6
GAS0_DMS = 3.01e-8
GAS0_OH = 3.01e-13
SO4MM0 = 1e-6  # test with known-good value first

# ── Gas-phase mechanism (identical to tutorial) ──
so2_g = mc.Species(name="SO2")
h2o2_g = mc.Species(name="H2O2")
o3_g = mc.Species(name="O3")
dms = mc.Species(name="DMS")
oh = mc.Species(name="OH")
emitted_so2 = mc.Species(name="emitted_SO2")

gas = mc.Phase(name="gas", species=[so2_g, h2o2_g, o3_g, dms, oh, emitted_so2])

dms_oxidation = mc.UserDefined(
    name="DMS_OH_to_SO2", scaling_factor=1.0,
    reactants=[dms, oh], products=[so2_g], gas_phase=gas,
)
so2_source = mc.Emission(
    name="SO2_source", scaling_factor=1.0, products=[emitted_so2], gas_phase=gas,
)
h2o2_source = mc.Emission(
    name="H2O2_source", scaling_factor=1.0, products=[h2o2_g], gas_phase=gas,
)

gas_mechanism = mc.Mechanism(
    name="cloud_precursors",
    species=[so2_g, h2o2_g, o3_g, dms, oh, emitted_so2],
    phases=[gas],
    reactions=[dms_oxidation, so2_source, h2o2_source],
)

# ── MIAM aqueous species (identical to tutorial) ──
h2o = mc.Species(name="H2O")
h2o.molecular_weight_kg_mol = MW_H2O
h2o.density_kg_m3 = RHO_H2O

so2_aq = mc.Species(name="SO2_aq")
h2o2_aq = mc.Species(name="H2O2_aq")
o3_aq = mc.Species(name="O3_aq")
hp = mc.Species(name="Hp")
ohm = mc.Species(name="OHm")
hso3m = mc.Species(name="HSO3m")
so3mm = mc.Species(name="SO3mm")
so4mm = mc.Species(name="SO4mm")
so2oohm = mc.Species(name="SO2OOHm")

aq_phase = mc.Phase(
    name="AQUEOUS",
    species=[h2o, so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm, so2oohm],
)

all_species = [
    so2_g, h2o2_g, o3_g, dms, oh, emitted_so2,
    so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm, so2oohm, h2o,
]

cloud = UniformSection(
    name="CLOUD",
    phase_names=["AQUEOUS"],
    min_radius=1e-6,
    max_radius=1e-5,
)

# ── Kinetic reactions (identical to tutorial) ──
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
r2 = DissolvedReaction(
    phase_name="AQUEOUS",
    reactant_names=["HSO3m", "O3_aq"],
    product_names=["SO4mm", "Hp"],
    solvent_name="H2O",
    rate_constant=ArrheniusRateConstant(a=C_H2O_M * 3.75e5, c=5530.0),
)
r3 = DissolvedReaction(
    phase_name="AQUEOUS",
    reactant_names=["SO3mm", "O3_aq"],
    product_names=["SO4mm"],
    solvent_name="H2O",
    rate_constant=ArrheniusRateConstant(a=C_H2O_M * 1.59e9, c=5280.0),
)

# ── Constraints (identical to tutorial except total-S) ──
constraints = []

# Henry's Law
henry_law_species = [
    ("SO2",  "SO2_aq",  1.23,    3120.0),
    ("H2O2", "H2O2_aq", 7.4e4,   6621.0),
    ("O3",   "O3_aq",   1.15e-2, 2560.0),
]
for gas_name, aq_name, hlc_ref_lit, c in henry_law_species:
    constraints.append(HenryLawEquilibriumConstraint(
        gas_species_name=gas_name,
        condensed_species_name=aq_name,
        solvent_name="H2O",
        condensed_phase_name="AQUEOUS",
        henrys_law_constant=HenrysLawConstant(
            hlc_ref=hlc_ref_lit * M_ATM_TO_MOL_M3_PA,
            c=c,
        ),
        mw_solvent=MW_H2O,
        rho_solvent=RHO_H2O,
    ))

# Dissociation
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
constraints.append(DissolvedEquilibriumConstraint(
    phase_name="AQUEOUS", reactant_names=["HSO3m"], product_names=["SO3mm", "Hp"],
    algebraic_species_name="SO3mm", solvent_name="H2O",
    equilibrium_constant=EquilibriumConstant(a=6.0e-8 / C_H2O_M, c=1495.0),
))

# ════════════════════════════════════════════════════════════════════════
# TOTAL SULFUR conservation (including SO₄²⁻)
# SO₂(g) is algebraic: SO₂ = C + emitted_SO₂ - aq_S(IV) - SO₄²⁻
# ════════════════════════════════════════════════════════════════════════
constraints.append(LinearConstraint(
    algebraic_phase_name="gas",
    algebraic_species_name="SO2",
    terms=[
        LinearConstraintTerm("gas", "SO2", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO2_aq", 1.0),
        LinearConstraintTerm("AQUEOUS", "HSO3m", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO3mm", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO2OOHm", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO4mm", 1.0),      # total S
        LinearConstraintTerm("gas", "emitted_SO2", -1.0),
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

# Total O3
constraints.append(LinearConstraint(
    algebraic_phase_name="gas",
    algebraic_species_name="O3",
    terms=[
        LinearConstraintTerm("gas", "O3", 1.0),
        LinearConstraintTerm("AQUEOUS", "O3_aq", 1.0),
    ],
    diagnose_from_state=True,
))

# Charge balance
constraints.append(LinearConstraint(
    algebraic_phase_name="AQUEOUS",
    algebraic_species_name="Hp",
    terms=[
        LinearConstraintTerm("AQUEOUS", "Hp", 1.0),
        LinearConstraintTerm("AQUEOUS", "OHm", -1.0),
        LinearConstraintTerm("AQUEOUS", "HSO3m", -1.0),
        LinearConstraintTerm("AQUEOUS", "SO3mm", -2.0),
        LinearConstraintTerm("AQUEOUS", "SO4mm", -2.0),
        LinearConstraintTerm("AQUEOUS", "SO2OOHm", -1.0),
    ],
    constant=0.0,
))

# ── Assemble MIAM model ──
miam_model = Model(
    name="cam_cloud_chemistry",
    species=all_species,
    condensed_phases=[aq_phase],
    representations=[cloud],
    processes=[r1a, r1b, r2, r3],
    constraints=constraints,
)

# ── Create solver ──
micm = MICM(
    mechanism=gas_mechanism,
    solver_type=SolverType.rosenbrock_dae4_standard_order,
    external_models=[miam_model],
)

tmp_state = micm.create_state()
ordering = tmp_state.get_species_ordering()
print(f"Species ordering: {ordering}")

# Set tolerances
abs_tols = [1e-3] * len(ordering)
for name, idx in ordering.items():
    if "CLOUD.AQUEOUS." in name:
        abs_tols[idx] = 1e-14

micm.set_solver_parameters(RosenbrockSolverParameters(
    relative_tolerance=1e-12,
    absolute_tolerances=abs_tols,
    h_start=0.0,
    h_max=0.0,
    constraint_init_max_iterations=100,
    constraint_init_tolerance=1e-8,
    max_number_of_steps=500000,
))

state = micm.create_state()
miam_model.set_default_parameters(state)
state.set_conditions(temperatures=T_INIT, pressures=P_INIT)

# ── Initial conditions ──
ics = {
    "SO2": GAS0_SO2,
    "H2O2": GAS0_H2O2,
    "O3": GAS0_O3,
    "DMS": GAS0_DMS,
    "OH": GAS0_OH,
    "emitted_SO2": 0.0,
    "CLOUD.AQUEOUS.H2O": C_H2O,
    "CLOUD.AQUEOUS.SO2_aq": 1e-12,
    "CLOUD.AQUEOUS.H2O2_aq": 1e-12,
    "CLOUD.AQUEOUS.O3_aq": 1e-14,
    "CLOUD.AQUEOUS.Hp": 2.0 * SO4MM0,
    "CLOUD.AQUEOUS.OHm": 1e-14,
    "CLOUD.AQUEOUS.HSO3m": 1e-12,
    "CLOUD.AQUEOUS.SO3mm": 1e-16,
    "CLOUD.AQUEOUS.SO4mm": SO4MM0,
    "CLOUD.AQUEOUS.SO2OOHm": 1e-30,
}
state.set_concentrations(ics)

# Rate parameters
k_dms = 1.1e-11 * math.exp(-240.0 / T_INIT)
k_dms_musica = k_dms * 6.022e23 * 1e-6
emis_so2 = 3e-10
emis_h2o2 = 5e-11

state.set_user_defined_rate_parameters({
    "USER.DMS_OH_to_SO2": k_dms_musica,
    "EMIS.SO2_source": emis_so2,
    "EMIS.H2O2_source": emis_h2o2,
})

# ── Diagnostic helper ──
KEY_SPECIES = [
    "SO2", "H2O2", "O3", "DMS", "OH", "emitted_SO2",
    "CLOUD.AQUEOUS.SO2_aq", "CLOUD.AQUEOUS.H2O2_aq", "CLOUD.AQUEOUS.O3_aq",
    "CLOUD.AQUEOUS.Hp", "CLOUD.AQUEOUS.OHm",
    "CLOUD.AQUEOUS.HSO3m", "CLOUD.AQUEOUS.SO3mm", "CLOUD.AQUEOUS.SO4mm",
    "CLOUD.AQUEOUS.SO2OOHm", "CLOUD.AQUEOUS.H2O",
]

def print_state(label, concs):
    print(f"\n{'='*60}")
    print(f"  {label}")
    print(f"{'='*60}")
    for sp in KEY_SPECIES:
        val = concs[sp][0]
        print(f"  {sp:35s} = {val:+.6e}")

    # Compute derived diagnostics
    so2g = concs["SO2"][0]
    so2aq = concs["CLOUD.AQUEOUS.SO2_aq"][0]
    hso3 = concs["CLOUD.AQUEOUS.HSO3m"][0]
    so3 = concs["CLOUD.AQUEOUS.SO3mm"][0]
    so2ooh = concs["CLOUD.AQUEOUS.SO2OOHm"][0]
    so4 = concs["CLOUD.AQUEOUS.SO4mm"][0]
    emit = concs["emitted_SO2"][0]
    hp_val = concs["CLOUD.AQUEOUS.Hp"][0]

    siv = so2g + so2aq + hso3 + so3 + so2ooh
    total_s = siv + so4
    total_s_constraint = total_s - emit

    print(f"\n  S(IV) pool:     {siv:+.6e}")
    print(f"  SO₄²⁻:         {so4:+.6e}")
    print(f"  Total S:        {total_s:+.6e}")
    print(f"  emitted_SO2:    {emit:+.6e}")
    print(f"  Constraint val: {total_s_constraint:+.6e}  (should be constant)")

    if hp_val > 0:
        pH = -math.log10(hp_val / AIR_TO_SOLUTION)
        print(f"  pH:             {pH:.3f}")
    else:
        print(f"  pH:             NEGATIVE H+ ({hp_val:.3e})")

# ── Focused test: single 30s solve with various tolerances ──
print(f"\n{'='*80}")
print("  TOLERANCE SWEEP: single 30s solve, SO₂(g) algebraic, total-S constraint")
print(f"{'='*80}")

# Test different tolerance setups - zoom in on the transition
configs = [
    ("atol=1e-18, rtol=1e-13", 1e-13, 1e-18),
    ("atol=1e-18, rtol=1e-14", 1e-14, 1e-18),
    ("atol=1e-19, rtol=1e-12", 1e-12, 1e-19),
    ("atol=1e-19, rtol=1e-13", 1e-13, 1e-19),
    ("atol=1e-19, rtol=1e-14", 1e-14, 1e-19),
    ("atol=1e-20, rtol=1e-12", 1e-12, 1e-20),
    ("atol=1e-20, rtol=1e-13", 1e-13, 1e-20),
    ("atol=1e-20, rtol=1e-14", 1e-14, 1e-20),
]

for label, rtol, atol_val in configs:
    tols = [atol_val] * len(ordering)

    micm.set_solver_parameters(RosenbrockSolverParameters(
        relative_tolerance=rtol,
        absolute_tolerances=tols,
        constraint_init_max_iterations=100,
        constraint_init_tolerance=1e-8,
        max_number_of_steps=5000000,
    ))

    st = micm.create_state()
    miam_model.set_default_parameters(st)
    st.set_conditions(temperatures=T_INIT, pressures=P_INIT)
    st.set_concentrations(ics)
    st.set_user_defined_rate_parameters({
        "USER.DMS_OH_to_SO2": k_dms_musica,
        "EMIS.SO2_source": emis_so2,
        "EMIS.H2O2_source": emis_h2o2,
    })

    res = micm.solve(st, time_step=30.0)
    c = st.get_concentrations()
    so2 = c["SO2"][0]
    so4 = c["CLOUD.AQUEOUS.SO4mm"][0]
    emit = c["emitted_SO2"][0]
    total_s = so2 + c["CLOUD.AQUEOUS.SO2_aq"][0] + c["CLOUD.AQUEOUS.HSO3m"][0] + c["CLOUD.AQUEOUS.SO3mm"][0] + c["CLOUD.AQUEOUS.SO2OOHm"][0] + so4
    print(f"  {label:55s} | SO₂={so2:+.4e} SO₄²⁻={so4:.4e} steps={res.stats.accepted:6d} rejects={res.stats.rejected:4d} state={res.state}")

print("\nDone.")
