# MIAM Integration into MUSICA — Development Plan

## Overview

This plan covers adding the [MIAM](https://github.com/NCAR/miam) (Model-Independent
Aerosol Module) aerosol model to MUSICA as a first-class component alongside MICM and
TUV-x. The target build-to configuration is the CAM Cloud Chemistry test in the MIAM
repository.

### Key Dependencies

| Component | Commit SHA | Notes |
|-----------|-----------|-------|
| MIAM | `aa19c24d612cc656328750c2f9c0d3a662d84bd8` | Header-only C++20 library |
| MICM | `9d9167393d67cd6a3d74a7f46ff774cfb08f7f79` | Adds DAE solvers, external model support, constraints |

### MIAM C++ API Summary

- **Representations**: `SingleMomentMode`, `TwoMomentMode`, `UniformSection`
- **Processes**: `DissolvedReaction`, `DissolvedReversibleReaction`, `HenryLawPhaseTransfer`
- **Constraints**: `DissolvedEquilibriumConstraint`, `HenryLawEquilibriumConstraint`, `LinearConstraint`
- **Model**: Collects representations + processes + constraints; integrates with MICM via `AddExternalModel()`

### New MICM Features Required

- `AddExternalModel()` / `AddExternalModelProcesses()` / `AddExternalModelConstraints()` on the solver builder
- `FourStageDifferentialAlgebraicRosenbrockParameters()` and `SixStageDifferentialAlgebraicRosenbrockParameters()` (DAE solvers)
- Constraint initialization parameters (`constraint_init_max_iterations_`, `constraint_init_tolerance_`)
- Constraint infrastructure: `HasConstraints` concept, `ConstraintSet`, `ExternalModelConstraintSet`

---

## Design Principles

1. **MIAM Python API mirrors the MIAM C++ API** — just like MICM's Python API mirrors its
   C++ API. Builder patterns, typed classes, and a clean object model. Do NOT follow the
   CARMA pattern (dictionary-based configuration).

2. **Bottom-up implementation** — get all MIAM and missing MICM features working up to the
   Python API before beginning Mechanism Configuration work. This lets us validate the
   programmatic API against the CAM Cloud Chemistry test before adding the config layer.

3. **Review checkpoints** — draft API designs and configuration schemas are reviewed before
   implementation in code.

---

## Phase 0: Foundation — Update MICM & Add MIAM Dependency

1. Update MICM commit in `cmake/dependencies.cmake` to `9d9167393d67cd6a3d74a7f46ff774cfb08f7f79`
2. Add MIAM as a new FetchContent dependency (with `MUSICA_ENABLE_MIAM` option)
3. Add `MUSICA_ENABLE_MIAM` build option to the top-level CMakeLists.txt
4. Verify the build compiles with both new dependencies

## Phase 1: Expose Missing MICM Features (DAE Solvers + Constraints)

5. Add DAE solver types to `MICMSolver` enum (e.g., `RosenbrockDAE4`, `RosenbrockDAE6`)
6. Extend `CpuSolver` to support the new DAE solver types
7. Expose DAE solver parameters (`constraint_init_max_iterations_`, `constraint_init_tolerance_`) in the C interface
8. Add `AddExternalModel()` support to the MICM C++ wrapper and C interface
9. Extend the pybind11 bindings for the new solver types and parameters
10. Extend the Python `SolverType` enum and `RosenbrockSolverParameters`
11. **REVIEW CHECKPOINT**: Draft Python API design for DAE solvers — present for review

## Phase 2: MIAM C/C++ Interface Layer

12. **REVIEW CHECKPOINT**: Draft Python API design for MIAM — representations, processes,
    constraints, model — present for review
13. Create `include/musica/miam/` headers: wrapper class, C interface, model/representation/process/constraint types
14. Create `src/miam/` implementation: MIAM C++ wrapper, C interface functions
15. Wire into MICM solver creation: allow a MICM solver to be built with an attached MIAM model
16. Add C/C++ unit tests for the MIAM interface layer

## Phase 3: MIAM Python Bindings

17. Create `python/bindings/miam/` pybind11 bindings for all MIAM types
18. Create `python/musica/miam/` Python wrapper package (mirroring the MIAM C++ API)
19. Build a complete Python integration test matching the CAM Cloud Chemistry configuration
20. Ensure the "naive initial conditions" flow works (solver constraint initialization)

## Phase 4: Mechanism Configuration Schema

21. **REVIEW CHECKPOINT**: Draft Mechanism Configuration schema for MIAM — JSON/YAML
    structure for aerosol models, representations, processes, constraints
22. Extend MechanismConfiguration C++ library to parse MIAM aerosol model definitions
23. Wire parsed MIAM configs into solver construction
24. Extend the Python `Mechanism` class and `Reactions` to support MIAM types
25. Add configuration-driven integration tests in Python

## Phase 5: Integration & Validation

26. End-to-end test: CAM Cloud Chemistry from configuration file through Python
27. Documentation updates
28. CI/Docker updates for MIAM-enabled builds

---

## Key Design Questions for Review Checkpoints

### Phase 1 Review (DAE Python API)

- Naming convention for DAE solver types
- Should DAE constraint init params be part of `RosenbrockSolverParameters` or a separate class?
- How to expose constraint specification from Python

### Phase 2 Review (MIAM Python API)

- Should `miam.Model` be standalone and get attached to `MICM`, or a unified `Solver` wrapper?
- How to handle MIAM's `std::function`-based rate constants in Python
- Representation/constraint API design — typed builder classes mirroring C++

### Phase 4 Review (Mechanism Config Schema)

- How aerosol models nest inside existing mechanism JSON/YAML
- Schema for representations, processes, constraints
- Relationship between mechanism species and aerosol phases

---

## Risk Areas

1. **Rate constant functions**: MIAM processes use `std::function<double(const Conditions&)>`
   for rate constants. Python exposure requires either pre-defined functional forms or a
   callback mechanism. The Mechanism Configuration path will likely use named forms.

2. **Template complexity**: MIAM's `Model` uses variant types and concept-checked templates.
   The C interface will need type erasure, similar to MICM's `IMicmSolver`.

3. **Constraint initialization**: The DAE solver's Newton-based constraint initialization is
   new and may need parameter tuning for different systems.
