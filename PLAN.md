# Plan: Coupled TUV-x + MICM Column Model

## Goal

Build a 1D photochemical column model that couples TUV-x (photolysis rate calculator) with MICM (chemical kinetics solver) in a time-stepping loop. Species concentrations evolve over a simulated day as solar zenith angle changes, driving photolysis rates that feed into the chemistry. Implement in both Python and Fortran.

## Context

The existing codebase already has:
- A standalone TUV-x v5.4 example (Python + Fortran) that computes photolysis rates on a static column
- A Python Chapman box model example (`python/musica/examples/chapman.py`) that couples TUV-x (TS1 config) with MICM (Chapman mechanism) over 10 grid cells at 20–30 km for 12 hours
- A Python TS1 box model (`python/musica/examples/ts1_box_model.py`) using the full TS1 mechanism
- No Fortran coupled example exists yet

The Chapman mechanism is ideal for this: 5 species (O2, O, O1D, O3, M), 7 reactions (3 photolysis + 4 Arrhenius), and the photolysis reactions map directly to TUV-x v5.4 output.

## Architecture

### Column Structure
- Height grid: 0–120 km, 1 km resolution (120 cells, 121 edges) — same as TUV-x v5.4
- Each grid cell is an independent MICM box (no transport between cells)
- Temperature and pressure from US Standard Atmosphere 1976
- Initial O3 from the v5.4 profile data; O2 and N2 from standard atmosphere

### Time Stepping Loop
```
for each time step:
    1. Compute solar zenith angle (SZA) from time of day and location
    2. Run TUV-x with current SZA → photolysis rate constants at all 121 levels
    3. Map TUV-x j-values to MICM rate parameter names
    4. Set MICM rate parameters for each grid cell
    5. Run MICM chemistry solver → updated species concentrations
    6. (Optional) Update TUV-x O3 profile from MICM output for next step
    7. Record concentrations for output
```

### Photolysis Aliasing (TUV-x v5.4 → Chapman MICM)

| TUV-x reaction name    | MICM rate parameter | Chapman v0 name |
|-------------------------|---------------------|-----------------|
| `O2+hv->O+O`           | `PHOTO.jo2_b`       | `PHOTO.jO2`     |
| `O3+hv->O2+O(3P)`      | `PHOTO.jo3_a`       | `PHOTO.jO3->O`  |
| `O3+hv->O2+O(1D)`      | `PHOTO.jo3_b`       | `PHOTO.jO3->O1D`|

Note: v1 Chapman uses `jo2_b`, `jo3_a`, `jo3_b`; v0 uses `jO2`, `jO3->O`, `jO3->O1D`. The v5.4 TUV-x config has no aliasing section, so we handle the mapping in code.

### SZA Calculation
For simplicity (avoiding `pvlib`/`ussa1976` dependencies in Fortran), compute SZA analytically:
```
SZA = arccos(sin(lat) * sin(dec) + cos(lat) * cos(dec) * cos(hour_angle))
```
where `hour_angle = (solar_hour - 12) * 15°` and `dec` = solar declination for the date.

At night (SZA > ~90°), photolysis rates go to zero.

## Phase 1: Python Column Model

### File: `python/musica/examples/column_model.py`

A standalone Python script that:
1. Sets up TUV-x v5.4 (reuses `musica.tuvx.v54.get_tuvx_calculator()`)
2. Creates MICM Chapman solver (v1 config)
3. Initializes 120 grid cells with US Standard Atmosphere conditions
4. Runs a 24-hour simulation (e.g., Boulder CO, summer solstice)
5. Updates photolysis rates every 15 minutes from TUV-x
6. Records O3, O, O1D concentrations at all levels over time
7. Saves results to NetCDF
8. Produces multi-panel plots:
   - O3 concentration vs height at selected times (dawn, noon, dusk, midnight)
   - O3 time series at selected altitudes (10, 20, 30, 50 km)
   - O1D and O time series at 30 km
   - Photolysis rate (j-value) diurnal cycle at 30 km

