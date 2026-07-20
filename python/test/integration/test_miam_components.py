# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Integration tests for individual MIAM components.
#
# Each test class covers one component in isolation (DissolvedReaction,
# DissolvedReversibleReaction, HenrysLawPhaseTransfer, HenrysLawEquilibrium,
# DissolvedEquilibrium, LinearConstraint).  Every test is parametrized over
# all three aerosol representations (UniformSection, SingleMomentMode,
# TwoMomentMode) to catch representation-specific bugs.
#
# Species, rate constants, and initial conditions are deliberately made-up
# and chosen to make the analytical solution simple to compute.
#
# Analytical solutions used
# -------------------------
# DissolvedReaction  foo -> bar, rate = k*[S]/([S]+eps)*[foo]
#   [foo](t) = [foo]0 * exp(-k_eff*t),  k_eff = k*[S]/(S+eps) ~ k for S>>eps
#
# DissolvedReversibleReaction  foo <=> bar
#   K = k_fwd/k_rev,  [foo]_eq = C/(1+K),  [bar]_eq = C*K/(1+K)
#   [foo](t) = [foo]_eq + ([foo]0-[foo]_eq)*exp(-(k_fwd+k_rev)*k_eff_scale*t)
#
# HenrysLawPhaseTransfer (equilibrium limit, long t)
#   [A_aq]_eq = H*R*T*fv*[A_gas]_eq  with  [A_gas]+[A_aq] = Total_A
#
# HenrysLawEquilibrium constraint (algebraic)
#   same equilibrium condition, enforced instantaneously
#
# DissolvedEquilibrium constraint  foo <=> bar  (algebraic species = bar)
#   K_eq*[S]*[foo]/([S]+eps) = [S]*[bar]/([S]+eps)  ->  [bar] = K_eq*[foo]
#   combined with kinetic loss A->D: [foo](t)=foo0*exp(-k_eff*t), [bar]=K*[foo]
#
# LinearConstraint  A + B + C = 5  (C algebraic)
#   A(t)=A0*exp(-k1*t), B built up from A,  C = 5 - A - B  (back-computed)

import math

import pytest

import musica
from musica import backend
import musica.mechanism_configuration as mc
from musica.micm import MICM, SolverState, SolverType
from musica.mechanism_configuration import (
    Aerosol,
    DissolvedEquilibrium,
    DissolvedReaction,
    DissolvedReversibleReaction,
    DiagnoseFromState,
    Equilibrium,
    FixedConstant,
    HenrysLawConstant,
    HenrysLawEquilibrium,
    HenrysLawPhaseTransfer,
    LinearConstraint,
    LinearConstraintTerm,
    SingleMomentMode,
    TwoMomentMode,
    UniformSection,
)

pytestmark = pytest.mark.skipif(
    not backend.miam_available(), reason="MIAM backend is not available"
)

# -- Physical constants --------------------------------------------------------
R = 8.314      # J mol^-1 K^-1
T = 300.0      # K  (all tests run at this temperature)
P = 101325.0   # Pa

# Solvent properties (water-like, round numbers for easy hand-calculation)
MW_S = 0.018    # kg mol^-1
RHO_S = 1000.0  # kg m^-3

# Solvent concentration in the condensed phase [mol m^-3 air]
# LWC = 1 g m^-3 -> 1e-3 kg m^-3 / 0.018 kg mol^-1 ~ 0.0556 mol m^-3
C_S = 0.0556   # mol m^-3 air  (kept fixed in all tests)
f_v = C_S * MW_S / RHO_S   # liquid volume fraction ~ 1e-6

# Particle size used for all representations
R_PARTICLE = 1e-5     # 10 um effective radius
GSD = 1.2             # geometric standard deviation (for modal representations)


# -- Representation fixtures ---------------------------------------------------

def _uniform_section(phase):
    """UniformSection with a fixed single radius."""
    return UniformSection(
        name="aero",
        phases=[phase],
        min_radius=R_PARTICLE,
        max_radius=R_PARTICLE,
    )


def _single_moment_mode(phase):
    """SingleMomentMode with prescribed geometric mean radius."""
    return SingleMomentMode(
        name="aero",
        phases=[phase],
        geometric_mean_radius=R_PARTICLE,
        geometric_standard_deviation=GSD,
    )


def _two_moment_mode(phase):
    """TwoMomentMode -- number concentration set in the state."""
    return TwoMomentMode(
        name="aero",
        phases=[phase],
        geometric_standard_deviation=GSD,
    )


REPR_FACTORIES = {
    "uniform_section": _uniform_section,
    "single_moment_mode": _single_moment_mode,
    "two_moment_mode": _two_moment_mode,
}


def _number_concentration_for_two_moment(phase_prefix):
    """
    Number concentration [m^-3] that gives a mean radius ~ R_PARTICLE.

    V_total = C_S * MW_S / RHO_S = f_v
    V_per_particle = (4/3)pi r^3
    N = f_v / V_per_particle
    """
    V_per = (4.0 / 3.0) * math.pi * R_PARTICLE**3
    return f_v / V_per


# -- Helper: build mechanism + solver -----------------------------------------

