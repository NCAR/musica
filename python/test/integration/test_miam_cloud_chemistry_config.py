# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Integration test: parses configs/v1/cam_cloud_chemistry/config.json

import pytest

import musica
from musica import backend
from musica.mechanism_configuration import parse
from musica.micm import MICM, SolverType, SolverState
from musica.utils import find_config_path

# Skip all tests if MIAM is not available
pytestmark = pytest.mark.skipif(not backend.miam_available(),
                                 reason="MIAM backend is not available")

CONFIG_PATH = find_config_path("v1", "cam_cloud_chemistry", "config.json")

# ═══ Initial conditions ══════════════════════════════════════════════════════
#
# Unlike test_miam_cloud_chemistry.py's hand-built mechanism (which uses
# distinct aqueous names like "O3_aq"), config.json reuses the SAME species
# name across the gas and AQUEOUS phases (just "O3"), so the aqueous state key
# is "CLOUD.AQUEOUS.O3", not "CLOUD.AQUEOUS.O3_aq".

MW_H2O = 0.018    # kg/mol
LWC = 0.3e-3      # kg/m3 (0.3 g/m3, typical cloud liquid water content)
C_H2O = LWC / MW_H2O  # mol/m3 of air - solvent (water) concentration

T_INIT = 280.0    # K
P_INIT = 70000.0  # Pa

GAS0_SO2 = 3.01e-8   # mol/m3 (~1 ppb)
GAS0_H2O2 = 3.01e-8  # mol/m3
GAS0_O3 = 1.5e-6     # mol/m3
SO4MM0 = 1.0         # mol/m3 (background sulfate)


def _naive_initial_conditions():
    """Naive initial conditions (mol/m3 of air); MIAM's constraint
    initialization projects these onto the constraint manifold before
    time-stepping begins."""
    return {
        "SO2": GAS0_SO2,
        "H2O2": GAS0_H2O2,
        "O3": GAS0_O3,
        "CLOUD.AQUEOUS.H2O": C_H2O,
        "CLOUD.AQUEOUS.SO2": 1e-10,
        "CLOUD.AQUEOUS.H2O2": 1e-10,
        "CLOUD.AQUEOUS.O3": 1e-10,
        "CLOUD.AQUEOUS.Hp": 2.0 * SO4MM0,
        "CLOUD.AQUEOUS.OHm": 1e-10,
        "CLOUD.AQUEOUS.HSO3m": 1e-10,
        "CLOUD.AQUEOUS.SO3mm": 1e-10,
        "CLOUD.AQUEOUS.SO4mm": SO4MM0,
        "CLOUD.AQUEOUS.SO2OOHm": 0.0,
    }


def test_parses_expected_structure():
    mechanism = parse(CONFIG_PATH)

    assert mechanism.name == "CAM Cloud Chemistry"
    assert len(mechanism.species) == 10
    assert len(mechanism.phases) == 2

    aerosol = mechanism.aerosol
    assert aerosol is not None
    assert len(aerosol.representations) == 1
    # R1a (reversible) + R1b, R2, R3 (irreversible S(IV) oxidation by H2O2 and O3)
    assert len(aerosol.processes) == 4
    # 3 Henry's Law equilibria + Kw/Ka1/Ka2 dissolved equilibria + 4 linear
    # constraints (total S, total H2O2, total O3, charge balance)
    assert len(aerosol.constraints) == 10


def test_builds_miam_solver():
    mechanism = parse(CONFIG_PATH)

    micm = MICM(
        mechanism=mechanism,
        solver_type=SolverType.rosenbrock_dae4_standard_order,
        external_models=[musica.MIAM()],
    )
    assert micm is not None


def test_solves_for_a_few_seconds():
    """Parses config.json, sets initial conditions, and integrates a few
    seconds of cloud chemistry, mirroring test_miam_cloud_chemistry.py's
    test_solve_converges.
    """
    mechanism = parse(CONFIG_PATH)

    micm = MICM(
        mechanism=mechanism,
        solver_type=SolverType.rosenbrock_dae4_standard_order,
        external_models=[musica.MIAM()],
    )

    state = micm.create_state()
    mechanism.aerosol.set_default_parameters(state)

    state.set_conditions(temperatures=T_INIT, pressures=P_INIT)
    state.set_concentrations(_naive_initial_conditions())

    total_time = 0.0
    target_time = 2.0
    dt = 0.01
    while total_time < target_time - 1e-10:
        step = min(dt, target_time - total_time)
        result = micm.solve(state, time_step=step)
        assert result.state == SolverState.Converged, \
            f"Solver failed at t={total_time:.4f}s"
        total_time += step
        if total_time > 0.1 and dt < 0.1:
            dt = 0.1

    # Sulfate should only increase from the S(IV) oxidation kinetics
    concs = state.get_concentrations()
    so4_f = concs.get("CLOUD.AQUEOUS.SO4mm", [0.0])[0]
    assert so4_f >= SO4MM0
