# Coupled TUV-x + MICM 1D Column Model (Fortran)

A Fortran example that couples the TUV-x photolysis rate calculator with
the MICM chemical kinetics solver to simulate Chapman stratospheric ozone
photochemistry in a 1D vertical column.

## Overview

The model runs a 24-hour diurnal simulation of the Chapman mechanism
over Boulder, CO on the summer solstice (June 21, day 172). The vertical
domain spans 0--120 km at 1 km resolution (120 grid cells). Each cell
is an independent box --- there is no transport between cells.

At each 15-minute photolysis update interval, TUV-x computes
altitude-dependent photolysis rate constants as a function of solar zenith
angle (SZA). These rates are passed to MICM, which integrates the
Chapman chemistry using a Rosenbrock solver.

## The Chapman Mechanism

The Chapman mechanism (`configs/v1/chapman/config.json`) describes
stratospheric ozone formation and destruction with 6 species and
7 reactions:

**Species:** O, O(1D), O2, O3, N2, M (third body)

**Photolysis reactions** (rates from TUV-x):

| Reaction | MICM parameter | TUV-x label |
|---|---|---|
| O2 + hv -> 2O | `PHOTO.jo2_b` | `O2+hv->O+O` |
| O3 + hv -> O + O2 | `PHOTO.jo3_a` | `O3+hv->O2+O(3P)` |
| O3 + hv -> O(1D) + O2 | `PHOTO.jo3_b` | `O3+hv->O2+O(1D)` |

**Thermal (Arrhenius) reactions:**

| Reaction | Notes |
|---|---|
| O + O2 + M -> O3 + M | Ozone formation (three-body) |
| O + O3 -> 2 O2 | Ozone destruction |
| O(1D) + N2 -> O + N2 | O(1D) quenching |
| O(1D) + O2 -> O + O2 | O(1D) quenching |

## Source Files

| File | Description |
|---|---|
| `column_model.F90` | Main program: initialization, time loop, CSV output |
| `column_model_setup.F90` | Setup module: SZA calculation, profile extraction, photolysis rate mapping |
| `tuvx_v54_setup.F90` | TUV-x v5.4 configuration (shared with the standalone TUV-x example) |

## How It Works

### 1. TUV-x Initialization

The program creates a fully configured TUV-x calculator using the v5.4
photolysis configuration. This loads wavelength grids, atmospheric profiles
(air density, O3, O2, temperature), aerosol radiator data, and
cross-section/quantum-yield data for 109 photolysis reactions.

```fortran
tuvx => get_tuvx_calculator(error)
```

The photolysis reaction ordering is queried to find the indices of the
three Chapman reactions:

```fortran
photo_mappings => tuvx%get_photolysis_rate_constants_ordering(error)
jo2_tuvx_idx  = photo_mappings%index("O2+hv->O+O", error)
jo3a_tuvx_idx = photo_mappings%index("O3+hv->O2+O(3P)", error)
jo3b_tuvx_idx = photo_mappings%index("O3+hv->O2+O(1D)", error)
```

### 2. Initial Atmospheric Profiles

Temperature, pressure, and species concentrations are extracted from the
TUV-x atmospheric profiles. The raw TUV-x data is in molecule/cm3 and is
converted to mol/m3 for MICM:

```fortran
call get_initial_profiles(tuvx, height_mid, temperature, pressure, &
                          o3_init, o2_init, n2_init, error)
```

Inside `get_initial_profiles`, the conversion is:

```fortran
conv = 1.0e6_dk / AVOGADRO  ! molecule/cm3 -> mol/m3
o3_mol_m3(i) = o3_mid(i) * conv
```

Pressure is derived from the ideal gas law:

```fortran
pressure(i) = air_mid(i) * 1.0e6_dk * BOLTZMANN * temp_mid(i)
```

N2 is estimated from the air density assuming a dry atmosphere
(N2 fraction = 0.78084 / (0.78084 + 0.20946)):

```fortran
n2_mol_m3(i) = air_mol_m3(i) * 0.78084_dk / (0.78084_dk + 0.20946_dk)
```

### 3. MICM Initialization