def _build(
    repr_name,
    aq_phase,
    all_species,
    gas_phase,
    processes,
    constraints,
    solver_type,
):
    """Assemble a Mechanism + MICM solver for the given component test."""
    repr_obj = REPR_FACTORIES[repr_name](aq_phase)
    mechanism = mc.Mechanism(
        name="test",
        species=all_species,
        phases=[gas_phase, aq_phase] if gas_phase is not None else [aq_phase],
        reactions=[],
        aerosol=Aerosol(
            representations=[repr_obj],
            processes=processes,
            constraints=constraints,
        ),
    )
    solver = MICM(
        mechanism=mechanism,
        solver_type=solver_type,
        external_models=[musica.MIAM()],
    )
    state = solver.create_state()
    mechanism.aerosol.set_default_parameters(state)
    state.set_conditions(temperatures=T, pressures=P)
    # For TwoMomentMode, set the number concentration
    if repr_name == "two_moment_mode":
        N = _number_concentration_for_two_moment("aero")
        state.set_concentrations({"aero.NUMBER_CONCENTRATION": N})
    return solver, mechanism, state


# =============================================================================
# 1.  DissolvedReaction:  foo  ->  bar
# =============================================================================

class TestDissolvedReaction:
    """
    System:  foo  ->  bar  (irreversible, first-order in solution)
    Rate:    d[foo]/dt = -k * [S]/([S]+eps) * [foo]   ~ -k*[foo]  for S>>eps
    Analytical:
        [foo](t) = [foo]0 * exp(-k_eff * t)
        [bar](t) = [foo]0 * (1 - exp(-k_eff * t))
    where k_eff = k * C_S / (C_S + 1e-20) ~ k.
    """

    K = 0.02          # s^-1  (temperature-independent at T=300)
    FOO0 = 1.0        # mol m^-3
    T_SIM = 50.0      # s  (~ 1 e-folding)
    DT = 1.0          # s

    @pytest.fixture(params=list(REPR_FACTORIES))
    def setup(self, request):
        repr_name = request.param

        foo = mc.Species(name="foo")
        bar = mc.Species(name="bar")
        solvent = mc.Species(name="solvent", molecular_weight_kg_mol=MW_S, density_kg_m3=RHO_S)
        aq = mc.Phase(name="aq", species=[foo, bar, solvent])

        rxn = DissolvedReaction(
            phase=aq, solvent=solvent,
            reactants=[foo], products=[bar],
            rate_constant=Equilibrium(A=self.K, C=0.0, T0=T),
        )

        solver, mechanism, state = _build(
            repr_name, aq, [foo, bar, solvent],
            gas_phase=None, processes=[rxn], constraints=[],
            solver_type=SolverType.rosenbrock,
        )
        state.set_concentrations({
            "aero.aq.foo": self.FOO0,
            "aero.aq.bar": 1e-20,
            "aero.aq.solvent": C_S,
        })
        return solver, state

    def test_exponential_decay(self, setup):
        solver, state = setup

        k_eff = self.K * C_S / (C_S + 1e-20)

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={t:.1f} s: {result.state}"
            t += self.DT

        concs = state.get_concentrations()
        foo = concs["aero.aq.foo"][0]
        bar = concs["aero.aq.bar"][0]

        foo_analytical = self.FOO0 * math.exp(-k_eff * self.T_SIM)
        bar_analytical = self.FOO0 * (1.0 - math.exp(-k_eff * self.T_SIM))

        assert abs(foo / foo_analytical - 1.0) < 0.01, \
            f"[foo] = {foo:.6e}, expected {foo_analytical:.6e}"
        assert abs(bar / bar_analytical - 1.0) < 0.01, \
            f"[bar] = {bar:.6e}, expected {bar_analytical:.6e}"

    def test_mass_conservation(self, setup):
        solver, state = setup

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged
            t += self.DT

        concs = state.get_concentrations()
        total = concs["aero.aq.foo"][0] + concs["aero.aq.bar"][0]
        assert abs(total / self.FOO0 - 1.0) < 1e-6, \
            f"Mass not conserved: total = {total:.6e}, expected {self.FOO0:.6e}"


# =============================================================================
# 2.  DissolvedReversibleReaction:  foo  <=>  bar
# =============================================================================

