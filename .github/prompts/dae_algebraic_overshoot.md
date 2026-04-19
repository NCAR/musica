# DAE Rosenbrock Error Estimator: Algebraic Variable Overshoot

## Summary

The Rosenbrock DAE solver's `NormalizedError()` function excludes algebraic
variables (those with `upper_left_identity_diagonal_[i] == 0`) from the
step-acceptance error norm. This allows the solver to accept large internal
steps where differential species overshoot a conservation budget, forcing the
algebraic "balance" variable negative — a physically impossible state that the
continuous system can never reach.

## Affected code

**File:** `include/micm/solver/rosenbrock.inl`
**Function:** `NormalizedError()` (two overloads: non-vectorized ~line 312, vectorized ~line 355)
**MICM commit:** `664233c97ffab587f56e21070e8dc95cabb5dcf0`

```cpp
// Current code — algebraic variables SKIPPED
for (std::size_t i_var = 0; i_var < Y.NumColumns(); ++i_var)
{
  // Skip algebraic variables (mass matrix diagonal = 0)
  if (state.upper_left_identity_diagonal_[i_var] > 0.0)
  {
    ymax = std::max(std::abs(Y[i_cell][i_var]), std::abs(Ynew[i_cell][i_var]));
    errors_over_scale = errors[i_cell][i_var] / (atol[i_var % n_vars] + rtol * ymax);
    error += errors_over_scale * errors_over_scale;
    ++num_ode_variables;
  }
}
```

## Problem description

### The physics

In a cloud chemistry system, gaseous SO₂ dissolves into cloud water, dissociates,
and is oxidized to SO₄²⁻ by H₂O₂. A total-sulfur conservation constraint
ensures mass balance:

```
SO₂(g) + SO₂(aq) + HSO₃⁻ + SO₂OOH⁻ + SO₄²⁻ = C_total
```

- **SO₂(g)** is the algebraic variable (set by the constraint)
- **SO₄²⁻** and **SO₂OOH⁻** are differential (evolved by kinetics)
- All other S-species are algebraic (set by Henry's Law / dissociation constraints)

The continuous system can never produce SO₂(g) < 0 because the kinetic rates
approach zero as S(IV) → 0. However, the discrete Rosenbrock method can
overshoot.

### The mechanism

1. The oxidation reactions produce SO₄²⁻ from HSO₃⁻ (fast kinetics, pH ~2)
2. The solver evaluates the error on differential variables only: SO₄²⁻, SO₂OOH⁻, H₂O
3. With SO₄²⁻ ~ 1e-6 and atol = 1e-14, the error scale is ~1e-12 — very tight
4. But the solver still takes large steps (~2 s each, ~14 steps for 30 s) because
   the *per-step* error on SO₄²⁻ is small — each step accurately advances SO₄²⁻
5. The cumulative result is that SO₄²⁻ exceeds the total-S budget
6. The algebraic SO₂(g) = C − SO₄²⁻ − ... goes to −6.5e-8 (budget is ~1.3e-6)
7. **This error is never detected** because SO₂(g) is excluded from the norm

### Quantitative evidence

| Tolerance setting | SO₂(g) after 30 s | Internal steps | Rejected |
|---|---|---|---|
| atol=1e-14, rtol=1e-6 (practical) | **−6.50e-8** | 14 | 0 |
| atol=1e-16, rtol=1e-12 | **−6.46e-8** | 498 | 0 |
| atol=1e-18, rtol=1e-12 | **−4.56e-8** | 26,931 | 0 |
| atol=1e-19, rtol=1e-12 | **+1.07e-7** | 316,519 | 6 |
| atol=1e-20, rtol=1e-14 | **+2.26e-7** | 811,128 | 29 |

The transition from negative to positive SO₂ occurs at atol ~ 1e-19. At that
point, the absolute tolerance on tiny species (OHm ~ 1e-16, SO₃²⁻ ~ 1e-17)
becomes smaller than the species values themselves, forcing sub-microsecond
internal steps that track the kinetic deceleration as S(IV) → 0.

This is not a practical solution — 800k steps per 30 s is orders of magnitude
too expensive for use in 3D atmospheric models.

## Test case

**File:** `python/test/integration/test_dae_algebraic_overshoot.py`

A minimal cloud chemistry system (SO₂/H₂O₂ pathway only, 10 species) that
demonstrates the problem. Contains:

- `test_negative_so2_with_default_tolerances`: proves SO₂ goes negative with
  practical tolerances (14 steps, converged)
- `test_positive_so2_with_extreme_tolerances`: proves the physics is correct
  when tolerances force enough steps (800k+ steps, SO₂ stays positive)

Run as standalone script for a full tolerance sweep:
```bash
python python/test/integration/test_dae_algebraic_overshoot.py
```

Run as pytest:
```bash
pytest python/test/integration/test_dae_algebraic_overshoot.py -v
```

## Proposed fix

Include algebraic variables in `NormalizedError()`. Both overloads
(non-vectorized and vectorized) need the same change:

```cpp
// Proposed fix — include ALL variables in the error norm
for (std::size_t i_var = 0; i_var < Y.NumColumns(); ++i_var)
{
  ymax = std::max(std::abs(Y[i_cell][i_var]), std::abs(Ynew[i_cell][i_var]));
  errors_over_scale = errors[i_cell][i_var] / (atol[i_var % n_vars] + rtol * ymax);
  error += errors_over_scale * errors_over_scale;
}
// Use total variable count instead of ODE-only count
const std::size_t N = Y.NumRows() * Y.NumColumns();
```

### Why this works

- When SO₂(g) changes sign (from +3e-7 to −6.5e-8), the relative change is
  enormous. The embedded error estimate for SO₂(g) will be large compared to
  `atol + rtol × max(|Y|, |Ynew|)`, causing the step to be rejected.
- The solver will then try a smaller step where SO₄²⁻ doesn't overshoot the
  budget, and SO₂(g) remains positive.
- This restores the invariant that the continuous-system solution respects
  non-negativity, without requiring impractical tolerance settings.

### Considerations

- The `Yerror` for algebraic variables is already computed by the Rosenbrock
  stages and error formula (`e_[stage] * K[stage]`). It represents the
  difference between the main and embedded solutions for the algebraic
  variables. No new computation is needed.
- Users who set very loose tolerances on algebraic variables will not see
  a performance impact (the error term will be small relative to the scale).
- An alternative approach would be to add this as an **option** (e.g., a boolean
  `include_algebraic_in_error` parameter, defaulting to `true`) to preserve
  backward compatibility.

## Impact

This issue affects any DAE system where:
1. A conservation constraint defines an algebraic variable
2. Fast kinetics convert species within the conservation pool
3. The differential product can temporarily exceed the conservation budget

The cloud chemistry use case (SO₂ oxidation to SO₄²⁻) is the primary
motivating example, but the issue is general to any stiff DAE system with
conservation constraints.
