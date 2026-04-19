# CAM Cloud Chemistry Tutorial (Tutorial 14): Status & Next Steps

## Current state

Tutorial 14 (`tutorials/14. cam_cloud_chemistry.ipynb`) is **functional** and
all tests pass, but it uses a workaround for the algebraic-variable overshoot
bug documented in `dae_algebraic_overshoot.md`.

### What works today

- Full 16-species SO₂/H₂O₂/O₃ cloud chemistry system (6 gas + 10 aqueous)
- Four aqueous oxidation reactions (H₂O₂ pathway R1a/R1b, O₃ pathway R2/R3)
- Ten algebraic constraints (3 Henry's Law, 3 dissociation, 3 conservation, 1 charge balance)
- Dummy `emitted_SO₂` tracker for SO₂ emissions into a mass-budget constraint
- 2-hour simulation with 30 s output, four-panel diagnostic plots
- Validation cells for S(IV) conservation, charge balance, and Henry's Law ratios

### Current workaround: S(IV)-only constraint

The total-sulfur constraint **excludes SO₄²⁻**:

```
SO₂(g) + SO₂(aq) + HSO₃⁻ + SO₃²⁻ + SO₂OOH⁻ − emitted_SO₂ = C
```

This avoids the negative-SO₂ problem because the algebraic variable (SO₂)
doesn't need to absorb the sulfate production. However, it has a known
limitation: **total sulfur is not conserved**. SO₄²⁻ accumulates as a net
addition rather than drawing down the S(IV) pool. The emission rate feeds
the S(IV) budget via `emitted_SO₂`, so SO₂(g) does decrease over time, but
the coupling between sulfate production and SO₂ depletion is indirect (through
the kinetics and pH feedback) rather than exact (through the mass budget).

For most tutorial purposes this produces qualitatively correct behavior —
SO₂ decreases, SO₄²⁻ increases, pH drops — but quantitatively the SO₂
lifetime is longer than it should be.

### Solver parameters

```python
RosenbrockSolverParameters(
    absolute_tolerances=[1e-3 (gas), 1e-14 (aqueous)],
    constraint_init_max_iterations=100,
    constraint_init_tolerance=1e-8,
    max_number_of_steps=5000,
)
```

### Initial conditions

| Species | Value (mol/m³) | Notes |
|---|---|---|
| SO₂(g) | 3.01e-7 | ~10 ppb, polluted |
| H₂O₂(g) | 3.01e-7 | ~10 ppb |
| O₃(g) | 1.5e-6 | ~50 ppb |
| DMS(g) | 3.01e-8 | ~1 ppb, marine |
| OH(g) | 3.01e-13 | ~0.01 ppt |
| SO₄²⁻ | 1e-6 | background sulfate |
| H⁺ | 2e-6 | charge balance |
| LWC | 0.3e-3 kg/m³ | typical stratocumulus |

---

## What needs to change once the MICM fix lands

The fix is to include algebraic variables in `NormalizedError()` — see
`dae_algebraic_overshoot.md` for the full description.

### 1. Switch to total-sulfur constraint

Replace the S(IV)-only constraint with one that includes SO₄²⁻:

```python
LinearConstraint(
    algebraic_phase_name="gas",
    algebraic_species_name="SO2",
    terms=[
        LinearConstraintTerm("gas", "SO2", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO2_aq", 1.0),
        LinearConstraintTerm("AQUEOUS", "HSO3m", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO3mm", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO2OOHm", 1.0),
        LinearConstraintTerm("AQUEOUS", "SO4mm", 1.0),
        # emitted_SO2 acts as the source term
        LinearConstraintTerm("gas", "emitted_SO2", -1.0),
    ],
    diagnose_from_state=True,
)
```

This gives exact sulfur conservation: `SO₂(g) = C_total + emitted_SO₂ − (all aq S)`.

### 2. Verify SO₂(g) stays non-negative

With the MICM fix, the error estimator will reject steps where SO₂(g)
crosses zero, so this should work with practical tolerances (~14 steps,
not 800k). Confirm by running the tolerance sweep in
`test_dae_algebraic_overshoot.py` against the patched MICM.

### 3. Update validation cells

The existing validation cell checks S(IV) conservation. Update it to check
**total-S conservation** instead:

```python
total_S = SO2_g + SO2_aq + HSO3m + SO3mm + SO2OOHm + SO4mm
expected = initial_total_S + cumulative_emitted_SO2
```

### 4. Re-examine initial SO₄²⁻ value

With the total-S constraint, the initial SO₄²⁻ counts against the SO₂
budget. The current `SO4MM0 = 1e-6` is already ~3× the initial SO₂(g),
meaning ~75% of the sulfur budget starts as sulfate. This may be fine
(representing previous cloud processing), but consider whether a lower
value (e.g., 1e-7) better illustrates the oxidation dynamics.

### 5. Tolerance tuning (optional)

Once the fix is in, it may be possible to relax `max_number_of_steps`
back to the default (1000) and remove the `constraint_init_*` overrides
if the solver converges reliably.

### 6. Run the full test suite

```bash
pytest python/test/integration/test_miam_cloud_chemistry.py -v
pytest python/test/integration/test_dae_algebraic_overshoot.py -v
```

The `test_dae_algebraic_overshoot.py` test assertions will need to be
**inverted** once the fix lands — the "default tolerances" test should
then show SO₂ > 0, and the extreme-tolerance test can be removed or
changed to a performance regression test.

---

## File inventory

| File | Purpose |
|---|---|
| `tutorials/14. cam_cloud_chemistry.ipynb` | The tutorial notebook |
| `python/test/integration/test_miam_cloud_chemistry.py` | Existing integration tests |
| `python/test/integration/test_dae_algebraic_overshoot.py` | Bug demonstration test case |
| `.github/prompts/dae_algebraic_overshoot.md` | MICM bug description and proposed fix |
| `.github/prompts/cam_cloud_chemistry_status.md` | This file |