class TestDissolvedReversibleReaction:
    """
    System:  foo  <=>  bar  (reversible, first-order in both directions)
    K = k_fwd / k_rev = 3.0  ->  [foo]_eq = C/4,  [bar]_eq = 3C/4
    Analytical (S >> eps):
        [foo](t) = [foo]_eq + ([foo]0 - [foo]_eq) * exp(-(k_fwd+k_rev)*t)
    """

    K_FWD = 0.03      # s^-1
    K_REV = 0.01      # s^-1  ->  K_eq = 3
    FOO0 = 1.0
    BAR0 = 1e-20
    T_SIM = 200.0     # s  (>> equilibration time ~ 1/(k_fwd+k_rev) = 25 s)
    DT = 2.0

    @pytest.fixture(params=list(REPR_FACTORIES))
    def setup(self, request):
        repr_name = request.param

        foo = mc.Species(name="foo")
        bar = mc.Species(name="bar")
        solvent = mc.Species(name="solvent", molecular_weight_kg_mol=MW_S, density_kg_m3=RHO_S)
        aq = mc.Phase(name="aq", species=[foo, bar, solvent])

        rxn = DissolvedReversibleReaction(
            phase=aq, solvent=solvent,
            reactants=[foo], products=[bar],
            forward_rate_constant=Equilibrium(A=self.K_FWD, C=0.0, T0=T),
            reverse_rate_constant=Equilibrium(A=self.K_REV, C=0.0, T0=T),
        )

        solver, mechanism, state = _build(
            repr_name, aq, [foo, bar, solvent],
            gas_phase=None, processes=[rxn], constraints=[],
            solver_type=SolverType.rosenbrock,
        )
        state.set_concentrations({
            "aero.aq.foo": self.FOO0,
            "aero.aq.bar": self.BAR0,
            "aero.aq.solvent": C_S,
        })
        return solver, state

    def test_equilibrium_reached(self, setup):
        solver, state = setup

        K_eq = self.K_FWD / self.K_REV
        C_total = self.FOO0 + self.BAR0
        foo_eq = C_total / (1.0 + K_eq)
        bar_eq = C_total * K_eq / (1.0 + K_eq)

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={t:.1f} s"
            t += self.DT

        concs = state.get_concentrations()
        foo = concs["aero.aq.foo"][0]
        bar = concs["aero.aq.bar"][0]

        assert abs(foo / foo_eq - 1.0) < 0.01, \
            f"[foo] = {foo:.6e}, expected {foo_eq:.6e}"
        assert abs(bar / bar_eq - 1.0) < 0.01, \
            f"[bar] = {bar:.6e}, expected {bar_eq:.6e}"

    def test_transient(self, setup):
        """Verify the transient response at an intermediate time."""
        solver, state = setup

        K_eq = self.K_FWD / self.K_REV
        C_total = self.FOO0 + self.BAR0
        foo_eq = C_total / (1.0 + K_eq)
        # S/(S+eps) ~ 1
        scale = C_S / (C_S + 1e-20)
        t_check = 1.0 / (self.K_FWD + self.K_REV) / scale  # ~ one e-folding time

        steps = max(1, int(t_check / self.DT))
        t = 0.0
        for _ in range(steps):
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged
            t += self.DT

        concs = state.get_concentrations()
        foo = concs["aero.aq.foo"][0]

        foo_analytical = foo_eq + (self.FOO0 - foo_eq) * math.exp(
            -(self.K_FWD + self.K_REV) * scale * t
        )
        assert abs(foo / foo_analytical - 1.0) < 0.02, \
            f"[foo] = {foo:.6e}, expected {foo_analytical:.6e} at t={t:.2f} s"


# =============================================================================
# 3.  HenrysLawPhaseTransfer:  gas_A  <->  aq_A
# =============================================================================

class TestHenrysLawPhaseTransfer:
    """
    System:  gas_A  <->  aq_A  via kinetic Henry's Law mass transfer.

    At equilibrium (long time):
        [A_aq]_eq / f_v = H * R * T * [A_gas]_eq
    with conservation  [A_gas] + [A_aq] = Total_A.

    Partition factor:
        H_eff = H * R * T * f_v
        [A_gas]_eq = Total_A / (1 + H_eff)
        [A_aq]_eq  = Total_A * H_eff / (1 + H_eff)

    Rate test (short time, [A_aq]~0):
        d[A_gas]/dt ~ -phi_p * k_c * [A_gas]
    For a single-phase mode, phi_p = 1.  k_c = 4pi*r*N*D_g*f(Kn).
    We don't recompute k_c analytically here; instead we verify that the
    gas phase decays monotonically and approaches the expected equilibrium.
    """

    # H = 0.01 mol m^-3 Pa^-1  (large, so equilibrium favours the aqueous phase)
    HLC_REF = 0.01      # mol m^-3 Pa^-1
    D_G = 1e-5          # m^2 s^-1  (gas-phase diffusion coefficient)
    ALPHA = 1.0         # accommodation coefficient
    A_GAS0 = 1e-6       # mol m^-3
    A_AQ0 = 1e-20       # mol m^-3 (small floor to avoid NaN in MIAM)
    T_EQ = 3600.0       # s  (long enough to reach equilibrium)
    DT = 10.0           # s

    @pytest.fixture(params=list(REPR_FACTORIES))
    def setup(self, request):
        repr_name = request.param

        a_gas = mc.Species(name="a_gas", molecular_weight_kg_mol=0.03, density_kg_m3=1000.0)
        a_aq = mc.Species(name="a_aq", molecular_weight_kg_mol=0.03, density_kg_m3=1000.0)
        solvent = mc.Species(name="solvent", molecular_weight_kg_mol=MW_S, density_kg_m3=RHO_S)
        gas = mc.Phase(name="gas", species=[
            mc.PhaseSpecies(a_gas, diffusion_coefficient_m2_s=self.D_G),
        ])
        aq = mc.Phase(name="aq", species=[a_aq, solvent])

        transfer = HenrysLawPhaseTransfer(
            gas_phase=gas,
            gas_species=a_gas,
            condensed_phase=aq,
            condensed_species=a_aq,
            solvent=solvent,
            henrys_law_constant=HenrysLawConstant(HLC_ref=self.HLC_REF, C=0.0),
            accommodation_coefficient=self.ALPHA,
            # diffusion_coefficient picked up from gas PhaseSpecies
        )

        solver, mechanism, state = _build(
            repr_name, aq, [a_gas, a_aq, solvent],
            gas_phase=gas, processes=[transfer], constraints=[],
            solver_type=SolverType.rosenbrock,
        )
        state.set_concentrations({
            "a_gas": self.A_GAS0,
            "aero.aq.a_aq": self.A_AQ0,
            "aero.aq.solvent": C_S,
        })
        return solver, state

    def test_equilibrium_partition(self, setup):
        """After sufficient time the Henry's Law equilibrium must be satisfied."""
        solver, state = setup

        H_eff = self.HLC_REF * R * T * f_v
        Total_A = self.A_GAS0 + self.A_AQ0
        a_gas_eq = Total_A / (1.0 + H_eff)
        a_aq_eq = Total_A * H_eff / (1.0 + H_eff)

        t = 0.0
        while t < self.T_EQ - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={t:.1f} s"
            t += self.DT

        concs = state.get_concentrations()
        a_gas = concs["a_gas"][0]
        a_aq = concs["aero.aq.a_aq"][0]

        assert abs(a_gas / a_gas_eq - 1.0) < 0.05, \
            f"[a_gas] = {a_gas:.4e}, expected {a_gas_eq:.4e}"
        assert abs(a_aq / a_aq_eq - 1.0) < 0.05, \
            f"[a_aq] = {a_aq:.4e}, expected {a_aq_eq:.4e}"

    def test_mass_conservation(self, setup):
        """Total A (gas + aq) must be conserved throughout."""
        solver, state = setup
        Total_A = self.A_GAS0 + self.A_AQ0

        t = 0.0
        while t < self.T_EQ - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged
            t += self.DT

        concs = state.get_concentrations()
        total = concs["a_gas"][0] + concs["aero.aq.a_aq"][0]
        assert abs(total / Total_A - 1.0) < 1e-4, \
            f"Total A not conserved: {total:.6e} vs {Total_A:.6e}"

    def test_direction_of_transfer(self, setup):
        """Gas should decrease and aqueous increase (transfer is condensation)."""
        solver, state = setup

        for _ in range(10):
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged

        concs = state.get_concentrations()
        assert concs["a_gas"][0] < self.A_GAS0, "Gas should decrease due to condensation"
        assert concs["aero.aq.a_aq"][0] > self.A_AQ0, "Aqueous should increase"


