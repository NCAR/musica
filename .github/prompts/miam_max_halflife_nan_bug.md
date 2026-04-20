# MIAM Bug: `max_halflife` Rate Cap Produces NaN When Any Reactant Is Zero

## Summary

The tanh-smooth rate cap in `DissolvedReaction` (`ForcingFunctionCapped` and
`JacobianFunctionCapped`) has a **0/0 removable singularity** that produces
NaN whenever any reactant concentration is exactly zero or very small
(≲ 1e-31). The NaN propagates through the forcing vector and Jacobian,
crashing the solver immediately.

**Affected file:** `include/miam/processes/dissolved_reaction.hpp`  
**Affected functions:** `ForcingFunctionCapped` (line ~500) and
`JacobianFunctionCapped` (line ~630)

## How to Reproduce

Any `DissolvedReaction` with `max_halflife > 0` whose reactants include a
species that starts at (or passes through) zero will trigger the bug.

```
# Pseudocode — minimal reproduction

reaction = DissolvedReaction(
    reactants = [A, B],
    products  = [C],
    solvent   = H2O,
    rate_constant = k,
    max_halflife  = 1.0,    # enable tanh rate cap
)

# Set initial conditions with one reactant at zero
state[A] = 0.0    # <-- triggers NaN
state[B] = 1e-6
state[H2O] = 16.65

solver.solve(state, dt=30.0)
# → SolverState::NaNDetected at first step
```

In the CAM cloud chemistry tutorial (tutorial 14), this manifests with
reaction R1b (`SO2OOH⁻ + H⁺ → SO₄²⁻`) because `SO2OOHm` starts at 0.0.

## Root Cause Analysis

The tanh rate-cap formula computes:

```
r_max = C_min / t_half
u     = r_raw / r_max
rate  = r_max * tanh(u)
```

where `C_min` is the soft-min of reactant concentrations:

```
C_min = (Σ max(R_i, 1e-300)^{-10})^{-1/10}
```

When any reactant `R_i = 0`:

1. `max(0, 1e-300) = 1e-300`
2. `pow(1e-300, -10)` overflows to `+inf`
3. `C_min = pow(+inf, -0.1) = 0`
4. `r_max = 0 / t_half = 0`
5. `r_raw = 0` (the zero reactant zeroes the product of concentrations)
6. `u = r_raw / r_max = 0 / 0 = NaN`
7. `tanh(NaN) = NaN` → propagates to forcing vector and Jacobian

The same 0/0 occurs in `JacobianFunctionCapped` at Step C:

```cpp
double u = rr / r_max;      // 0/0 = NaN
double th = std::tanh(u);   // NaN
s2 = 1.0 - th * th;         // NaN
cr = (th - u * s2) / t_half; // NaN
```

### The singularity is removable

Mathematically, the limit is well-defined. When the smallest reactant
`R_min → 0`, `C_min → R_min` (the soft-min converges to the true min),
so `r_max = R_min / t_half`. Meanwhile, `r_raw` is proportional to
`R_min` (since it's a factor in the product). Therefore:

```
u = r_raw / r_max
  = (k * S/(S+ε)^n_r * R_min * ∏_{j≠min} R_j) / (R_min / t_half)
  = k * S/(S+ε)^n_r * ∏_{j≠min} R_j * t_half
  → finite as R_min → 0
```

The limit `tanh(u)` is bounded and smooth. The NaN is purely a floating-point
artifact from evaluating `0/0` instead of taking the limit.

## Proposed Fix

Guard against `r_max ≈ 0` in both the forcing and Jacobian functions. When
`r_max` is negligibly small, the rate is already negligible (reactant is
essentially absent), so the tanh cap has no effect — fall back to the raw
(uncapped) values.

### In `ForcingFunctionCapped` (Step 3, tanh cap):

```cpp
// Current code:
double c_min = std::pow(acc, -1.0 / kSoftMinP);
double r_max = c_min / t_half;
rate = r_max * std::tanh(rate / r_max);

// Proposed fix:
double c_min = std::pow(acc, -1.0 / kSoftMinP);
double r_max = c_min / t_half;
if (r_max > 1e-300) {
    rate = r_max * std::tanh(rate / r_max);
}
// else: rate stays as r_raw (already ≈ 0, no capping needed)
```

### In `JacobianFunctionCapped` (Step C, shared quantities):

```cpp
// Current code:
cm = std::pow(cm, -1.0 / kSoftMinP);
double r_max = cm / t_half;
double u = rr / r_max;
double th = std::tanh(u);
s2 = 1.0 - th * th;          // sech²(u)
cr = (th - u * s2) / t_half;  // correction factor

// Proposed fix:
cm = std::pow(cm, -1.0 / kSoftMinP);
double r_max = cm / t_half;
if (r_max > 1e-300) {
    double u = rr / r_max;
    double th = std::tanh(u);
    s2 = 1.0 - th * th;
    cr = (th - u * s2) / t_half;
} else {
    s2 = 1.0;   // sech²(0) = 1 → uncapped Jacobian
    cr = 0.0;   // no correction from rate cap
}
```

**Why `s2 = 1` and `cr = 0`?** In the limit `r_max → 0`:
- `u → finite`, but since `r_max → 0`, the capped partial becomes
  `sech²(u) * dr/dR_j + correction`. Setting `s2 = 1, cr = 0` recovers
  the uncapped Jacobian `dr/dR_j`, which is the correct limiting behavior
  — when the cap threshold is zero, capping is inactive.

## Verification

With the tutorial's cloud chemistry system:

| Test | Reactant IC | max_halflife | Result |
|------|------------|--------------|--------|
| SO2OOHm = 0.0 | R1b + R2 damped | **NaN** (bug) |
| SO2OOHm = 1e-20 | R1b + R2 damped | Converged |
| SO2OOHm = 0.0 | R2 only (non-zero reactants) | Converged |
| SO2OOHm = 1e-20, 7200 s | R1b + R2 damped | All 240 intervals converged |

This confirms the bug is specific to the `r_max = 0` singularity and not
related to the DAE error estimation changes or the solver algorithm itself.

## Notes

- The MICM DAE step-change error fix (`7e46f9b2`) does not **cause** this
  bug, but makes it immediately visible. Previously, the embedded error was
  ~0 for algebraic rows, so NaN may have been masked in certain code paths.
- The `kSoftMinFloor = 1e-300` constant is too small to prevent overflow of
  `pow(x, -10)`. An alternative fix would be to raise this floor to something
  like `1e-30` (so `pow(1e-30, -10) = 1e300`, still representable). However,
  the guard on `r_max` is more robust and doesn't change the soft-min
  behavior for normal concentrations.
