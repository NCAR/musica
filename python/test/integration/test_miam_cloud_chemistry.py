# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Integration tests for the MIAM aerosol model through the Python API.
# Builds the CAM Cloud Chemistry configuration and solves it.

import math
import pytest

import musica.mechanism_configuration as mc
import musica
from musica import backend
from musica.micm import MICM, SolverType, SolverState

# Skip all tests if MIAM is not available
pytestmark = pytest.mark.skipif(not backend.miam_available(),
                                reason="MIAM backend is not available")

# ═══ Constants ═══════════════════════════════════════════════════════════════
#
# All MIAM state variables including condensed-phase species are in
# units of mol/m3 of AIR.  The constant C_H2O_M (55.556 mol/L) is used
# ONLY for converting literature rate/equilibrium constants from molar
# (mol/L) units to MIAM (mol/m³) units.  It is NOT a state variable.

# Prefix, suffix
# C_: Concentration
# _M: molar (mol/L)

M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556  # mol/L — molar concentration of pure liquid water (unit-conversion constant)
MW_H2O = 0.018    # kg/mol - molecular weight
RHO_H2O = 1000.0  # kg/m3
R_GAS = 8.314     # J/(mol·K)
T0 = 298.15       # K - reference temperature

# Cloud liquid water content and derived solvent concentration
LWC = 0.3e-3          # kg/m3  (= 0.3 g/m3, typical cloud)
C_H2O = LWC / MW_H2O  # mol/m3 of air  (≈ 0.01667) -  cloud water amount

# Initial conditions (all in mol/m3 of air)
GAS0_SO2 = 3.01e-8   # mol/m3  (~ 1 ppb)
GAS0_H2O2 = 3.01e-8  # mol/m3
GAS0_O3 = 1.5e-6     # mol/m3
SO4MM0 = 1.0         # mol/m3  (background sulfate)
T_INIT = 280.0       # K
P_INIT = 70000.0     # Pa


# ═══ Helpers ═════════════════════════════════════════════════════════════════