# =============================================================================
# 4.  HenrysLawEquilibrium constraint:  gas_A  <=>  aq_A  (algebraic)
# =============================================================================

class TestHenrysLawEquilibriumConstraint:
    """
    System:  gas_A decays kinetically (gas reaction A->C at rate k_gas),
             while aq_A is kept in instantaneous Henry's Law equilibrium
             with gas_A via the DAE constraint.

    At all times:
        [A_aq] = H * R * T * f_v * [A_gas]   (== H_eff * [A_gas])

    Gas_A evolves as ODE.  With the LinearConstraint for total A conservation,
    [A_gas] tracks a modified decay (total A = gas + aq leaves slowly via gas
    reaction, since only the gas phase is kinetically active here).

    For simplicity we test the algebraic relationship [A_aq] = H_eff*[A_gas]
    after each step, regardless of what gas_A is doing.
    """

    HLC_REF = 0.005   # mol m^-3 Pa^-1
    K_GAS = 0.001     # s^-1 (slow gas-phase loss, via Arrhenius gas reaction)
    A_GAS0 = 1e-6     # mol m^-3
    T_SIM = 100.0     # s
    DT = 10.0

    @pytest.fixture(params=list(REPR_FACTORIES))
    def setup(self, request):
        repr_name = request.param

        a_gas = mc.Species(name="a_gas")
        c_gas = mc.Species(name="c_gas")
        a_aq = mc.Species(name="a_aq")
        solvent = mc.Species(name="solvent", molecular_weight_kg_mol=MW_S, density_kg_m3=RHO_S)

        gas = mc.Phase(name="gas", species=[a_gas, c_gas])
        aq = mc.Phase(name="aq", species=[a_aq, solvent])

        # Gas-phase kinetic loss: a_gas -> c_gas  (Arrhenius v1, k=K_GAS at T)
        gas_rxn = mc.Arrhenius(
            name="a_loss",
            A=self.K_GAS,
            C=0.0,
            reactants=[a_gas],
            products=[c_gas],
            gas_phase=gas,
        )

        # HenrysLawEquilibrium: aq_A algebraic, tracks gas_A
        hlc = HenrysLawEquilibrium(
            gas_phase=gas,
            gas_species=a_gas,
            condensed_phase=aq,
            condensed_species=a_aq,
            solvent=solvent,
            henrys_law_constant=HenrysLawConstant(HLC_ref=self.HLC_REF, C=0.0),
            # solvent_molecular_weight and solvent_density are auto-looked-up
            # from the solvent Species object (bug fix tested here)
        )

        # Mass conservation for A: total_A = gas_A + aq_A  (gas_A is algebraic)
        total_A_constraint = LinearConstraint(
            algebraic_phase=gas,
            algebraic_species=a_gas,
            terms=[
                LinearConstraintTerm(gas, a_gas, 1.0),
                LinearConstraintTerm(aq, a_aq, 1.0),
            ],
            constant=DiagnoseFromState(),
        )

        # Build mechanism -- note reactions go into gas-phase reactions list
        repr_obj = REPR_FACTORIES[repr_name](aq)
        mechanism = mc.Mechanism(
            name="test_hlc_eq",
            species=[a_gas, c_gas, a_aq, solvent],
            phases=[gas, aq],
            reactions=[gas_rxn],
            aerosol=Aerosol(
                representations=[repr_obj],
                processes=[],
                constraints=[hlc, total_A_constraint],
            ),
        )
        solver = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_dae4_standard_order,
            external_models=[musica.MIAM()],
        )
        state = solver.create_state()
        mechanism.aerosol.set_default_parameters(state)
        state.set_conditions(temperatures=T, pressures=P)

        H_eff = self.HLC_REF * R * T * f_v
        a_aq0 = H_eff * self.A_GAS0
        state.set_concentrations({
            "a_gas": self.A_GAS0,
            "c_gas": 1e-20,
            "aero.aq.a_aq": a_aq0,
            "aero.aq.solvent": C_S,
        })
        if repr_name == "two_moment_mode":
            N = _number_concentration_for_two_moment("aero")
            state.set_concentrations({"aero.NUMBER_CONCENTRATION": N})

        return solver, state

    def test_equilibrium_maintained(self, setup):
        """[A_aq] must equal H_eff*[A_gas] after every step."""
        solver, state = setup
        H_eff = self.HLC_REF * R * T * f_v

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={t:.1f} s"
            t += self.DT

            concs = state.get_concentrations()
            a_gas = concs["a_gas"][0]
            a_aq = concs["aero.aq.a_aq"][0]
            expected_aq = H_eff * a_gas
            assert abs(a_aq / expected_aq - 1.0) < 0.01, \
                f"Henry's Law violated at t={t:.1f}s: " \
                f"[A_aq]={a_aq:.4e}, H_eff*[A_gas]={expected_aq:.4e}"


