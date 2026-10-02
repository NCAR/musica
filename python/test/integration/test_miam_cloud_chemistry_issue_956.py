# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Regression test for NCAR/musica#956: the DAE solver failed for the
# tutorial 17 cloud chemistry case at some temperatures and liquid water
# contents. The cause was NCAR/micm#1094: MIAM process terms went into the
# Jacobian rows of algebraic variables. The dissociation constraints then
# drifted during time stepping, even in the cases that converged.

import math
import pytest

import musica
import musica.mechanism_configuration as mc
from musica import backend
from musica.micm import MICM, SolverType, SolverState
from musica.micm.solver_parameters import RosenbrockSolverParameters

pytestmark = pytest.mark.skipif(not backend.miam_available(),
                                reason="MIAM backend is not available")

# ═══ Constants (tutorial 17) ═════════════════════════════════════════════════

M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556  # mol/L (unit-conversion constant only)
MW_H2O = 0.018    # kg/mol
RHO_H2O = 1000.0  # kg/m3
T0 = 298.15       # K

GAS0_SO2 = 3.01e-7    # mol/m3
GAS0_H2O2 = 3.01e-7   # mol/m3
GAS0_O3 = 1.5e-6      # mol/m3
GAS0_DMS = 3.01e-8    # mol/m3
GAS0_OH = 3.01e-13    # mol/m3
SO4MM0 = 1e-6         # mol/m3

KW = 1e-14 / (C_H2O_M * C_H2O_M)
KA1 = (1.7e-2 / C_H2O_M, 2090.0)
KA2 = (6.0e-8 / C_H2O_M, 1120.0)

# (temperature [K], pressure [Pa], liquid water content [kg/m3])
ISSUE_CASES = [
    (280.0, 70000.0, 0.3e-3),   # tutorial default
    (280.0, 85000.0, 0.3e-3),   # issue case 1
    (286.0, 85000.0, 0.3e-3),   # issue case 2: failed at t = 0
    (285.0, 85000.0, 0.3e-3),   # issue case 3
    (285.0, 85000.0, 0.03e-3),  # issue case 4: failed at t = 7170 s
    (233.0, 20000.0, 1.0e-2),   # cold, high, wet end of the requested range
    (300.0, 100000.0, 1.0e-7),  # warm, low, dry end of the requested range
]


def _equilibrium(A, C, T):
    return A * math.exp(C * (1.0 / T0 - 1.0 / T))


