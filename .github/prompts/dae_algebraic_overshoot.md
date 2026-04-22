# DAE Rosenbrock Error Estimator: Algebraic Variable Step-Change Error

## Summary

MICM's Rosenbrock DAE solver now uses **step-change error estimation** for
algebraic variables. After computing the embedded error, the solver replaces
each algebraic entry with `Yerror[a] = Ynew[a] - Y[a]`. This means algebraic
variable tolerances now control step acceptance.

**MICM commit:** `7e46f9b2f9b968b56cebb6f1272005409b30df3d`
**MIAM commit:** `002eadfeac2deb7b5bae84c1addf2d09a00f7544`

## Background

Previously, the embedded error formula (`Yerror = Σ e_i·K_i`) produced
near-zero entries for algebraic rows because the mass matrix diagonal
`M_ii = 0` zeroed out inter-stage coupling terms. The solver was completely
insensitive to algebraic tolerances. The fix replaces the near-zero embedded
error with the actual step change for algebraic variables.

## Behavior

With the step-change error, algebraic tolerances control how much the variable
can change per internal step before the solver rejects and retries smaller:

| Algebraic atol | SO₂(g) after 30 s | Steps | Notes |
|---|---|---|---|
| 1e-3 (loose) | −6.50e-8 | 13 | Overshoot — too few steps |
| 1e-8 | −6.50e-8 | 48 | More steps, still overshoots |
| 1e-10 | −6.25e-8 | 3,470 | Significant step reduction |
| 1e-12 (tight) | +1.73e-8 | 130,766 | No overshoot — SO₂ positive |
| 1e-14 | +1.19e-7 | 391,987 | Very tight — expensive |

**Key insight:** The guide recommends `atol = 1e-8 to 1e-10` for algebraic
variables. At these tolerances, SO₂ still goes slightly negative (within atol)
in this stiff system. For guaranteed non-negativity, `atol ≤ 1e-12` is needed
but costs ~130k steps per 30 s timestep.

## Practical tolerance strategy

For cloud chemistry with conservation constraints:

```python
ordering = micm.create_state().get_species_ordering()
abs_tols = [1e-14] * len(ordering)  # tight default for differential species

# Per the guide: 1e-8 to 1e-10 for algebraic species
algebraic_species = {"SO2", "H2O2", "CLOUD.AQUEOUS.SO2_aq", ...}
for name, idx in ordering.items():
    if name in algebraic_species:
        abs_tols[idx] = 1e-10  # or 1e-8 for speed
```

If the balance variable going slightly negative (within atol) is unacceptable,
combine with rate damping (`max_halflife`) to limit per-step overshoot.

## Test case

**File:** `python/test/integration/test_dae_algebraic_overshoot.py`

Three tests:

- `test_algebraic_tolerance_sensitivity`: Proves that changing atol for
  algebraic species from 1e-3 to 1e-10 produces >10× more steps
- `test_negative_so2_with_loose_algebraic_tolerances`: SO₂ goes negative
  with loose algebraic atol (1e-3), confirming the overshoot mechanism
- `test_positive_so2_with_tight_algebraic_tolerances`: SO₂ stays positive
  with tight algebraic atol (1e-12), confirming the fix prevents overshoot

```bash
pytest python/test/integration/test_dae_algebraic_overshoot.py -v
```

## Reference

See the MICM developer guide at `micm/.github/prompts/dae_algebraic_tolerance_guide.md`
for the full specification and C++ examples.