The MICM solver is created from the Chapman v1 mechanism configuration
and a multi-cell state is allocated:

```fortran
micm => micm_t("../v1/chapman/config.json", RosenbrockStandardOrder, error)
state => micm%get_state(NUM_CELLS, error)
```

Species concentrations are stored in a flat 1D array accessed via stride-based
indexing. The strides allow the same code to work regardless of the internal
memory layout:

```fortran
cell_stride = state%species_strides%grid_cell
var_stride  = state%species_strides%variable

state%concentrations(1 + (i-1)*cell_stride + (O3_idx-1)*var_stride) = o3_init(i)
```

### 4. Solar Zenith Angle Calculation

SZA is computed analytically using the Spencer (1971) formula for solar
declination, avoiding any dependency on external astronomy packages:

```fortran
function compute_sza(hour_utc, day_of_year) result(sza)
  lat = LATITUDE * DEG2RAD
  gamma = 2.0_dk * PI * real(day_of_year - 1, dk) / 365.0_dk
  dec = 0.006918_dk &
      - 0.399912_dk * cos(gamma) + 0.070257_dk * sin(gamma) &
      - 0.006758_dk * cos(2.0_dk * gamma) + 0.000907_dk * sin(2.0_dk * gamma) &
      - 0.002697_dk * cos(3.0_dk * gamma) + 0.00148_dk * sin(3.0_dk * gamma)
  solar_hour = hour_utc + LONGITUDE / 15.0_dk
  ha = (solar_hour - 12.0_dk) * 15.0_dk * DEG2RAD
  cos_sza = sin(lat) * sin(dec) + cos(lat) * cos(dec) * cos(ha)
  sza = acos(max(-1.0_dk, min(1.0_dk, cos_sza)))
end function
```

The simulation starts at midnight local time (MDT), which is 06:00 UTC.

### 5. Time Loop

The main loop runs 96 steps of 15 minutes each (24 hours total):

```fortran
do step = 1, n_steps
  hour_utc = START_UTC + current_sec / 3600.0_dk
  sza = compute_sza(hour_utc, DAY_OF_YEAR)
  is_day = (sza < PI / 2.0_dk)

  ! Update photolysis rates from TUV-x (daytime only)
  if (is_day) then
    call tuvx%run(sza, 1.0_dk, photo_rates, heating_rates, error)
  end if
  call set_photolysis_rates(state, photo_rates, ...)

  ! Integrate chemistry with adaptive sub-stepping
  elapsed = 0.0_dk
  do while (elapsed < DT_PHOTO)
    call micm%solve(remaining, state, solver_state, solver_stats, error)
    elapsed = elapsed + solver_stats%final_time()
  end do
end do
```

### 6. Photolysis Rate Mapping

TUV-x returns photolysis rates on grid edges (121 values for 120 cells).
These are averaged to cell midpoints before being written into the MICM
state rate parameters array:

```fortran
jo3a_out(i) = 0.5_dk * (photo_rates(i, jo3a_tuvx_idx) &
                       + photo_rates(i + 1, jo3a_tuvx_idx))

state%rate_parameters(1 + (i-1)*cell_stride + (jo3a_micm_idx-1)*var_stride) &
    = jo3a_out(i)
```

At night (SZA >= 90 degrees), all photolysis rates are set to zero.

## Building

The example is built as part of the MUSICA test suite when the Fortran
interface is enabled:

```bash
cd build
cmake .. -DMUSICA_BUILD_FORTRAN_INTERFACE=ON \
         -DMUSICA_ENABLE_MICM=ON \
         -DMUSICA_ENABLE_TUVX=ON \
         -DMUSICA_ENABLE_TESTS=ON
make -j$(nproc) test_column_model
```

The CMake configuration builds two object libraries and the executable:

```cmake
add_library(tuvx_v54_setup_lib OBJECT tuvx_v54_setup.F90)
add_library(column_model_setup_lib OBJECT column_model_setup.F90)
add_executable(test_column_model column_model.F90)
target_link_libraries(test_column_model PUBLIC
  musica::musica-fortran tuvx_v54_setup_lib column_model_setup_lib)
```