def _create_mechanism():
    """The tutorial 17 mechanism: gas-phase sources and the aqueous S(IV) system."""
    so2_g = mc.Species(name="SO2")
    h2o2_g = mc.Species(name="H2O2")
    o3_g = mc.Species(name="O3")
    dms = mc.Species(name="DMS")
    oh = mc.Species(name="OH")
    emitted_so2 = mc.Species(name="emitted_SO2")
    gas = mc.Phase(name="gas", species=[so2_g, h2o2_g, o3_g, dms, oh, emitted_so2])

    dms_oxidation = mc.UserDefined(name="DMS_OH_to_SO2", scaling_factor=1.0,
                                   reactants=[dms, oh], products=[so2_g], gas_phase=gas)
    so2_source = mc.Emission(name="SO2_source", scaling_factor=1.0,
                             products=[emitted_so2], gas_phase=gas)
    h2o2_source = mc.Emission(name="H2O2_source", scaling_factor=1.0,
                              products=[h2o2_g], gas_phase=gas)

    h2o = mc.Species(name="H2O")
    h2o.molecular_weight_kg_mol = MW_H2O
    so2_aq = mc.Species(name="SO2_aq")
    h2o2_aq = mc.Species(name="H2O2_aq")
    o3_aq = mc.Species(name="O3_aq")
    hp = mc.Species(name="Hp")
    ohm = mc.Species(name="OHm")
    hso3m = mc.Species(name="HSO3m")
    so3mm = mc.Species(name="SO3mm")
    so4mm = mc.Species(name="SO4mm")
    so2oohm = mc.Species(name="SO2OOHm")
    aq = mc.Phase(name="AQUEOUS", species=[
        mc.PhaseSpecies(h2o, density_kg_m3=RHO_H2O),
        so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm, so2oohm])
    all_species = [so2_g, h2o2_g, o3_g, dms, oh, emitted_so2,
                   so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm, so2oohm, h2o]
    cloud = mc.UniformSection(name="CLOUD", phases=[aq], min_radius=1e-6, max_radius=1e-5)

    processes = [
        mc.DissolvedReversibleReaction(
            phase=aq, reactants=[hso3m, h2o2_aq], products=[so2oohm, h2o], solvent=h2o,
            forward_rate_constant=mc.Equilibrium(A=C_H2O_M * (7.45e7 / 13.0), C=4430.0),
            equilibrium_constant=mc.Equilibrium(A=1725.0)),
        mc.DissolvedReaction(
            phase=aq, reactants=[so2oohm, hp], products=[so4mm], solvent=h2o,
            rate_constant=mc.Equilibrium(A=C_H2O_M * 2.4e6, C=4430.0), min_halflife=1.0),
        mc.DissolvedReaction(
            phase=aq, reactants=[hso3m, o3_aq], products=[so4mm, hp], solvent=h2o,
            rate_constant=mc.Equilibrium(A=C_H2O_M * 3.75e5, C=5530.0), min_halflife=1.0),
        mc.DissolvedReaction(
            phase=aq, reactants=[so3mm, o3_aq], products=[so4mm], solvent=h2o,
            rate_constant=mc.Equilibrium(A=C_H2O_M * 1.59e9, C=5280.0), min_halflife=1.0),
    ]

    constraints = []
    for gas_sp, aq_sp, hlc, c in [(so2_g, so2_aq, 1.23, 3120.0),
                                  (h2o2_g, h2o2_aq, 7.4e4, 6621.0),
                                  (o3_g, o3_aq, 1.15e-2, 2560.0)]:
        constraints.append(mc.HenrysLawEquilibrium(
            gas_phase=gas, gas_species=gas_sp, condensed_phase=aq, condensed_species=aq_sp,
            solvent=h2o,
            henrys_law_constant=mc.HenrysLawConstant(HLC_ref=hlc * M_ATM_TO_MOL_M3_PA, C=c),
            solvent_molecular_weight=MW_H2O, solvent_density=RHO_H2O))
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq, reactants=[h2o], products=[hp, ohm], algebraic_species=ohm, solvent=h2o,
        equilibrium_constant=mc.Equilibrium(A=KW, C=0.0)))
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq, reactants=[so2_aq], products=[hso3m, hp], algebraic_species=hso3m, solvent=h2o,
        equilibrium_constant=mc.Equilibrium(A=KA1[0], C=KA1[1])))
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq, reactants=[hso3m], products=[so3mm, hp], algebraic_species=so3mm, solvent=h2o,
        equilibrium_constant=mc.Equilibrium(A=KA2[0], C=KA2[1])))

    term = mc.LinearConstraintTerm
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=so2_g,
        terms=[term(gas, so2_g, 1.0), term(aq, so2_aq, 1.0), term(aq, hso3m, 1.0),
               term(aq, so3mm, 1.0), term(aq, so2oohm, 1.0), term(aq, so4mm, 1.0),
               term(gas, emitted_so2, -1.0)],
        constant=mc.DiagnoseFromState()))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=h2o2_g,
        terms=[term(gas, h2o2_g, 1.0), term(aq, h2o2_aq, 1.0)],
        constant=mc.DiagnoseFromState()))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=o3_g,
        terms=[term(gas, o3_g, 1.0), term(aq, o3_aq, 1.0)],
        constant=mc.DiagnoseFromState()))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=aq, algebraic_species=hp,
        terms=[term(aq, hp, 1.0), term(aq, ohm, -1.0), term(aq, hso3m, -1.0),
               term(aq, so3mm, -2.0), term(aq, so4mm, -2.0), term(aq, so2oohm, -1.0)],
        constant=mc.FixedConstant(0.0)))

    return mc.Mechanism(
        name="cam_cloud_chemistry", species=all_species, phases=[gas, aq],
        reactions=[dms_oxidation, so2_source, h2o2_source],
        aerosol=mc.Aerosol(representations=[cloud], processes=processes, constraints=constraints))


