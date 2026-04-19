TS1 + Cloud Chemistry Mechanism Development Plan
Goal
Incrementally add CAM cloud chemistry processes to the TS1 gas-phase mechanism, testing convergence, constraint application, and mass conservation at each step in a Python box model. The final product is a verified ts1_cloud configuration.

Key Insight: Source/Sink Dummy Species for Mass Conservation
When gas-phase SO2 is made algebraic (its ODE replaced by a constraint), the gas-phase reactions that produce or consume SO2 are lost. To track the net gas-phase budget through the constraint, we introduce dummy species:

SO2_source — added as a product (same yield as SO2) to every TS1 reaction that produces SO2. Accumulates total SO2 production.
SO2_sink — added as a product (same yield as SO2 consumed) to every TS1 reaction that consumes SO2. Accumulates total SO2 consumption.
These are inert tracers — they appear only as products, never as reactants, so they don't alter any reaction kinetics. The LINEAR_CONSTRAINT then becomes:

C = SO2(g) − SO2_source + SO2_sink + SO2(aq) + HSO3⁻ + SO3²⁻ + SO4²⁻
with algebraic species = SO2(g) and diagnose from state = true.

At the start of each solve, MICM evaluates C from the current state. The algebraic equation replaces the SO2(g) ODE:

SO2(g) = C + SO2_source − SO2_sink − [aqueous S species]
Same pattern applies to H2O2 and O3.

Which species are algebraic?
In MIAM's DAE system, each cloud chemistry process designates one algebraic species (whose ODE is replaced by the algebraic equation):

Henry's Law Equilibrium → the condensed-phase species is algebraic (e.g., SO2(aq) is determined from SO2(g) via HLC, not the other way around)
Dissolved Equilibrium → the named algebraic species is algebraic (OHm from Kw, HSO3m from Ka1, SO3mm from Ka2)
Linear Constraint → the named algebraic species is algebraic (SO2(g) from sulfur conservation, H2O2(g) from H2O2 conservation, etc.)
Charge Balance → the named algebraic species is algebraic (Hp)
So for SO2 in the combined mechanism:

SO2(aq) — algebraic, set by HLC from SO2(g)
SO2(g) — algebraic, set by LINEAR_CONSTRAINT (source/sink balance)
HSO3⁻ — algebraic, set by Ka1 equilibrium
SO3²⁻ — algebraic, set by Ka2 equilibrium
SO4²⁻ — differential (has an ODE from R1)
TS1 Gas-Phase Reactions Involving SO2, H2O2, O3
SO2 Sources (produce SO2) — need SO2_source co-product
#	Reaction	Type	SO2 yield
R18	SO + BRO → SO2 + BR	ARRHENIUS	1
R41	SO + CLO → SO2 + CL	ARRHENIUS	1
R56	OH + DMS → SO2	ARRHENIUS	1
R70	NO3 + DMS → SO2 + HNO3	ARRHENIUS	1
R86	SO + O2 → SO2 + O	ARRHENIUS	1
R92	SO + NO2 → SO2 + NO	ARRHENIUS	1
R203	SO + OCLO → SO2 + CLO	ARRHENIUS	1
R217	SO + OH → SO2 + H	ARRHENIUS	1
R238	SO + O3 → SO2 + O2	ARRHENIUS	1
R292	OH + OCS → SO2 + CO + H	ARRHENIUS	1
R468	SO3 → SO2 + O	USER_DEFINED	1
R499	OH + DMS → 0.5 SO2 + 0.5 HO2	USER_DEFINED	0.5
SO2 Sinks (consume SO2) — need SO2_sink co-product
#	Reaction	Type	SO2 consumed
R379	SO2 + OH → SO3 + HO2	TROE	1
R476	SO2 → SO + O	USER_DEFINED	1
H2O2 Sources (produce H2O2)
#	Reaction	Type	H2O2 yield
R259	2 HO2 → H2O2 + O2	ARRHENIUS	1
R260	2 HO2 + M → H2O2 + O2 + M	ARRHENIUS	1
R261	2 HO2 + H2O → H2O2 + O2 + H2O	ARRHENIUS	1
R262	2 HO2 + M + H2O → H2O2 + O2 + M + H2O	ARRHENIUS	1
R400	2 OH → H2O2	TROE	1
H2O2 Sinks (consume H2O2)
#	Reaction	Type	H2O2 consumed
R163	O + H2O2 → OH + HO2	ARRHENIUS	1
R290	CL + H2O2 → HCL + HO2	ARRHENIUS	1
R346	OH + H2O2 → H2O + HO2	ARRHENIUS	1
R434	H2O2 → 2 OH	USER_DEFINED	1
O3 Sources (produce O3)
#	Reaction	Type	O3 yield
R73	O2 + M + O → O3 + M	ARRHENIUS	1
R97	MCO3 + HO2 → 0.15 O3 + ...	ARRHENIUS	0.15
R273	CH3CO3 + HO2 → 0.15 O3 + ...	ARRHENIUS	0.15
O3 Sinks (consume O3) — 25 reactions
O3 has many sink reactions (photolysis, OH, HO2, NO, CL, BR, alkenes, etc.). Full list in TS1 reaction indices: R29, R34, R52, R64, R89, R114, R134, R180, R184, R202, R238, R243, R247, R248, R250, R265, R267, R289, R295, R311, R319, R336, R342, R451, R459.