## Running

The executable must be run from `build/configs/tuvx/` so that TUV-x
can resolve its relative data paths, and MICM can find the Chapman
config at `../v1/chapman/config.json`:

```bash
cd build/configs/tuvx
../../test_column_model
```

Or via ctest:

```bash
cd build
ctest -R column_model --output-on-failure
```

### Output

The program writes `column_model_fortran.csv` with columns:

| Column | Units | Description |
|---|---|---|
| `time_hr` | hours | Local time since midnight |
| `height_km` | km | Grid cell midpoint altitude |
| `air_density` | mol/m3 | Air number density |
| `O3` | mol/m3 | Ozone concentration |
| `O` | mol/m3 | Atomic oxygen concentration |
| `O1D` | mol/m3 | Excited oxygen O(1D) concentration |
| `jO3a` | s-1 | j(O3 -> O + O2) photolysis rate |
| `jO3b` | s-1 | j(O3 -> O(1D) + O2) photolysis rate |
| `jO2` | s-1 | j(O2 -> 2O) photolysis rate |

Each time step produces 120 rows (one per grid cell), for a total of
97 time slices (initial state + 96 steps) = 11,640 rows.

## Plotting

A plotting script is provided at `utils/plot/plot_column_model.py` that
reads the CSV output and produces a four-panel figure:

```bash
python utils/plot/plot_column_model.py column_model_fortran.csv \
    -o column_model_results.png
```

The panels show:
1. **O3 vertical profiles** at 00:00, 06:00, 12:00, 18:00, and 24:00 local time (ppm)
2. **O3 diurnal evolution** at 15, 25, 35, and 50 km (ppm)
3. **Short-lived species** O and O(1D) at 30 km (ppb)
4. **Photolysis rates** j(O3) at 30 km (s-1)

The script uses NCAR brand styling (`utils/plot/style.py`) and outputs
both PNG and PDF at 300 DPI.

### Fortran-Python Comparison

The plot script supports overlaying Fortran and Python results. The
Python column model (`python/musica/examples/column_model.py`) produces
NetCDF output that can be compared directly:

```bash
# Run both models
cd build/configs/tuvx && ../../test_column_model
cd /path/to/repo && python python/musica/examples/column_model.py -o column_model

# Compare
python utils/plot/plot_column_model.py column_model_fortran.csv \
    --nc column_model.nc \
    --labels Fortran Python \
    -o column_model_comparison.png
```

In comparison mode, Fortran results are drawn as solid lines and Python
results as dashed lines. The two implementations produce identical results
because they call the same underlying C/C++ library (MUSICA) through
different language bindings.

## Results

### Expected Behavior

The simulation reproduces the well-known features of Chapman
stratospheric ozone chemistry:

- **O3 peak near 25--35 km** at ~4--8 ppm, consistent with the observed
  stratospheric ozone layer
- **Diurnal O3 variation**: O3 decreases during daytime (photolysis
  destroys O3 faster than thermal recombination replenishes it at upper
  altitudes) and recovers at night when photolysis ceases
- **O and O(1D) follow sunlight**: both short-lived species peak at
  local noon and drop to zero at night, as expected for
  photolytically-produced radicals
- **Photolysis rates track SZA**: j-values peak at solar noon
  (minimum SZA ~17 degrees at Boulder on the summer solstice) and
  vanish when SZA exceeds 90 degrees

### Cross-Language Validation

The Fortran and Python implementations produce numerically identical
results. Both call the same compiled MUSICA C++ library through their
respective language bindings (Fortran via `iso_c_binding`, Python via
ctypes). Key validation points:

- **SZA values match exactly** at every time step (same Spencer 1971
  formula, same constants)
- **Photolysis rates are identical** (same TUV-x C++ code, same
  edge-to-midpoint averaging)
- **Species concentrations agree** to machine precision (same MICM
  Rosenbrock solver, same mechanism configuration)

This cross-language agreement serves as a validation that the Fortran
bindings correctly expose the full MUSICA functionality, including
multi-cell state management, stride-based array access, and the
coupled TUV-x/MICM workflow.