# =============================================================================
# 5.  DissolvedEquilibrium constraint:  foo <=> bar  (bar algebraic)
#                                       combined with kinetic  foo -> lost
# =============================================================================

class TestDissolvedEquilibriumConstraint:
    """
    DissolvedEquilibrium enforces:
        G = K_eq * [S]*[foo] / ([S]+eps) - [S]*[bar] / ([S]+eps) = 0
        ->  [bar] = K_eq * [foo]   (for S >> eps, single reactant/product)

    Combined with irreversible kinetic loss  foo -> lost  at rate k_loss,
    so foo decays in time and bar tracks it algebraically.

    Analytical:
        [foo](t) ~ [foo]0 * exp(-k_eff * t)   (k_eff = k_loss * S/(S+eps))
        [bar](t) = K_eq * [foo](t)
        [lost](t) = [foo]0 * (1 - exp(-k_eff * t))
    """

    K_EQ = 3.0         # dimensionless equilibrium constant
    K_LOSS = 0.01      # s^-1 (kinetic loss of foo)
    FOO0 = 1.0         # mol m^-3
    T_SIM = 100.0      # s
    DT = 2.0

    @pytest.fixture(params=list(REPR_FACTORIES))
    def setup(self, request):
        repr_name = request.param

        foo = mc.Species(name="foo")
        bar = mc.Species(name="bar")
        lost = mc.Species(name="lost")
        solvent = mc.Species(name="solvent", molecular_weight_kg_mol=MW_S, density_kg_m3=RHO_S)
        aq = mc.Phase(name="aq", species=[foo, bar, lost, solvent])

        # Kinetic loss: foo -> lost
        loss_rxn = DissolvedReaction(
            phase=aq, solvent=solvent,
            reactants=[foo], products=[lost],
            rate_constant=Equilibrium(A=self.K_LOSS, C=0.0, T0=T),
        )

        # Equilibrium constraint: foo <=> bar,  bar is algebraic
        eq_constraint = DissolvedEquilibrium(
            phase=aq,
            reactants=[foo],
            products=[bar],
            algebraic_species=bar,
            solvent=solvent,
            equilibrium_constant=Equilibrium(A=self.K_EQ, C=0.0, T0=T),
        )

        solver, mechanism, state = _build(
            repr_name, aq, [foo, bar, lost, solvent],
            gas_phase=None, processes=[loss_rxn], constraints=[eq_constraint],
            solver_type=SolverType.rosenbrock_dae4_standard_order,
        )
        bar0 = self.K_EQ * self.FOO0
        state.set_concentrations({
            "aero.aq.foo": self.FOO0,
            "aero.aq.bar": bar0,
            "aero.aq.lost": 1e-20,
            "aero.aq.solvent": C_S,
        })
        return solver, state

    def test_constraint_satisfied(self, setup):
        """[bar] must equal K_eq * [foo] after every step."""
        solver, state = setup
        k_eff = self.K_LOSS * C_S / (C_S + 1e-20)

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={t:.1f} s"
            t += self.DT

            concs = state.get_concentrations()
            foo = concs["aero.aq.foo"][0]
            bar = concs["aero.aq.bar"][0]
            assert abs(bar / (self.K_EQ * foo) - 1.0) < 0.01, \
                f"Equilibrium violated at t={t:.1f}s: " \
                f"[bar]={bar:.4e}, K*[foo]={self.K_EQ*foo:.4e}"

    def test_foo_decay(self, setup):
        """[foo] must follow the analytical exponential decay."""
        solver, state = setup
        k_eff = self.K_LOSS * C_S / (C_S + 1e-20)

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged
            t += self.DT

        concs = state.get_concentrations()
        foo = concs["aero.aq.foo"][0]
        foo_analytical = self.FOO0 * math.exp(-k_eff * self.T_SIM)
        assert abs(foo / foo_analytical - 1.0) < 0.02, \
            f"[foo] = {foo:.6e}, expected {foo_analytical:.6e}"


# =============================================================================
# 6.  LinearConstraint:  A + B + C = 5  (C algebraic)
#                        with kinetic reactions A->B  and  B->C_sink
# =============================================================================