Incremental Development Steps
Each step produces a testable config and a Python box-model test that verifies:

Solver converges (SolverState.Converged)
Mass is conserved where expected
Concentrations are physically reasonable (no negatives, no blowups)
Step 0: Baseline TS1 Gas-Phase Only
Config: Pure TS1 mechanism (configs/v1/ts1/ts1.json), Rosenbrock solver
Test: Set typical tropospheric ICs for SO2, H2O2, O3, etc. Solve for 900s with dt=900s. Verify convergence and species stay positive.
Purpose: Establish that the base mechanism works before adding anything.
Step 1: Add CLOUD Aerosol Representation + H2O Species
Config: TS1 + AQUEOUS phase definition + CLOUD UNIFORM_SECTION aerosol representation. Add AQUEOUS species (SO2, H2O2, O3, Hp, OHm, HSO3m, SO3mm, SO4mm, H2O) to species list. No aerosol processes yet.
Solver: Switch to RosenbrockDAE4StandardOrder
Test: Same ICs as Step 0. Set cloud water (H2O) to typical value. All aqueous species start at floor (1e-30). Verify convergence — nothing should change since there are no aerosol processes.
Purpose: Confirm DAE solver handles the extra inert species.
Step 2: Add Henry's Law for SO2 + Source/Sink Species + Sulfur Constraint
Config changes:
Add species SO2_source and SO2_sink (inert tracers, gas phase)
Add SO2_source as co-product (yield matching SO2) to all 12 SO2-source reactions (R18, R41, R56, R70, R86, R92, R203, R217, R238, R292, R468, R499)
Add SO2_sink as co-product (yield = 1) to both SO2-sink reactions (R379, R476)
Add HENRY_LAW_EQUILIBRIUM aerosol process for SO2
Add LINEAR_CONSTRAINT for sulfur conservation:
{
  "type": "LINEAR_CONSTRAINT",
  "algebraic phase": "gas",
  "algebraic species": "SO2",
  "diagnose from state": true,
  "terms": [
    { "phase": "gas",     "species": "SO2",        "coefficient":  1.0 },
    { "phase": "gas",     "species": "SO2_source",  "coefficient": -1.0 },
    { "phase": "gas",     "species": "SO2_sink",    "coefficient":  1.0 },
    { "phase": "AQUEOUS", "species": "SO2",         "coefficient":  1.0 },
    { "phase": "AQUEOUS", "species": "HSO3m",       "coefficient":  1.0 },
    { "phase": "AQUEOUS", "species": "SO3mm",       "coefficient":  1.0 },
    { "phase": "AQUEOUS", "species": "SO4mm",       "coefficient":  1.0 }
  ]
}
Test:
Set SO2(g) IC, cloud water, run 900s
Verify SO2 partitions into aqueous phase
Verify: SO2(g) + SO2_source_change - SO2_sink_change + aq_sulfur ≈ initial
Verify solver converges
Purpose: First real cloud interaction. Validates the source/sink + constraint pattern works for SO2 with the full TS1 gas-phase chemistry running.
Step 3: Add Dissolved Equilibria (Ka1, Ka2, Kw)
Config changes:
Add DISSOLVED_EQUILIBRIUM for Kw (2 H2O ↔ H⁺ + OH⁻), algebraic species = OHm
Add DISSOLVED_EQUILIBRIUM for Ka1 (SO2(aq) ↔ H⁺ + HSO3⁻), algebraic species = HSO3m
Add DISSOLVED_EQUILIBRIUM for Ka2 (HSO3⁻ ↔ H⁺ + SO3²⁻), algebraic species = SO3mm
Test:
Same ICs. Verify H⁺/OH⁻/HSO3⁻/SO3²⁻ reach equilibrium ratios
Verify sulfur constraint still holds
Verify pH is reasonable (~4-5 for typical SO2 levels)
Verify solver converges
Purpose: Validate that dissolved equilibria + constraint coexist properly.
Step 4: Add Charge Balance Constraint
Config changes:
Add LINEAR_CONSTRAINT for charge balance:
{
  "type": "LINEAR_CONSTRAINT",
  "algebraic phase": "AQUEOUS",
  "algebraic species": "Hp",
  "constant [mol m-3]": 0.0,
  "terms": [
    { "phase": "AQUEOUS", "species": "Hp",    "coefficient":  1.0 },
    { "phase": "AQUEOUS", "species": "OHm",   "coefficient": -1.0 },
    { "phase": "AQUEOUS", "species": "HSO3m", "coefficient": -1.0 },
    { "phase": "AQUEOUS", "species": "SO3mm", "coefficient": -2.0 },
    { "phase": "AQUEOUS", "species": "SO4mm", "coefficient": -2.0 }
  ]
}
Test:
Verify charge balance: [H⁺] = [OH⁻] + [HSO3⁻] + 2[SO3²⁻] + 2[SO4²⁻]
Verify sulfur still conserved
Verify solver converges
Purpose: Add the second algebraic constraint. Test that two constraints interact correctly.
Step 5: Add Henry's Law for H2O2 + Source/Sink + H2O2 Constraint
Config changes:
Add species H2O2_source and H2O2_sink (inert tracers, gas phase)
Add H2O2_source co-product to 5 H2O2-source reactions
Add H2O2_sink co-product to 4 H2O2-sink reactions
Add HENRY_LAW_EQUILIBRIUM for H2O2
Add LINEAR_CONSTRAINT for H2O2 conservation:
{
  "type": "LINEAR_CONSTRAINT",
  "algebraic phase": "gas",
  "algebraic species": "H2O2",
  "diagnose from state": true,
  "terms": [
    { "phase": "gas",     "species": "H2O2",        "coefficient":  1.0 },
    { "phase": "gas",     "species": "H2O2_source",  "coefficient": -1.0 },
    { "phase": "gas",     "species": "H2O2_sink",    "coefficient":  1.0 },
    { "phase": "AQUEOUS", "species": "H2O2",         "coefficient":  1.0 }
  ]
}
Test:
Verify H2O2 partitions correctly
Verify H2O2 conservation with source/sink accounting
Verify previous constraints still hold
Verify solver converges
Purpose: Second HLC species. H2O2 is critical for R1.
Step 6: Add Henry's Law for O3 + Source/Sink + O3 Constraint
Config changes:
Add species O3_source and O3_sink (inert tracers, gas phase)
Add O3_source co-product to 3 O3-source reactions
Add O3_sink co-product to 25 O3-sink reactions
Add HENRY_LAW_EQUILIBRIUM for O3
Add LINEAR_CONSTRAINT for O3 conservation:
{
  "type": "LINEAR_CONSTRAINT",
  "algebraic phase": "gas",
  "algebraic species": "O3",
  "diagnose from state": true,
  "terms": [
    { "phase": "gas",     "species": "O3",        "coefficient":  1.0 },
    { "phase": "gas",     "species": "O3_source",  "coefficient": -1.0 },
    { "phase": "gas",     "species": "O3_sink",    "coefficient":  1.0 },
    { "phase": "AQUEOUS", "species": "O3",         "coefficient":  1.0 }
  ]
}
Test:
O3 has low solubility — verify only trace amounts dissolve
Verify all 3 constraints + charge balance hold simultaneously
Verify solver converges
Purpose: Complete all HLC + constraint setup before adding kinetic reaction.
Step 7: Add Dissolved Reaction R1 (HSO3⁻ + H2O2(aq) → SO4²⁻ + H2O + H⁺)
Config changes:
Add DISSOLVED_REACTION for R1
Test:
This is the critical step. R1 irreversibly converts HSO3⁻ + H2O2(aq) → SO4²⁻
Verify SO4²⁻ grows, HSO3⁻ and H2O2(aq) decrease
Verify sulfur constraint still holds (SO4²⁻ is included!)
Verify H2O2 constraint holds (aqueous H2O2 consumed → gas H2O2 adjusted via constraint + HLC replenishment)
Verify charge balance: H⁺ produced by R1 is captured
Check mass conservation quantitatively: ΔSO4²⁻ ≈ −Δ(SO2_source − SO2_sink + SO2(g) + SO2(aq) + HSO3⁻ + SO3²⁻)
Verify solver converges at dt=900s
Purpose: The whole point of cloud chemistry. If this works with proper mass conservation, the mechanism is correct.
Step 8: Sensitivity Tests
Test variations on Step 7 config:
Vary cloud water content (thick cloud vs thin cloud)
Vary SO2 concentration (polluted vs clean)
Vary temperature (cold vs warm — affects HLC and equilibria)
Long integration (3600s) to check stability
Zero cloud water (should behave like pure TS1)
Multiple sequential solves (simulating MPAS time-stepping)
Purpose: Ensure robustness across atmospheric conditions.
Step 9: Generate Final ts1_cloud Config
Action: Use MIAM Python tools (or manual assembly) to produce MPAS-Model/chemistry_data/ts1_cloud/config.json from the verified Step 7 config.
Test: Run full Python integration test suite.
Purpose: Produce the deployment artifact.
Step 10: MPAS Integration Test
Action: Rebuild MPAS with new config, run JW baroclinic wave test.
Test:
Verify solver converges in cloud cells
Compare wet vs dry: H2O2 should show depletion, SO4²⁻ should grow
Verify no NaN/Inf in output
Purpose: End-to-end validation in the target application.
Implementation Notes
Each step's config is built programmatically by a Python script that reads configs/v1/ts1/ts1.json, modifies reactions/species, and appends aerosol processes. This ensures reproducibility and makes it easy to regenerate after upstream TS1 changes.
The Python box model test for each step lives in python/test/integration/test_ts1_cloud_incremental.py.
Source/sink species need "molecular weight [kg mol-1]": 0.0 (or same as the species they track) since they're purely accounting tracers.
In MPAS, the source/sink species must be reset to zero at the beginning of each chemistry timestep (before solve), since they accumulate within a solve and the constraint's diagnose from state re-evaluates C each call.