Dependencies: `numpy`, `matplotlib`, `xarray` (already in `musica` conda env). Use simple analytical SZA rather than `pvlib` to keep it self-contained.

### Key Design Decisions (Python)
- Use v1 Chapman config (`configs/v1/chapman/config.json`) via `mechanism_configuration.Parser`
- 120 grid cells = full column (MICM vectorized solver handles this efficiently)
- Initial O3 from v5.4 `.dat` file, converted to mol/m³
- O2 and N2 from standard atmosphere mixing ratios (0.20946 and 0.78084)
- Time step: 15 minutes for photolysis update, MICM internal adaptive stepping within each interval

## Phase 2: Fortran Column Model

### File: `fortran/examples/column_model_setup.F90` — Setup module

Module `column_model_setup` providing:
- `setup_tuvx()` → configured `tuvx_t` (reuses `tuvx_v54_setup` module)
- `setup_micm()` → configured `micm_t` + `state_t` with 120 grid cells
- `compute_sza(hour_utc, day_of_year, latitude)` → solar zenith angle in radians
- `map_photolysis_rates(photo_rates, tuvx_ordering, state)` — copy TUV-x j-values into MICM rate_parameters
- `init_concentrations(state, profiles)` — set initial species from v5.4 atmosphere data
- `us_standard_atmosphere(height_km)` → temperature (K) and pressure (Pa)

### File: `fortran/examples/column_model.F90` — Main program

1. Initialize TUV-x (v5.4) and MICM (Chapman, `RosenbrockStandardOrder`)
2. Create MICM state with 120 grid cells
3. Set T, P, air_density for each cell from US Standard Atmosphere
4. Set initial O3, O2, N2 concentrations
5. Time loop (24 hours, 15-min photolysis updates):
   - Compute SZA
   - If daytime: run TUV-x, map j-values to MICM rate parameters
   - If nighttime: set all photolysis rates to zero
   - Call `micm%solve()` for chemistry integration
   - Record O3, O, O1D at all levels
6. Write CSV output (time, height, O3, O, O1D, jO3a, jO3b)

### CMake Integration
- Add to `fortran/examples/CMakeLists.txt`
- Build `column_model_setup_lib` as OBJECT library (depends on `tuvx_v54_setup_lib`)
- Build `column_model` executable
- Register as ctest with working directory `${CMAKE_BINARY_DIR}/configs/tuvx`

## Phase 3: Plotting Utilities

### File: `utils/plot/plot_column_model.py`

Reads the CSV/NetCDF output and produces:
1. **O3 altitude profiles** at selected times (4-panel or single with multiple lines)
2. **O3 diurnal time series** at selected altitudes
3. **O1D and O diurnal profiles** showing the short-lived species response
4. **Photolysis rate diurnal cycle** (j-values vs local time at selected altitudes)
5. **Fortran vs Python comparison** (optional two-file overlay mode)

## Implementation Order

1. **Python column model** — faster iteration, validate the coupling logic
2. **Plotting utilities** — visualize Python results
3. **Fortran column model** — port the validated Python logic
4. **Cross-language comparison** — verify Fortran matches Python

## Unit Conversions

MICM uses mol/m³ internally. TUV-x profiles use molecule/cm³.
- molecule/cm³ → mol/m³: multiply by `1e6 / 6.02214076e23`
- mol/m³ → molecule/cm³: multiply by `6.02214076e23 / 1e6`

Air density from ideal gas: `n = P / (R * T)` where R = 8.31446 J/(mol·K), giving mol/m³.

## Open Questions

- **Feedback loop**: Should updated O3 concentrations feed back into TUV-x for the next photolysis calculation? The existing Chapman example doesn't do this (it only updates j-values from SZA changes, not from composition changes). Full feedback would be more physically realistic but adds complexity.
- **Transport**: This model has no vertical transport (diffusion, advection). Each cell evolves independently. Worth noting in documentation but not implementing in v1.
- **Mechanism choice**: Start with Chapman (5 species). Could later extend to a larger mechanism, but that requires matching more photolysis aliases and initial conditions.