class TestLinearConstraint:
    """
    Dissolved reactions:
        A -> B   at rate k1
        B -> Bsink  at rate k2

    LinearConstraint:
        A + B + C = 5.0   (C is algebraic, determined by conservation)

    Analytical (A, B are ODE; C is algebraic):
        A(t) = A0 * exp(-k1_eff * t)
        B(t) = A0 * k1_eff/(k2_eff-k1_eff) * (exp(-k1_eff*t) - exp(-k2_eff*t))
             + B0 * exp(-k2_eff * t)
        C(t) = 5.0 - A(t) - B(t)

    where k_eff = k * C_S / (C_S + eps) ~ k.
    """

    K1 = 0.02      # A -> B
    K2 = 0.05      # B -> Bsink
    TOTAL = 5.0    # conserved sum A+B+C
    A0 = 3.0
    B0 = 1.0
    # C0 = TOTAL - A0 - B0 = 1.0
    T_SIM = 100.0
    DT = 2.0

    @pytest.fixture(params=list(REPR_FACTORIES))
    def setup(self, request):
        repr_name = request.param

        a = mc.Species(name="a")
        b = mc.Species(name="b")
        c = mc.Species(name="c")       # algebraic (set by constraint)
        bsink = mc.Species(name="bsink")
        solvent = mc.Species(name="solvent", molecular_weight_kg_mol=MW_S, density_kg_m3=RHO_S)
        aq = mc.Phase(name="aq", species=[a, b, c, bsink, solvent])

        rxn_ab = DissolvedReaction(
            phase=aq, solvent=solvent,
            reactants=[a], products=[b],
            rate_constant=Equilibrium(A=self.K1, C=0.0, T0=T),
        )
        rxn_bsink = DissolvedReaction(
            phase=aq, solvent=solvent,
            reactants=[b], products=[bsink],
            rate_constant=Equilibrium(A=self.K2, C=0.0, T0=T),
        )

        constraint = LinearConstraint(
            algebraic_phase=aq,
            algebraic_species=c,
            terms=[
                LinearConstraintTerm(aq, a, 1.0),
                LinearConstraintTerm(aq, b, 1.0),
                LinearConstraintTerm(aq, c, 1.0),
            ],
            constant=FixedConstant(self.TOTAL),
        )

        solver, mechanism, state = _build(
            repr_name, aq, [a, b, c, bsink, solvent],
            gas_phase=None,
            processes=[rxn_ab, rxn_bsink],
            constraints=[constraint],
            solver_type=SolverType.rosenbrock_dae4_standard_order,
        )
        c0 = self.TOTAL - self.A0 - self.B0
        state.set_concentrations({
            "aero.aq.a": self.A0,
            "aero.aq.b": self.B0,
            "aero.aq.c": c0,
            "aero.aq.bsink": 1e-20,
            "aero.aq.solvent": C_S,
        })
        return solver, state

    def test_constraint_satisfied(self, setup):
        """A + B + C must equal TOTAL after every step."""
        solver, state = setup

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged, \
                f"Solver failed at t={t:.1f} s"
            t += self.DT

            concs = state.get_concentrations()
            total = (
                concs["aero.aq.a"][0]
                + concs["aero.aq.b"][0]
                + concs["aero.aq.c"][0]
            )
            assert abs(total / self.TOTAL - 1.0) < 1e-6, \
                f"Conservation violated at t={t:.1f}s: sum={total:.8e} != {self.TOTAL}"

    def test_analytical_solution(self, setup):
        """A, B, C must match the analytical solution at the end."""
        solver, state = setup
        k1 = self.K1 * C_S / (C_S + 1e-20)
        k2 = self.K2 * C_S / (C_S + 1e-20)

        t = 0.0
        while t < self.T_SIM - 1e-10:
            result = solver.solve(state, time_step=self.DT)
            assert result.state == SolverState.Converged
            t += self.DT

        concs = state.get_concentrations()
        a_num = concs["aero.aq.a"][0]
        b_num = concs["aero.aq.b"][0]
        c_num = concs["aero.aq.c"][0]

        # Analytical
        a_an = self.A0 * math.exp(-k1 * self.T_SIM)
        b_an = (
            self.A0 * k1 / (k2 - k1)
            * (math.exp(-k1 * self.T_SIM) - math.exp(-k2 * self.T_SIM))
            + self.B0 * math.exp(-k2 * self.T_SIM)
        )
        c_an = self.TOTAL - a_an - b_an

        assert abs(a_num / a_an - 1.0) < 0.02, \
            f"[a] = {a_num:.6e}, expected {a_an:.6e}"
        assert abs(b_num / b_an - 1.0) < 0.02, \
            f"[b] = {b_num:.6e}, expected {b_an:.6e}"
        assert abs(c_num / c_an - 1.0) < 0.02, \
            f"[c] = {c_num:.6e}, expected {c_an:.6e}"


# =============================================================================
# 7.  Tutorial-14 regression: multi-phase TwoMomentMode system
#
#  This mirrors the mechanism from tutorials/14. aerosol_chemistry_intro.ipynb
#  and uses systematic sub-tests to isolate the NaNDetected root cause.
#
#  Mechanism:
#   - Gas: A, B, C  (A->B->C via Arrhenius)
#   - Foo phase: A, E, F, solvent
#   - Bar phase: B, D, E, F, solvent
#   - foo_r1:  A + F -> E   (DissolvedReaction in foo)
#   - bar_r1:  B + D -> E   (DissolvedReaction in bar)
#   - bar_r2:  E <=> F+solvent (DissolvedReversibleReaction in bar)
#   - a_transfer: A(gas) <-> A(foo) via HenrysLawPhaseTransfer
#   - b_transfer: B(gas) <-> B(bar) via HenrysLawPhaseTransfer
#   - little_mode: TwoMomentMode covering [foo]
#   - big_mode:    TwoMomentMode covering [bar, foo]
# =============================================================================