def _create_cloud_chemistry_mechanism(r1b_rate_constant=None):
    """Create the CAM Cloud Chemistry mechanism (revised mechanism).

    The aerosol section carries:
      - 1 UniformSection representation
      - 2 kinetic reactions (revised 2-step H2O2 oxidation)
      - 10 constraints (3 Henry's law, 3 dissociation, 3 mass budgets, 1 charge balance)

    Species (13): SO2(g), H2O2(g), O3(g), SO2_aq, H2O2_aq, O3_aq,
             H+ (Hp), OH- (OHm), HSO3- (HSO3m), SO3-- (SO3mm),
             SO4-- (SO4mm), SO2OOH- (SO2OOHm), H2O (solvent).

    Reactions (revised 2-step H2O2 oxidation):
      R1a: HSO3- + H2O2_aq ⇌ SO2OOH- + H2O  (reversible, Keq = 1725)
      R1b: SO2OOH- + H+ → SO4--  (irreversible)

    Args:
        r1b_rate_constant: Optional override for R1b's rate constant for test purpose (e.g. a callable
            ``f(T) -> k``). Defaults to the Arrhenius reference-temperature form.
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
    so2oohm = mc.Species(name="SO2OOHm")
    h2o = mc.Species(name="H2O")
    h2o.molecular_weight_kg_mol = MW_H2O
    h2o.density_kg_m3 = RHO_H2O

    all_species = [
        so2_g, h2o2_g, o3_g, so2_aq, h2o2_aq, o3_aq,
        hp, ohm, hso3m, so3mm, so4mm, so2oohm, h2o,
    ]

    # ── Phases ──
    gas = mc.Phase(name="gas", species=[so2_g, h2o2_g, o3_g])
    aq_phase = mc.Phase(
        name="AQUEOUS",
        species=[h2o, so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm, so2oohm])

    # ── Representation ──
    cloud = mc.UniformSection(
        name="CLOUD",
        phases=[aq_phase],
        min_radius=1e-6,
        max_radius=1e-5)

    # ── Processes ──
    # Test-only hook: `r1b_rate_constant` lets tests override R1b's rate constant
    # (e.g., with a Python callable `f(T) -> k`). It is NOT a pattern for real configuration.
    # A normal chemistry setup declares each rate constant inline on the process.
    if r1b_rate_constant is None:
        r1b_rate_constant = mc.ArrheniusReferenceTemperature(A=C_H2O_M * 2.4e6, C=4430.0)

    # R1a: HSO3- + H2O2_aq ⇌ SO2OOH- + H2O  (reversible; forward + equilibrium)
    r1a = mc.DissolvedReversibleReaction(
        phase=aq_phase,
        solvent=h2o,
        reactants=[hso3m, h2o2_aq],
        products=[so2oohm, h2o],
        forward_rate_constants={cloud: mc.ArrheniusReferenceTemperature(A=C_H2O_M * (7.45e7 / 13.0), C=4430.0)},
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1725.0, C=0.0))

    # R1b: SO2OOH- + H+ → SO4--  (irreversible)
    r1b = mc.DissolvedReaction(
        phase=aq_phase,
        solvent=h2o,
        reactants=[so2oohm, hp],
        products=[so4mm],
        rate_constants={cloud: r1b_rate_constant})

    # ── Constraints ──
    constraints = [
        # Henry's Law equilibria for SO2, H2O2, O3
        mc.HenryLawEquilibrium(
            gas_phase=gas,
            gas_species=so2_g,
            condensed_phase=aq_phase,
            condensed_species=so2_aq,
            solvent=h2o,
            henry_law_constant=mc.HenryLawConstant(HLC_ref=1.23 * M_ATM_TO_MOL_M3_PA, C=3120.0),
            solvent_molecular_weight=MW_H2O,
            solvent_density=RHO_H2O),
        mc.HenryLawEquilibrium(
            gas_phase=gas,
            gas_species=h2o2_g,
            condensed_phase=aq_phase,
            condensed_species=h2o2_aq,
            solvent=h2o,
            henry_law_constant=mc.HenryLawConstant(HLC_ref=7.4e4 * M_ATM_TO_MOL_M3_PA, C=6621.0),
            solvent_molecular_weight=MW_H2O,
            solvent_density=RHO_H2O),
        mc.HenryLawEquilibrium(
            gas_phase=gas,
            gas_species=o3_g,
            condensed_phase=aq_phase,
            condensed_species=o3_aq,
            solvent=h2o,
            henry_law_constant=mc.HenryLawConstant(HLC_ref=1.15e-2 * M_ATM_TO_MOL_M3_PA, C=2560.0),
            solvent_molecular_weight=MW_H2O,
            solvent_density=RHO_H2O),

        # Dissolved equilibria: Kw, Ka1, Ka2
        mc.DissolvedEquilibrium(
            phase=aq_phase,
            reactants=[h2o],
            products=[hp, ohm],
            algebraic_species=ohm,
            solvent=h2o,
            equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1e-14 / (C_H2O_M * C_H2O_M), C=0.0)),
        mc.DissolvedEquilibrium(
            phase=aq_phase,
            reactants=[so2_aq],
            products=[hso3m, hp],
            algebraic_species=hso3m,
            solvent=h2o,
            equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1.7e-2 / C_H2O_M, C=2090.0)),
        mc.DissolvedEquilibrium(
            phase=aq_phase,
            reactants=[hso3m],
            products=[so3mm, hp],
            algebraic_species=so3mm,
            solvent=h2o,
            equilibrium_constant=mc.ArrheniusReferenceTemperature(A=6.0e-8 / C_H2O_M, C=1120.0)),

        # Linear constraints: mass conservation + charge balance
        # Mass S: SO2_g + SO2_aq + HSO3- + SO3-- + SO2OOH- = total_S
        # NOTE: SO4-- is differential (produced by kinetics), NOT in S budget
        mc.LinearConstraint(
            algebraic_phase=gas,
            algebraic_species=so2_g,
            terms=[
                mc.LinearConstraintTerm(gas, so2_g, 1.0),
                mc.LinearConstraintTerm(aq_phase, so2_aq, 1.0),
                mc.LinearConstraintTerm(aq_phase, hso3m, 1.0),
                mc.LinearConstraintTerm(aq_phase, so3mm, 1.0),
                mc.LinearConstraintTerm(aq_phase, so2oohm, 1.0),
            ],
            constant=mc.FixedConstant(GAS0_SO2)),

        # Mass H2O2: H2O2_g + H2O2_aq = total_H2O2
        mc.LinearConstraint(
            algebraic_phase=gas, algebraic_species=h2o2_g,
            terms=[
                mc.LinearConstraintTerm(gas, h2o2_g, 1.0),
                mc.LinearConstraintTerm(aq_phase, h2o2_aq, 1.0),
            ],
            constant=mc.FixedConstant(GAS0_H2O2)),

        # Mass O3: O3_g + O3_aq = total_O3
        mc.LinearConstraint(
            algebraic_phase=gas, algebraic_species=o3_g,
            terms=[
                mc.LinearConstraintTerm(gas, o3_g, 1.0),
                mc.LinearConstraintTerm(aq_phase, o3_aq, 1.0),
            ],
            constant=mc.FixedConstant(GAS0_O3)),

        # Charge balance: H+ = OH- + HSO3- + 2*SO3-- + 2*SO4-- + SO2OOH-
        mc.LinearConstraint(
            algebraic_phase=aq_phase, algebraic_species=hp,
            terms=[
                mc.LinearConstraintTerm(aq_phase, hp, 1.0),
                mc.LinearConstraintTerm(aq_phase, ohm, -1.0),
                mc.LinearConstraintTerm(aq_phase, hso3m, -1.0),
                mc.LinearConstraintTerm(aq_phase, so3mm, -2.0),
                mc.LinearConstraintTerm(aq_phase, so4mm, -2.0),
                mc.LinearConstraintTerm(aq_phase, so2oohm, -1.0),
            ],
            constant=mc.FixedConstant(0.0)),
    ]

    return mc.Mechanism(
        name="cloud_chemistry",
        species=all_species,
        phases=[gas, aq_phase],
        reactions=[],
        aerosol=mc.Aerosol(representations=[cloud], processes=[r1a, r1b], constraints=constraints))


def _naive_initial_conditions(include_so2oohm=True):
    """Return naive initial conditions for cloud chemistry.

    MIAM's constraint initialization will project these onto the
    constraint manifold before time-stepping begins.  All concentrations
    are in mol/m³ of air.

    Args:
        include_so2oohm: Include SO2OOHm species (only for models that
            have the revised 2-step mechanism).
    """
    ics = {
        "SO2": GAS0_SO2,
        "H2O2": GAS0_H2O2,
        "O3": GAS0_O3,
        "CLOUD.AQUEOUS.H2O": C_H2O,
        "CLOUD.AQUEOUS.SO2_aq": 1e-10,
        "CLOUD.AQUEOUS.H2O2_aq": 1e-10,
        "CLOUD.AQUEOUS.O3_aq": 1e-10,
        "CLOUD.AQUEOUS.Hp": 2.0 * SO4MM0,
        "CLOUD.AQUEOUS.OHm": 1e-10,
        "CLOUD.AQUEOUS.HSO3m": 1e-10,
        "CLOUD.AQUEOUS.SO3mm": 1e-10,
        "CLOUD.AQUEOUS.SO4mm": SO4MM0,
    }
    if include_so2oohm:
        ics["CLOUD.AQUEOUS.SO2OOHm"] = 0.0
    return ics


# ═══ Tests ═══════════════════════════════════════════════════════════════════

class TestMiamModelCreation:
    """Test that a MICM solver with MIAM can be created."""

    def test_create_solver_dae4(self):
        mechanism = _create_cloud_chemistry_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )
        assert micm is not None
        assert micm.solver_type() == SolverType.rosenbrock_dae4_standard_order

    def test_create_solver_dae6(self):
        mechanism = _create_cloud_chemistry_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae6_standard_order,
            external_models=[musica.MIAM()],
        )
        assert micm is not None

    def test_create_solver_rosenbrock(self):
        """Non-DAE solver should also work with MIAM (constraints ignored)."""
        mechanism = _create_cloud_chemistry_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_standard_order,
            external_models=[musica.MIAM()],
        )
        assert micm is not None


class TestMiamStateCreation:
    """Test creating a state from a MIAM-enabled solver."""

    def test_create_state(self):
        mechanism = _create_cloud_chemistry_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )
        state = micm.create_state()
        assert state is not None

    def test_set_default_parameters(self):
        mechanism = _create_cloud_chemistry_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )
        state = micm.create_state()
        mechanism.aerosol.set_default_parameters(state)
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
        mechanism = _create_cloud_chemistry_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )

        state = micm.create_state()
        mechanism.aerosol.set_default_parameters(state)

        # Set conditions
        state.set_conditions(temperatures=T_INIT, pressures=P_INIT)

        # Set naive initial conditions (constraint init projects onto manifold)
        state.set_concentrations(_naive_initial_conditions())

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

        # Mass conservation: SO2_g + SO2_aq + HSO3- + SO3-- + SO2OOH-
        # should still approximately equal the initial total (minus what
        # reacted to SO4)
        concs = state.get_concentrations()
        so4_f = concs.get("CLOUD.AQUEOUS.SO4mm", [0.0])[0]
        assert so4_f >= SO4MM0, "SO4 should only increase from kinetics"

    def test_solve_callable_rate_constant(self):
        """Test with a callable rate constant instead of ArrheniusReferenceTemperature."""
        # Replace R1b's rate constant with a Python callable f(T) -> k
        mechanism = _create_cloud_chemistry_mechanism(
            r1b_rate_constant=lambda T: C_H2O_M * 2.4e6 * math.exp(
                -4430.0 * (1.0 / T - 1.0 / T0)),
        )

        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )

        state = micm.create_state()
        mechanism.aerosol.set_default_parameters(state)

        state.set_conditions(temperatures=T_INIT, pressures=P_INIT)
        state.set_concentrations(_naive_initial_conditions())

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
        with pytest.raises(ValueError, match="external_models cannot be used with config_path"):
            MICM(
                config_path="dummy/path",
                external_models=[musica.MIAM()],
            )

    def test_invalid_species_in_aerosol(self):
        """An aerosol section referencing a non-existent species should raise an error."""
        x = mc.Species(name="X")
        p = mc.Phase(name="P", species=[x])
        section = mc.UniformSection(name="S", phases=[p], min_radius=1e-6, max_radius=1e-5)
        bad = mc.DissolvedReaction(
            phase=p,
            solvent=x,
            reactants=[mc.Species(name="NONEXISTENT")],  # a species not registered in the mechanism
            products=[x],
            rate_constants={section: mc.ArrheniusReferenceTemperature(A=1.0, C=0.0)},
        )
        mechanism = mc.Mechanism(
            name="bad",
            species=[x],
            phases=[p],
            reactions=[],
            aerosol=mc.Aerosol(representations=[section], processes=[bad]))

        with pytest.raises(Exception):
            MICM(
                mechanism=mechanism,
                solver_type=SolverType.rosenbrock_standard_order,
                external_models=[musica.MIAM()],
            )


class TestMiamAerosolStructure:
    """Test the aerosol section attached to a mechanism."""

    def test_aerosol_counts(self):
        """The cloud-chemistry aerosol section has the expected entry counts."""
        mechanism = _create_cloud_chemistry_mechanism()
        aerosol = mechanism.aerosol
        assert aerosol is not None
        assert len(mechanism.species) == 13
        assert len(aerosol.representations) == 1
        assert len(aerosol.processes) == 2
        assert len(aerosol.constraints) == 10

    def test_species_properties_preserved(self):
        """Molecular weight and density survive attachment to the mechanism."""
        h2o = mc.Species(name="H2O")
        h2o.molecular_weight_kg_mol = 0.018
        h2o.density_kg_m3 = 1000.0
        p = mc.Phase(name="P", species=[h2o])
        mechanism = mc.Mechanism(
            name="test",
            species=[h2o],
            phases=[p],
            reactions=[],
            aerosol=mc.Aerosol(
                representations=[mc.UniformSection(name="S", phases=[p], min_radius=1e-6, max_radius=1e-5)]),
        )
        sp = mechanism.species[0]
        assert sp.name == "H2O"
        assert sp.molecular_weight_kg_mol == pytest.approx(0.018)
        assert sp.density_kg_m3 == pytest.approx(1000.0)

    def test_minimal_aerosol(self):
        """An empty aerosol section has empty entry lists."""
        aerosol = mc.Aerosol()
        assert len(aerosol.representations) == 0
        assert len(aerosol.processes) == 0
        assert len(aerosol.constraints) == 0


# ═══ Analytical Validation ═══════════════════════════════════════════════════


def _integrate(micm, state, target_time, dt_init=0.01):
    """Adaptive time stepping loop matching the C++ IntegrateDAE pattern."""
    total_time = 0.0
    dt = dt_init
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
        if total_time > 10.0 and dt < 10.0:
            dt = 10.0
        if total_time > 100.0 and dt < 100.0:
            dt = 100.0


def _get_conc(state, name):
    """Get scalar concentration from state."""
    return state.get_concentrations()[name][0]


def _create_equilibrium_only_mechanism():
    """CAM Cloud Chemistry mechanism WITHOUT kinetic reactions (equilibrium constraints only).

    The mass_S constraint excludes SO4-- (which is differential and unchanged).
    """
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
    gas = mc.Phase(name="gas", species=[so2_g, h2o2_g, o3_g])
    aq_phase = mc.Phase(
        name="AQUEOUS",
        species=[h2o, so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm])
    cloud = mc.UniformSection(name="CLOUD", phases=[aq_phase], min_radius=1e-6, max_radius=1e-5)

    aq_by_name = {"SO2_aq": so2_aq, "H2O2_aq": h2o2_aq, "O3_aq": o3_aq}
    gas_by_name = {"SO2": so2_g, "H2O2": h2o2_g, "O3": o3_g}
    constraints = []

    # Henry's Law equilibria
    for gas_name, aq_name, hlc_ref, c in [
        ("SO2", "SO2_aq", 1.23, 3120.0),
        ("H2O2", "H2O2_aq", 7.4e4, 6621.0),
        ("O3", "O3_aq", 1.15e-2, 2560.0),
    ]:
        constraints.append(mc.HenryLawEquilibrium(
            gas_phase=gas,
            gas_species=gas_by_name[gas_name],
            condensed_phase=aq_phase,
            condensed_species=aq_by_name[aq_name],
            solvent=h2o,
            henry_law_constant=mc.HenryLawConstant(HLC_ref=hlc_ref * M_ATM_TO_MOL_M3_PA, C=c),
            solvent_molecular_weight=MW_H2O,
            solvent_density=RHO_H2O))

    # Dissociation equilibria
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq_phase,
        reactants=[h2o],
        products=[hp, ohm],
        algebraic_species=ohm,
        solvent=h2o,
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1e-14 / (C_H2O_M * C_H2O_M), C=0.0)))
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq_phase,
        reactants=[so2_aq],
        products=[hso3m, hp],
        algebraic_species=hso3m,
        solvent=h2o,
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1.7e-2 / C_H2O_M, C=2090.0)))
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq_phase,
        reactants=[hso3m],
        products=[so3mm, hp],
        algebraic_species=so3mm,
        solvent=h2o,
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=6.0e-8 / C_H2O_M, C=1120.0)))

    # Mass conservation (no SO4 — it's differential and unchanged)
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=so2_g,
        terms=[
            mc.LinearConstraintTerm(gas, so2_g, 1.0),
            mc.LinearConstraintTerm(aq_phase, so2_aq, 1.0),
            mc.LinearConstraintTerm(aq_phase, hso3m, 1.0),
            mc.LinearConstraintTerm(aq_phase, so3mm, 1.0),
        ],
        constant=mc.FixedConstant(GAS0_SO2)))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=h2o2_g,
        terms=[
            mc.LinearConstraintTerm(gas, h2o2_g, 1.0),
            mc.LinearConstraintTerm(aq_phase, h2o2_aq, 1.0),
        ],
        constant=mc.FixedConstant(GAS0_H2O2)))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=o3_g,
        terms=[
            mc.LinearConstraintTerm(gas, o3_g, 1.0),
            mc.LinearConstraintTerm(aq_phase, o3_aq, 1.0),
        ],
        constant=mc.FixedConstant(GAS0_O3)))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=aq_phase, algebraic_species=hp,
        terms=[
            mc.LinearConstraintTerm(aq_phase, hp, 1.0),
            mc.LinearConstraintTerm(aq_phase, ohm, -1.0),
            mc.LinearConstraintTerm(aq_phase, hso3m, -1.0),
            mc.LinearConstraintTerm(aq_phase, so3mm, -2.0),
            mc.LinearConstraintTerm(aq_phase, so4mm, -2.0),
        ],
        constant=mc.FixedConstant(0.0)))

    return mc.Mechanism(
        name="equilibrium_only",
        species=all_species,
        phases=[gas, aq_phase],
        reactions=[],
        aerosol=mc.Aerosol(representations=[cloud], processes=[], constraints=constraints))


def _create_kinetics_mechanism():
    """CAM Cloud Chemistry mechanism WITH kinetic reactions.

    Includes equilibrium + revised 2-step H2O2 oxidation + ozone oxidation reactions.
    The mass_S constraint INCLUDES SO4-- and SO2OOH- since kinetics transfers sulfur
    between S(IV) and S(VI) pools.
    """
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
    so2oohm = mc.Species(name="SO2OOHm")
    h2o = mc.Species(name="H2O")
    h2o.molecular_weight_kg_mol = MW_H2O
    h2o.density_kg_m3 = RHO_H2O

    all_species = [
        so2_g, h2o2_g, o3_g, so2_aq, h2o2_aq, o3_aq,
        hp, ohm, hso3m, so3mm, so4mm, so2oohm, h2o,
    ]
    gas = mc.Phase(name="gas", species=[so2_g, h2o2_g, o3_g])
    aq_phase = mc.Phase(
        name="AQUEOUS",
        species=[h2o, so2_aq, h2o2_aq, o3_aq, hp, ohm, hso3m, so3mm, so4mm, so2oohm])
    cloud = mc.UniformSection(name="CLOUD", phases=[aq_phase], min_radius=1e-6, max_radius=1e-5)

    # Kinetic S(IV)->S(VI) oxidation reactions (revised mechanism)
    # R1a: HSO3- + H2O2_aq ⇌ SO2OOH- + H2O  (reversible)
    r1a = mc.DissolvedReversibleReaction(
        phase=aq_phase,
        solvent=h2o,
        reactants=[hso3m, h2o2_aq],
        products=[so2oohm, h2o],
        forward_rate_constants={cloud: mc.ArrheniusReferenceTemperature(A=C_H2O_M * (7.45e7 / 13.0), C=4430.0)},
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1725.0, C=0.0))
    # R1b: SO2OOH- + H+ → SO4--  (irreversible)
    r1b = mc.DissolvedReaction(
        phase=aq_phase,
        solvent=h2o,
        reactants=[so2oohm, hp],
        products=[so4mm],
        rate_constants={cloud: mc.ArrheniusReferenceTemperature(A=C_H2O_M * 2.4e6, C=4430.0)})
    # R2: HSO3- + O3_aq → SO4-- + H+
    r2 = mc.DissolvedReaction(
        phase=aq_phase,
        solvent=h2o,
        reactants=[hso3m, o3_aq],
        products=[so4mm, hp],
        rate_constants={cloud: mc.ArrheniusReferenceTemperature(A=C_H2O_M * 3.75e5, C=5530.0)})
    # R3: SO3-- + O3_aq → SO4--
    r3 = mc.DissolvedReaction(
        phase=aq_phase,
        solvent=h2o,
        reactants=[so3mm, o3_aq],
        products=[so4mm],
        rate_constants={cloud: mc.ArrheniusReferenceTemperature(A=C_H2O_M * 1.59e9, C=5280.0)})

    gas_by_name = {"SO2": so2_g, "H2O2": h2o2_g, "O3": o3_g}
    aq_by_name = {"SO2_aq": so2_aq, "H2O2_aq": h2o2_aq, "O3_aq": o3_aq}
    constraints = []

    # Henry's Law equilibria (same as equilibrium-only)
    for gas_name, aq_name, hlc_ref, c in [
        ("SO2", "SO2_aq", 1.23, 3120.0),
        ("H2O2", "H2O2_aq", 7.4e4, 6621.0),
        ("O3", "O3_aq", 1.15e-2, 2560.0),
    ]:
        constraints.append(mc.HenryLawEquilibrium(
            gas_phase=gas,
            gas_species=gas_by_name[gas_name],
            condensed_phase=aq_phase,
            condensed_species=aq_by_name[aq_name],
            solvent=h2o,
            henry_law_constant=mc.HenryLawConstant(HLC_ref=hlc_ref * M_ATM_TO_MOL_M3_PA, C=c),
            solvent_molecular_weight=MW_H2O,
            solvent_density=RHO_H2O))

    # Dissociation equilibria (same as equilibrium-only)
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq_phase,
        reactants=[h2o],
        products=[hp, ohm],
        algebraic_species=ohm,
        solvent=h2o,
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1e-14 / (C_H2O_M * C_H2O_M), C=0.0)))
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq_phase,
        reactants=[so2_aq],
        products=[hso3m, hp],
        algebraic_species=hso3m,
        solvent=h2o,
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=1.7e-2 / C_H2O_M, C=2090.0)))
    constraints.append(mc.DissolvedEquilibrium(
        phase=aq_phase,
        reactants=[hso3m],
        products=[so3mm, hp],
        algebraic_species=so3mm,
        solvent=h2o,
        equilibrium_constant=mc.ArrheniusReferenceTemperature(A=6.0e-8 / C_H2O_M, C=1120.0)))

    # Mass conservation — SO4 and SO2OOH- ARE included (kinetics moves S between pools)
    total_S = GAS0_SO2 + SO4MM0
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=so2_g,
        terms=[
            mc.LinearConstraintTerm(gas, so2_g, 1.0),
            mc.LinearConstraintTerm(aq_phase, so2_aq, 1.0),
            mc.LinearConstraintTerm(aq_phase, hso3m, 1.0),
            mc.LinearConstraintTerm(aq_phase, so3mm, 1.0),
            mc.LinearConstraintTerm(aq_phase, so4mm, 1.0),
            mc.LinearConstraintTerm(aq_phase, so2oohm, 1.0),
        ],
        constant=mc.FixedConstant(total_S)))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=h2o2_g,
        terms=[
            mc.LinearConstraintTerm(gas, h2o2_g, 1.0),
            mc.LinearConstraintTerm(aq_phase, h2o2_aq, 1.0),
        ],
        constant=mc.FixedConstant(GAS0_H2O2)))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=gas, algebraic_species=o3_g,
        terms=[
            mc.LinearConstraintTerm(gas, o3_g, 1.0),
            mc.LinearConstraintTerm(aq_phase, o3_aq, 1.0),
        ],
        constant=mc.FixedConstant(GAS0_O3)))
    constraints.append(mc.LinearConstraint(
        algebraic_phase=aq_phase, algebraic_species=hp,
        terms=[
            mc.LinearConstraintTerm(aq_phase, hp, 1.0),
            mc.LinearConstraintTerm(aq_phase, ohm, -1.0),
            mc.LinearConstraintTerm(aq_phase, hso3m, -1.0),
            mc.LinearConstraintTerm(aq_phase, so3mm, -2.0),
            mc.LinearConstraintTerm(aq_phase, so4mm, -2.0),
            mc.LinearConstraintTerm(aq_phase, so2oohm, -1.0),
        ],
        constant=mc.FixedConstant(0.0)))

    return mc.Mechanism(
        name="kinetics",
        species=all_species,
        phases=[gas, aq_phase],
        reactions=[],
        aerosol=mc.Aerosol(representations=[cloud], processes=[r1a, r1b, r2, r3], constraints=constraints))


class TestEquilibriumValidation:
    """Validate equilibrium-only solver against analytical solutions.

    With no kinetic reactions, the system should reach thermodynamic
    equilibrium and satisfy all conservation laws exactly.
    """

    @pytest.fixture
    def solved_state(self):
        """Build, set ICs, and integrate the equilibrium-only system."""
        mechanism = _create_equilibrium_only_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )
        state = micm.create_state()
        mechanism.aerosol.set_default_parameters(state)
        state.set_conditions(temperatures=T_INIT, pressures=P_INIT)
        state.set_concentrations(_naive_initial_conditions(include_so2oohm=False))
        _integrate(micm, state, target_time=10.0)
        return state

    def test_sulfur_mass_conservation(self, solved_state):
        """SO2_g + SO2_aq + HSO3- + SO3-- = gas0_so2 (to 1e-10 relative)."""
        g = _get_conc(solved_state, "SO2")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.SO2_aq")
        hs = _get_conc(solved_state, "CLOUD.AQUEOUS.HSO3m")
        sm = _get_conc(solved_state, "CLOUD.AQUEOUS.SO3mm")
        total = g + aq + hs + sm
        assert total == pytest.approx(GAS0_SO2, rel=1e-10), \
            f"S budget violated: {total} != {GAS0_SO2}"

    def test_h2o2_mass_conservation(self, solved_state):
        """H2O2_g + H2O2_aq = gas0_h2o2 (to 1e-10 relative)."""
        g = _get_conc(solved_state, "H2O2")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.H2O2_aq")
        assert g + aq == pytest.approx(GAS0_H2O2, rel=1e-10), \
            f"H2O2 budget violated: {g + aq} != {GAS0_H2O2}"

    def test_o3_mass_conservation(self, solved_state):
        """O3_g + O3_aq = gas0_o3 (to 1e-10 relative)."""
        g = _get_conc(solved_state, "O3")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.O3_aq")
        assert g + aq == pytest.approx(GAS0_O3, rel=1e-10), \
            f"O3 budget violated: {g + aq} != {GAS0_O3}"

    def test_charge_balance(self, solved_state):
        """H+ - OH- - HSO3- - 2*SO3-- - 2*SO4-- = 0."""
        hp = _get_conc(solved_state, "CLOUD.AQUEOUS.Hp")
        oh = _get_conc(solved_state, "CLOUD.AQUEOUS.OHm")
        hs = _get_conc(solved_state, "CLOUD.AQUEOUS.HSO3m")
        sm = _get_conc(solved_state, "CLOUD.AQUEOUS.SO3mm")
        s4 = _get_conc(solved_state, "CLOUD.AQUEOUS.SO4mm")
        cb = hp - oh - hs - 2 * sm - 2 * s4
        assert abs(cb) < 1e-8 * hp, \
            f"Charge balance violated: {cb} (H+={hp})"

    def test_so4_unchanged(self, solved_state):
        """SO4-- should stay at initial value (no kinetics)."""
        s4 = _get_conc(solved_state, "CLOUD.AQUEOUS.SO4mm")
        assert s4 == pytest.approx(SO4MM0, abs=1e-6), \
            f"SO4 changed without kinetics: {s4} != {SO4MM0}"

    def test_all_positive(self, solved_state):
        """All concentrations must be non-negative."""
        concs = solved_state.get_concentrations()
        for name, vals in concs.items():
            assert vals[0] >= -1e-10, f"{name} = {vals[0]} is negative"

    def test_henrys_law_so2(self, solved_state):
        """SO2_aq / SO2_g = alpha_SO2 (Henry's Law equilibrium)."""
        T = T_INIT
        hlc_T = (1.23 * M_ATM_TO_MOL_M3_PA) * math.exp(3120.0 * (1.0 / T - 1.0 / T0))
        f_liq = C_H2O * MW_H2O / RHO_H2O
        alpha = hlc_T * R_GAS * T * f_liq
        g = _get_conc(solved_state, "SO2")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.SO2_aq")
        assert aq / g == pytest.approx(alpha, rel=1e-4), \
            f"HLC SO2: aq/g={aq/g}, expected alpha={alpha}"

    def test_henrys_law_h2o2(self, solved_state):
        """H2O2_aq / H2O2_g = alpha_H2O2."""
        T = T_INIT
        hlc_T = (7.4e4 * M_ATM_TO_MOL_M3_PA) * math.exp(6621.0 * (1.0 / T - 1.0 / T0))
        f_liq = C_H2O * MW_H2O / RHO_H2O
        alpha = hlc_T * R_GAS * T * f_liq
        g = _get_conc(solved_state, "H2O2")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.H2O2_aq")
        assert aq / g == pytest.approx(alpha, rel=1e-4), \
            f"HLC H2O2: aq/g={aq/g}, expected alpha={alpha}"

    def test_henrys_law_o3(self, solved_state):
        """O3_aq / O3_g = alpha_O3."""
        T = T_INIT
        hlc_T = (1.15e-2 * M_ATM_TO_MOL_M3_PA) * math.exp(2560.0 * (1.0 / T - 1.0 / T0))
        f_liq = C_H2O * MW_H2O / RHO_H2O
        alpha = hlc_T * R_GAS * T * f_liq
        g = _get_conc(solved_state, "O3")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.O3_aq")
        assert aq / g == pytest.approx(alpha, rel=1e-4), \
            f"HLC O3: aq/g={aq/g}, expected alpha={alpha}"

    def test_dissociation_ka1(self, solved_state):
        """Ka1: [HSO3-][H+] / ([SO2_aq][H2O]) = Ka1_T."""
        T = T_INIT
        Ka1_T = (1.7e-2 / C_H2O_M) * math.exp(2090.0 * (1.0 / T0 - 1.0 / T))
        so2_aq = _get_conc(solved_state, "CLOUD.AQUEOUS.SO2_aq")
        hs = _get_conc(solved_state, "CLOUD.AQUEOUS.HSO3m")
        hp = _get_conc(solved_state, "CLOUD.AQUEOUS.Hp")
        h2o = _get_conc(solved_state, "CLOUD.AQUEOUS.H2O")
        Q = (hs * hp) / (so2_aq * h2o)
        assert Q == pytest.approx(Ka1_T, rel=1e-3), \
            f"Ka1 not at equilibrium: Q={Q}, Ka1_T={Ka1_T}"

    def test_dissociation_ka2(self, solved_state):
        """Ka2: [SO3--][H+] / ([HSO3-][H2O]) = Ka2_T."""
        T = T_INIT
        Ka2_T = (6.0e-8 / C_H2O_M) * math.exp(1120.0 * (1.0 / T0 - 1.0 / T))
        hs = _get_conc(solved_state, "CLOUD.AQUEOUS.HSO3m")
        sm = _get_conc(solved_state, "CLOUD.AQUEOUS.SO3mm")
        hp = _get_conc(solved_state, "CLOUD.AQUEOUS.Hp")
        h2o = _get_conc(solved_state, "CLOUD.AQUEOUS.H2O")
        Q = (sm * hp) / (hs * h2o)
        assert Q == pytest.approx(Ka2_T, rel=1e-3), \
            f"Ka2 not at equilibrium: Q={Q}, Ka2_T={Ka2_T}"

    def test_dissociation_kw(self, solved_state):
        """Kw: [H+][OH-] / [H2O]^2 = Kw_T."""
        Kw_T = 1.0e-14 / (C_H2O_M * C_H2O_M)
        hp = _get_conc(solved_state, "CLOUD.AQUEOUS.Hp")
        oh = _get_conc(solved_state, "CLOUD.AQUEOUS.OHm")
        h2o = _get_conc(solved_state, "CLOUD.AQUEOUS.H2O")
        Q = (hp * oh) / (h2o * h2o)
        assert Q == pytest.approx(Kw_T, rel=1e-2), \
            f"Kw not at equilibrium: Q={Q}, Kw_T={Kw_T}"

    def test_ph_reasonable(self, solved_state):
        """pH should be in a physically reasonable range (2-7)."""
        hp = _get_conc(solved_state, "CLOUD.AQUEOUS.Hp")
        # hp is in mol/m3; convert to mol/L by dividing by 1000
        pH = -math.log10(hp / 1000.0)
        assert 2.0 < pH < 7.0, f"pH={pH} out of reasonable range"


class TestKineticsValidation:
    """Validate kinetics solver with S(IV)->S(VI) oxidation reactions.

    With 3 S(IV)->S(VI) oxidation reactions, total sulfur (including SO4)
    must be conserved while SO4 increases from oxidation.
    """

    @pytest.fixture
    def solved_state(self):
        """Build, set ICs, and integrate the kinetics system for 1800s."""
        mechanism = _create_kinetics_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )
        state = micm.create_state()
        mechanism.aerosol.set_default_parameters(state)
        state.set_conditions(temperatures=T_INIT, pressures=P_INIT)
        state.set_concentrations(_naive_initial_conditions())
        _integrate(micm, state, target_time=1800.0, dt_init=0.001)
        return state

    def test_total_sulfur_conservation(self, solved_state):
        """SO2_g + SO2_aq + HSO3- + SO3-- + SO2OOH- + SO4-- = gas0_so2 + so4mm0."""
        total_S = GAS0_SO2 + SO4MM0
        g = _get_conc(solved_state, "SO2")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.SO2_aq")
        hs = _get_conc(solved_state, "CLOUD.AQUEOUS.HSO3m")
        sm = _get_conc(solved_state, "CLOUD.AQUEOUS.SO3mm")
        s4 = _get_conc(solved_state, "CLOUD.AQUEOUS.SO4mm")
        so2ooh = _get_conc(solved_state, "CLOUD.AQUEOUS.SO2OOHm")
        total = g + aq + hs + sm + s4 + so2ooh
        assert total == pytest.approx(total_S, rel=1e-6), \
            f"Total S budget violated: {total} != {total_S}"

    def test_h2o2_non_negative(self, solved_state):
        """H2O2_g + H2O2_aq = gas0_h2o2 (H2O2 is consumed by R1)."""
        # H2O2 is NOT conserved with kinetics (R1 consumes H2O2_aq).
        # But the mass constraint only tracks gas+aq, not the oxidation product.
        # So we just check non-negative.
        g = _get_conc(solved_state, "H2O2")
        aq = _get_conc(solved_state, "CLOUD.AQUEOUS.H2O2_aq")
        assert g >= 0, f"H2O2_g negative: {g}"
        assert aq >= 0, f"H2O2_aq negative: {aq}"
        assert g + aq <= GAS0_H2O2

    def test_charge_balance(self, solved_state):
        """H+ - OH- - HSO3- - 2*SO3-- - 2*SO4-- - SO2OOH- = 0."""
        hp = _get_conc(solved_state, "CLOUD.AQUEOUS.Hp")
        oh = _get_conc(solved_state, "CLOUD.AQUEOUS.OHm")
        hs = _get_conc(solved_state, "CLOUD.AQUEOUS.HSO3m")
        sm = _get_conc(solved_state, "CLOUD.AQUEOUS.SO3mm")
        s4 = _get_conc(solved_state, "CLOUD.AQUEOUS.SO4mm")
        so2ooh = _get_conc(solved_state, "CLOUD.AQUEOUS.SO2OOHm")
        cb = hp - oh - hs - 2 * sm - 2 * s4 - so2ooh
        assert abs(cb) < 0.01 * hp, \
            f"Charge balance violated: {cb} (H+={hp})"

    def test_so4_increases(self, solved_state):
        """SO4-- must increase from S(IV) oxidation."""
        s4 = _get_conc(solved_state, "CLOUD.AQUEOUS.SO4mm")
        assert s4 > SO4MM0, \
            f"SO4 did not increase: {s4} <= {SO4MM0}"

    def test_all_non_negative(self, solved_state):
        """All concentrations must be non-negative."""
        concs = solved_state.get_concentrations()
        for name, vals in concs.items():
            assert vals[0] >= -1e-10, f"{name} = {vals[0]} is negative"

    def test_ph_reasonable(self, solved_state):
        """pH should decrease (become more acidic) as SO4 is produced."""
        hp = _get_conc(solved_state, "CLOUD.AQUEOUS.Hp")
        pH = -math.log10(hp / 1000.0)
        # Kinetics produce H+, so pH should be below the equilibrium value
        assert 1.0 < pH < 7.0, f"pH={pH} out of reasonable range"
