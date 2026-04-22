# Rosenbrock DAE Error Estimator: Algebraic Variable Overshoot

## Summary

The Rosenbrock DAE4 error estimator produces near-zero error values for algebraic
variables, causing the step-acceptance criterion to ignore large errors in
constraint-derived quantities. This allows algebraic "balance" variables (e.g.,
SO₂(g) in a total-sulfur conservation constraint) to overshoot through zero
without triggering step rejection.

## Reproducer

See `python/test/integration/test_dae_algebraic_overshoot.py` for a minimal
chemistry system that demonstrates the issue.

**System**: H₂O₂-pathway sulfate production in cloud droplets.
- 10 species: 3 differential (SO₄²⁻, SO₂OOH⁻, H₂O), 7 algebraic (SO₂, H₂O₂,
  SO₂(aq), H₂O₂(aq), HSO₃⁻, OH⁻, H⁺)
- SO₂(g) is algebraic via `LinearConstraint`: SO₂ = C − SO₂(aq) − HSO₃⁻ − SO₂OOH⁻ − SO₄²⁻
- Fast reactions convert S(IV) → S(VI) until SO₂(g) should approach zero

**Observed**: With a single 30 s solve and default tolerances (atol=1e-3 for gas,
1e-14 for aqueous, rtol=1e-6):
- SO₂(g) = −6.5e-8 (should be ≥ 0)
- 47 internal steps, 0 rejections
- Total sulfur is conserved (constraint works correctly)
- The solver never rejects a step despite SO₂(g) changing sign

**Key evidence**: Changing `atol` for SO₂ from 1e-3 to 1e-12 produces **identical
results** (same 47 steps, same SO₂ value). This proves the error vector entry for
SO₂ is effectively zero — the tolerance is irrelevant.

## Root Cause Analysis

### The error vector is ~0 for algebraic variables

The DAE4 error estimate is `Yerror = K[3]` (only `e[3] = 1.0`, all others zero).

For algebraic variable `i` (mass matrix diagonal `M_ii = 0`):

1. **Stage coupling is zeroed**: In the RHS accumulation:
   ```
   K[stage][i] += (c_jk/H) * M_ii * K[j][i]
   ```
   Since `M_ii = 0`, the coupling terms from previous stages are zero for algebraic rows.

2. **K[3] is determined by the linear solve**: After coupling, the system
   `(M/(γH) − J) · K[3] = RHS` is solved. For algebraic row `i`:
   ```
   −J_i · K[3] = f_i(Y + 2·K[0] + K[2])
   ```
   where `f_i` is the constraint forcing term.

3. **The constraint forcing at the evaluation point is small**: The constraint
   initialization step ensures `f_i ≈ 0` at the initial point. The evaluation
   point `Y + 2·K[0] + K[2]` may deviate, but the Jacobian coupling between the
   constraint row and all other variables means the linear solve distributes the
   residual across many variables, resulting in a small `K[3]` for any single
   algebraic variable.

4. **The error norm contribution is negligible**: Since `Yerror[i] ≈ 0`:
   ```
   errors_over_scale = Yerror[i] / (atol[i] + rtol * ymax[i]) ≈ 0
   ```
   No matter what `atol` is set to.

### Why tight differential tolerances help indirectly

With `atol = 1e-16` and `rtol = 1e-12` for **all** species, the solver takes 690k
steps. This works not because the algebraic error is checked, but because:
- SO₄²⁻ (differential) has a tighter tolerance
- The solver must resolve the SO₄²⁻ trajectory with high accuracy
- This forces small enough steps that SO₄²⁻ never overshoots the total-S budget
- SO₂(g) stays positive as a consequence

This is impractical for real applications — 690k steps for 30 s of chemistry.

## What the current "fix" (commit c20507ad) does

The fix removes the `if (upper_left_identity_diagonal_[i] > 0.0)` skip from
`NormalizedError()`, and replaces `num_ode_variables` with `Y.NumRows() * Y.NumColumns()`.

This is correct in principle but has **no practical effect** because `Yerror[i] ≈ 0`
for algebraic variables. The skip was a symptom, not the cause.

## Proposed fix options

### Option A: Compute algebraic error from the constraint residual

After computing `Ynew`, evaluate the constraint residual `g(Ynew)` and use it as
the error for algebraic variables:

```cpp
// After: Ynew.Copy(Y); for (...) Ynew.Axpy(m_[stage], K[stage]);
// Compute constraint residual for error estimation
if (has_constraints) {
    DenseMatrixPolicy constraint_residual;
    constraint_residual.Fill(0);
    constraints_.AddForcingTerms(Ynew, state.custom_rate_parameters_, constraint_residual);
    // Replace Yerror entries for algebraic variables with constraint residual
    for (i_cell...) for (i_var...) {
        if (state.upper_left_identity_diagonal_[i_var] == 0.0) {
            Yerror[i_cell][i_var] = constraint_residual[i_cell][i_var];
        }
    }
}
```

**Pro**: Directly measures what we care about — how well the constraint is satisfied.
**Con**: The residual may not scale correctly compared to the Rosenbrock error estimate.

### Option B: Use the "state constraint" error estimate

For DAEs, the error in algebraic variables `y_a` is related to the error in
differential variables `y_d` through the constraint Jacobian:

```
δy_a ≈ −(∂g/∂y_a)^{-1} · (∂g/∂y_d) · δy_d
```

This could be computed from the Jacobian blocks. For linear constraints like
total-S conservation, this is exact: `δSO₂ = −δSO₄²⁻ − δSO₂OOH⁻`.

**Pro**: Theoretically sound, relates algebraic error to differential error.
**Con**: Requires extracting Jacobian blocks, more complex implementation.

### Option C: Solve the "over-determined" error system

Replace the Rosenbrock error computation for algebraic rows with a secondary
solve using the constraint Jacobian, as described in Hairer & Wanner (1996),
Section VI.6.

### Option D: Rate damping (user-level workaround)

MIAM's `max_halflife` parameter caps reaction rates so that no reactant is
consumed faster than a specified half-life. This prevents the differential
variable (SO₄²⁻) from overshooting, which indirectly keeps SO₂(g) positive.

**Pro**: Already implemented, user can apply immediately.
**Con**: Workaround, not a solver-level fix. Slightly perturbs the kinetics.

## Recommended approach

**Option A** is the most practical and directly addresses the root cause. The
constraint residual at `Ynew` is already available (just call `AddForcingTerms`
again) and provides a meaningful error measure for algebraic variables.

For the constraint residual to work as an error estimate, it should be scaled
appropriately. One approach: use the L2 norm of the residual per algebraic variable,
normalized by `atol + rtol * |y_a|`.
