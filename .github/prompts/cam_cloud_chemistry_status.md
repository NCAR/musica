# CAM Cloud Chemistry Tutorial (Tutorial 14): Status & Next Steps

## Current state

Tutorial 14 (`tutorials/14. cam_cloud_chemistry.ipynb`) uses a **total-sulfur
constraint** (including SO₄²⁻) with rate damping (`max_halflife`) and
per-species tolerances.

**MICM commit `7e46f9b2`** implements step-change error estimation for
algebraic variables (`Yerror[a] = Ynew[a] - Y[a]`), making algebraic
tolerances control step acceptance. This replaces the previous near-zero
embedded error for algebraic rows.

### What works today

- Full SO₂/H₂O₂ cloud chemistry system (gas + aqueous species)
- Aqueous oxidation reactions (H₂O₂ pathway R1a/R1b)
- Algebraic constraints (Henry's Law, dissociation, conservation, charge balance)
- Total-sulfur conservation constraint (SO₂ + SO₂_aq + HSO₃⁻ + SO₂OOH⁻ + SO₄²⁻)
- Dummy `emitted_SO₂` tracker for SO₂ source term in mass budget
- Rate damping via `max_halflife` parameter
- Solvent damping via `solvent_damping_epsilon` (default 1e-20)
- Per-species absolute tolerances (1e-14 aqueous, 1e-9 gas algebraic)
- Validation cells for total-S conservation, charge balance, Henry's Law
- Step-change error estimate for algebraic variables (MICM fix)

### Tolerance strategy

With the step-change error fix, algebraic tolerances now matter:

| Algebraic atol | SO₂ sign | Steps/30s | Practical? |
|---|---|---|---|
| 1e-3 to 1e-8 | negative | 13–48 | fast but overshoots |
| 1e-10 | negative | ~3,500 | moderate |
| 1e-12 | **positive** | ~131k | expensive |

For the tutorial, the combination of moderate algebraic tolerances (1e-8 to
1e-10) plus rate damping (`max_halflife`) provides a practical balance: the
rate damping limits per-step overshoot and the step-change error controls
step acceptance.

### Solver parameters

```python
RosenbrockSolverParameters(
    absolute_tolerances=[1e-9 (gas algebraic), 1e-14 (aqueous)],
    constraint_init_max_iterations=100,
    constraint_init_tolerance=1e-8,
    max_number_of_steps=5000,
)
```

---

## Remaining work

### 1. Tutorial tolerance tuning

With the step-change error fix, re-evaluate whether the tutorial's per-species
tolerances and `max_halflife` settings give the best balance of accuracy and
performance. Consider testing:
- `alg_atol=1e-10` with `max_halflife=10` — may prevent overshoot at ~3.5k steps
- `alg_atol=1e-8` with `max_halflife=5` — fewer steps, rate damping does the work

### 2. Run the full test suite

```bash
pytest python/test/integration/test_miam_cloud_chemistry.py -v   # 31 tests
pytest python/test/integration/test_dae_algebraic_overshoot.py -v  # 3 tests
```

### 3. Tutorial notebook validation

Run the tutorial end-to-end to verify all cells produce correct output with
the new MICM.

---

## File inventory

| File | Purpose |
|---|---|
| `tutorials/14. cam_cloud_chemistry.ipynb` | The tutorial notebook |
| `python/test/integration/test_miam_cloud_chemistry.py` | Integration tests (31) |
| `python/test/integration/test_dae_algebraic_overshoot.py` | Overshoot tests (3) — sensitivity, loose, tight |
| `python/test/integration/diagnose_dae_error.py` | Diagnostic script (historical) |
| `.github/issues/rosenbrock_dae_algebraic_error.md` | Issue doc (historical — fix landed) |
| `.github/prompts/dae_algebraic_overshoot.md` | Step-change error description |
| `.github/prompts/cam_cloud_chemistry_status.md` | This file |