def _create_solver_and_state(temperature, pressure, lwc):
    """Set up the solver and state the same way as tutorial 17."""
    mechanism = _create_mechanism()
    micm = MICM(mechanism=mechanism, solver_type=SolverType.rosenbrock_dae4_standard_order,
                external_models=[musica.MIAM()])

    ordering = micm.create_state().get_species_ordering()
    abs_tols = [1e-3] * len(ordering)
    for name, idx in ordering.items():
        if "CLOUD.AQUEOUS." in name:
            abs_tols[idx] = 1e-8
        elif name in ("SO2", "H2O2", "O3"):
            abs_tols[idx] = 1e-9
    micm.set_solver_parameters(RosenbrockSolverParameters(
        absolute_tolerances=abs_tols, h_start=0.01, constraint_init_max_iterations=100,
        constraint_init_tolerance=1e-8, max_number_of_steps=200000))

    state = micm.create_state()
    mechanism.aerosol.set_default_parameters(state)
    state.set_conditions(temperatures=temperature, pressures=pressure)
    state.set_concentrations({
        "SO2": GAS0_SO2, "H2O2": GAS0_H2O2, "O3": GAS0_O3, "DMS": GAS0_DMS, "OH": GAS0_OH,
        "emitted_SO2": 0.0,
        "CLOUD.AQUEOUS.H2O": lwc / MW_H2O,
        "CLOUD.AQUEOUS.SO2_aq": 1e-12,
        "CLOUD.AQUEOUS.H2O2_aq": 1e-12,
        "CLOUD.AQUEOUS.O3_aq": 1e-14,
        "CLOUD.AQUEOUS.Hp": 2.0 * SO4MM0,
        "CLOUD.AQUEOUS.OHm": 1e-14,
        "CLOUD.AQUEOUS.HSO3m": 1e-12,
        "CLOUD.AQUEOUS.SO3mm": 1e-16,
        "CLOUD.AQUEOUS.SO4mm": SO4MM0,
        "CLOUD.AQUEOUS.SO2OOHm": 0.0,
    })
    k_dms = 1.1e-11 * math.exp(-240.0 / temperature) * 6.022e23 * 1e-6  # m3/(mol s)
    state.set_user_defined_rate_parameters({
        "USER.DMS_OH_to_SO2": k_dms,
        "EMIS.SO2_source": 3e-10,
        "EMIS.H2O2_source": 5e-11,
    })
    return micm, state


def _equilibrium_ratios(concs, temperature):
    """Return Q/K for the Kw, Ka1, and Ka2 constraints. Each ratio is 1 on the constraint."""
    def c(name):
        return concs[name][0]

    h2o = c("CLOUD.AQUEOUS.H2O")
    hp = c("CLOUD.AQUEOUS.Hp")
    return {
        "Kw": hp * c("CLOUD.AQUEOUS.OHm") / h2o**2 / KW,
        "Ka1": c("CLOUD.AQUEOUS.HSO3m") * hp / (c("CLOUD.AQUEOUS.SO2_aq") * h2o)
        / _equilibrium(*KA1, temperature),
        "Ka2": c("CLOUD.AQUEOUS.SO3mm") * hp / (c("CLOUD.AQUEOUS.HSO3m") * h2o)
        / _equilibrium(*KA2, temperature),
    }


def _total_sulfur(concs):
    names = ["SO2", "CLOUD.AQUEOUS.SO2_aq", "CLOUD.AQUEOUS.HSO3m", "CLOUD.AQUEOUS.SO3mm",
             "CLOUD.AQUEOUS.SO2OOHm", "CLOUD.AQUEOUS.SO4mm"]
    return sum(concs[name][0] for name in names) - concs["emitted_SO2"][0]


@pytest.mark.parametrize("temperature,pressure,lwc", ISSUE_CASES)
def test_cloud_chemistry_converges_and_keeps_constraints(temperature, pressure, lwc):
    """Integrate 2 hours of tutorial 17 and check the constraints after each 30 s step."""
    micm, state = _create_solver_and_state(temperature, pressure, lwc)

    target_time = 7200.0
    output_interval = 30.0
    total_time = 0.0
    sulfur_at_start = None
    while total_time < target_time - 1e-10:
        result = micm.solve(state, time_step=output_interval)
        assert result.state == SolverState.Converged, \
            f"Solver failed at t={total_time:.0f} s (state={result.state})"
        total_time += output_interval

        concs = state.get_concentrations()
        for name, ratio in _equilibrium_ratios(concs, temperature).items():
            assert ratio == pytest.approx(1.0, rel=1e-3), \
                f"{name} not at equilibrium at t={total_time:.0f} s: Q/K={ratio:.3e}"
        assert all(value[0] >= 0.0 for value in concs.values()), \
            f"Negative concentration at t={total_time:.0f} s"

        if sulfur_at_start is None:
            sulfur_at_start = _total_sulfur(concs)
        assert _total_sulfur(concs) == pytest.approx(sulfur_at_start, rel=1e-9)