def _build_tutorial14(
    accommodation_a=0.5,
    accommodation_b=1.0,
    include_dissolved=True,
    include_transfers=True,
):
    """
    Build the tutorial-14 mechanism.  Returns (solver, mechanism, state).

    Parameters
    ----------
    accommodation_a, accommodation_b
        Accommodation coefficients for the Henry's Law transfers.
        Notebook currently sets these to ~1e-20; physically they should be
        0.01-1.0.
    include_dissolved
        Whether to include the three dissolved reactions.
    include_transfers
        Whether to include the two HenrysLawPhaseTransfer processes.
    """
    M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0

    # -- Species --------------------------------------------------------------
    A = mc.Species(name="A", molecular_weight_kg_mol=0.024, density_kg_m3=987.0)
    B = mc.Species(name="B", molecular_weight_kg_mol=0.043, density_kg_m3=1001.3)
    C = mc.Species(name="C")
    D = mc.Species(name="D", molecular_weight_kg_mol=0.023, density_kg_m3=993.0)
    E = mc.Species(name="E", molecular_weight_kg_mol=0.052, density_kg_m3=1002.1)
    F = mc.Species(name="F", molecular_weight_kg_mol=0.019, density_kg_m3=996.4)
    solvent = mc.Species(name="solvent", molecular_weight_kg_mol=0.018, density_kg_m3=1000.0)

    # -- Phases ----------------------------------------------------------------
    gas = mc.Phase(name="gas", species=[
        mc.PhaseSpecies(A, diffusion_coefficient_m2_s=1.3e-9),
        mc.PhaseSpecies(B, diffusion_coefficient_m2_s=2.3e-9),
        C,
    ])
    foo = mc.Phase(name="foo", species=[A, E, F, solvent])
    bar = mc.Phase(name="bar", species=[B, D, E, F, solvent])

    # -- Gas reactions ---------------------------------------------------------
    gas_r1 = mc.Arrhenius(name="A_to_B", A=4.0e-3, C=50,
                          reactants=[A], products=[B], gas_phase=gas)
    gas_r2 = mc.Arrhenius(name="B_to_C", A=4.0e-3, C=50,
                          reactants=[B], products=[C], gas_phase=gas)

    # -- Dissolved reactions ---------------------------------------------------
    processes = []
    if include_dissolved:
        a_foo_r1 = 1.2e-2
        a_bar_r1 = 3.2e-3
        a_bar_r2 = 2.2e-3
        foo_r1 = DissolvedReaction(
            phase=foo, solvent=solvent,
            reactants=[A, F], products=[E],
            rate_constant=mc.Arrhenius(A=a_foo_r1, C=-12.3),
        )
        bar_r1 = DissolvedReaction(
            phase=bar, solvent=solvent,
            reactants=[B, D], products=[E],
            rate_constant=mc.Arrhenius(A=a_bar_r1, C=-5.4),
        )
        bar_r2 = DissolvedReversibleReaction(
            phase=bar, solvent=solvent,
            reactants=[E], products=[F, solvent],
            forward_rate_constant=mc.Arrhenius(A=a_bar_r2, C=-13.4),
            equilibrium_constant=Equilibrium(A=0.2, C=-1.2, T0=295.0),
        )
        processes.extend([foo_r1, bar_r1, bar_r2])

    # -- Henry's Law transfers -------------------------------------------------
    if include_transfers:
        a_transfer = HenrysLawPhaseTransfer(
            gas_phase=gas, gas_species=A,
            condensed_phase=foo, condensed_species=A,
            solvent=solvent,
            henrys_law_constant=HenrysLawConstant(
                HLC_ref=1.23 * M_ATM_TO_MOL_M3_PA, C=3120.0),
            accommodation_coefficient=accommodation_a,
        )
        b_transfer = HenrysLawPhaseTransfer(
            gas_phase=gas, gas_species=B,
            condensed_phase=bar, condensed_species=B,
            solvent=solvent,
            henrys_law_constant=HenrysLawConstant(
                HLC_ref=3.2 * M_ATM_TO_MOL_M3_PA, C=2190.0),
            accommodation_coefficient=accommodation_b,
        )
        processes.extend([a_transfer, b_transfer])

    # -- Representations -------------------------------------------------------
    little_mode = TwoMomentMode(
        name="the little mode", phases=[foo], geometric_standard_deviation=1.2)
    big_mode = TwoMomentMode(
        name="the big mode", phases=[bar, foo], geometric_standard_deviation=1.4)

    # -- Mechanism + solver ----------------------------------------------------
    mechanism = mc.Mechanism(
        name="tutorial14",
        species=[A, B, C, D, E, F, solvent],
        phases=[gas, foo, bar],
        reactions=[gas_r1, gas_r2],
        aerosol=mc.Aerosol(
            representations=[little_mode, big_mode],
            processes=processes,
        ),
    )
    solver = MICM(
        mechanism=mechanism,
        solver_type=SolverType.rosenbrock,
        external_models=[musica.MIAM()],
    )
    state = solver.create_state()
    mechanism.aerosol.set_default_parameters(state)
    state.set_conditions(temperatures=265.0, pressures=101321.3)

    # -- Initial conditions (matching the notebook) ----------------------------
    state.set_concentrations({
        "A": 1.2,
        "B": 2.3,
        "the big mode.NUMBER_CONCENTRATION": 1.0e4,
        "the big mode.bar.B": 1e-20,
        "the big mode.bar.D": 0.02,
        "the big mode.bar.E": 1e-20,
        "the big mode.bar.F": 1e-20,
        "the big mode.bar.solvent": 0.5,
        "the big mode.foo.A": 1e-20,
        "the big mode.foo.F": 0.03,
        "the big mode.foo.solvent": 0.4,
        "the little mode.NUMBER_CONCENTRATION": 1.0e5,
        "the little mode.foo.A": 1e-20,
        "the little mode.foo.F": 0.006,
        "the little mode.foo.solvent": 0.08,
    })

    return solver, mechanism, state


class TestTutorial14:
    """
    Regression / diagnostic tests for the tutorial-14 mechanism.

    Each test enables a different subset of the mechanism to narrow down
    which component triggers NaNDetected.
    """

    def test_dissolved_only_no_transfers(self):
        """Dissolved reactions alone (no Henry's Law transfer) must converge."""
        solver, _, state = _build_tutorial14(
            include_dissolved=True, include_transfers=False)
        result = solver.solve(state, time_step=0.1)
        assert result.state == SolverState.Converged, \
            f"Dissolved-only failed: {result.state}"

    def test_transfers_only_no_dissolved(self):
        """Henry's Law transfers alone (no dissolved reactions) must converge."""
        solver, _, state = _build_tutorial14(
            accommodation_a=0.5, accommodation_b=1.0,
            include_dissolved=False, include_transfers=True)
        result = solver.solve(state, time_step=0.1)
        assert result.state == SolverState.Converged, \
            f"Transfers-only failed: {result.state}"

    def test_full_mechanism_physical_accommodation(self):
        """Full mechanism with physically reasonable accommodation coefficients."""
        solver, _, state = _build_tutorial14(
            accommodation_a=0.5, accommodation_b=1.0,
            include_dissolved=True, include_transfers=True)
        result = solver.solve(state, time_step=0.1)
        assert result.state == SolverState.Converged, \
            f"Full mechanism (physical alpha) failed: {result.state}"

    def test_full_mechanism_near_zero_accommodation(self):
        """
        Full mechanism with accommodation ~ 0 (the current notebook values).

        This test is expected to PASS once the root cause is fixed.
        Near-zero accommodation is physically unreasonable but the solver
        should not NaN -- it should either converge or report MaxStepsExceeded.
        """
        solver, _, state = _build_tutorial14(
            accommodation_a=0.5e-20, accommodation_b=1.0e-20,
            include_dissolved=True, include_transfers=True)
        result = solver.solve(state, time_step=0.1)
        assert result.state != SolverState.NaNDetected, \
            f"NaN with near-zero accommodation (alpha~1e-20). " \
            f"This indicates a numerical instability in the Fuchs-Sutugin Jacobian."

    def test_transfers_near_zero_accommodation(self):
        """
        Henry's Law transfers with accommodation ~ 0, no dissolved reactions.

        Isolates whether the NaN comes from the transfer Jacobian alone.
        """
        solver, _, state = _build_tutorial14(
            accommodation_a=0.5e-20, accommodation_b=1.0e-20,
            include_dissolved=False, include_transfers=True)
        result = solver.solve(state, time_step=0.1)
        assert result.state != SolverState.NaNDetected, \
            f"NaN from transfer Jacobian with near-zero accommodation."

    def test_missing_set_default_parameters_causes_nan(self):
        """
        ROOT CAUSE TEST: GSD=0 when set_default_parameters is not called.

        TwoMomentMode stores GSD as a rate parameter (default 0.0).
        Without set_default_parameters(), GSD=0 -> ln(0)=-inf ->
        r_eff = r_mean * exp(2.5*inf) = inf -> NaN in Henry's Law Jacobian.

        This is the root cause of the NaNDetected in tutorial 14.
        The notebook state-creation cell is missing the call:
            mechanism.aerosol.set_default_parameters(state)
        """
        M_ATM = 1000.0 / 101325.0
        A = mc.Species(name="A", molecular_weight_kg_mol=0.024, density_kg_m3=987.0)
        solvent = mc.Species(name="solvent", molecular_weight_kg_mol=0.018, density_kg_m3=1000.0)
        gas = mc.Phase(name="gas", species=[mc.PhaseSpecies(A, diffusion_coefficient_m2_s=1.3e-9)])
        foo = mc.Phase(name="foo", species=[A, solvent])
        mode = TwoMomentMode(name="mode", phases=[foo], geometric_standard_deviation=1.2)
        mechanism = mc.Mechanism(
            name="test", species=[A, solvent], phases=[gas, foo], reactions=[],
            aerosol=mc.Aerosol(representations=[mode], processes=[
                HenrysLawPhaseTransfer(
                    gas_phase=gas, gas_species=A,
                    condensed_phase=foo, condensed_species=A,
                    solvent=solvent,
                    henrys_law_constant=HenrysLawConstant(HLC_ref=1.23 * M_ATM, C=3120.0),
                    accommodation_coefficient=0.5,
                ),
            ]),
        )
        solver = MICM(mechanism=mechanism, solver_type=SolverType.rosenbrock,
                      external_models=[musica.MIAM()])

        def _state_with_ics(call_set_defaults):
            s = solver.create_state()
            if call_set_defaults:
                mechanism.aerosol.set_default_parameters(s)
            s.set_conditions(temperatures=265.0, pressures=101321.3)
            s.set_concentrations({
                "A": 1.2,
                "mode.foo.A": 1e-20,
                "mode.NUMBER_CONCENTRATION": 1.0e5,
                "mode.foo.solvent": 0.4,
            })
            return s

        # Without set_default_parameters -> GSD=0 -> NaN
        result_bad = solver.solve(_state_with_ics(call_set_defaults=False), time_step=0.1)
        assert result_bad.state == SolverState.NaNDetected, \
            "Expected NaN without set_default_parameters (GSD=0), but solver converged."

        # With set_default_parameters -> GSD=1.2 -> Converged
        result_good = solver.solve(_state_with_ics(call_set_defaults=True), time_step=0.1)
        assert result_good.state == SolverState.Converged, \
            f"Expected Converged with set_default_parameters, got {result_good.state}."
