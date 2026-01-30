# Building MUSICA with MICM and TUV-x Enabled

Build performed on NCAR Derecho, 2026-01-30.

## 1. Environment / Loaded Modules

```
module list
```

```
Currently Loaded Modules:
  1) craype/2.7.31          5) cray-mpich/8.1.29       9) node-js/18.12.1
  2) intel/2024.2.1         6) hdf5/1.14.3            10) npm/9.3.1
  3) ncarcompilers/1.0.0    7) netcdf/4.9.2
  4) gcc-toolchain/13.2.0   8) ncarenv/23.09
```

### Compilers detected by CMake

| Language | Compiler | Version |
|----------|----------|---------|
| C        | icx      | 2024.2.1 (IntelLLVM) |
| C++      | icpx     | 2024.2.1 (IntelLLVM) |
| Fortran  | ifort    | 2021.1.0.20240703 (Intel Classic) |

### Key tool versions

```
cmake --version   # cmake 3.28.3
pkg-config --version   # 0.29.2
nc-config --version    # netCDF 4.9.2
nf-config --version    # netCDF-Fortran 4.6.1
gcc --version          # GCC 13.2.0 (Spack, via gcc-toolchain)
```

## 2. Fix: Remove `yaml-cpp` from install targets

The file `src/packaging/CMakeLists.txt` references a `yaml-cpp` target in its
`install(TARGETS ...)` call, but `yaml-cpp` is only built internally by the
`mechanism_configuration` dependency (via FetchContent). This causes a CMake
configuration error:

```
CMake Error at src/packaging/CMakeLists.txt:3 (install):
  install TARGETS given target "yaml-cpp" which does not exist.
```

**Fix:** Remove `yaml-cpp` from line 6 of `src/packaging/CMakeLists.txt`:

```diff
 install(
   TARGETS
     musica
-    yaml-cpp
     mechanism_configuration
   EXPORT
     musica_Exports
```

## 3. Create the build directory

```bash
mkdir -p build
cd build
```

## 4. Configure with CMake

```bash
cmake .. \
  -DMUSICA_ENABLE_MICM=ON \
  -DMUSICA_ENABLE_TUVX=ON \
  -DMUSICA_ENABLE_CARMA=OFF \
  -DMUSICA_ENABLE_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Release
```

### Notes on options

- `MUSICA_ENABLE_MICM=ON` and `MUSICA_ENABLE_TUVX=ON` are actually the
  defaults, but are set explicitly here for clarity.
- `MUSICA_ENABLE_CARMA=OFF` is required because CARMA needs BLAS/LAPACK,
  which are not available in this module environment. If you need CARMA, load
  a BLAS module first.
- Dependencies (MICM v3.11.0, TUV-x v0.14.0, MechanismConfiguration v1.1.1,
  GoogleTest) are fetched automatically via CMake FetchContent.

### Configuration summary output

```
MUSICA Version:     0.14.4
Build type:         Release
C Compiler:         icx (2024.2.1)
C++ Compiler:       icpx (2024.2.1)
Fortran Compiler:   ifort
MPI:                OFF
OPENMP:             OFF
CXX Interface:      ON
Fortran Interface:  OFF
MICM:               ON (v3.11.0)
TUV-X:              ON (v0.14.0)
CARMA:              OFF
```

## 5. Build

```bash
make -j4
```

Build completes at 100% producing:

- `lib64/libmusica.a` -- the MUSICA static library
- Test binaries: `test_util`, `test_micm_wrapper`, `test_micm_c_api`,
  `test_parser`, `test_tuvx_c_api`, `test_tuvx_run_from_config`

### Warnings (non-fatal)

- `ifort` deprecation remarks (Intel recommends transitioning to `ifx`).
- `--gcc-toolchain` option not supported by `ifort` (passed by ncarcompilers wrapper).
- Several Fortran `INTENT(OUT)` warnings in TUV-x interface files.

## 6. Run tests

```bash
ctest --output-on-failure
```

All 6 tests passed (total time 1.57s):

| # | Test | Result | Time |
|---|------|--------|------|
| 1 | util | Passed | 0.06s |
| 2 | micm_wrapper | Passed | 0.03s |
| 3 | micm_c_api | Passed | 0.04s |
| 4 | parser | Passed | 0.21s |
| 5 | tuvx_c_api | Passed | 1.17s |
| 6 | tuvx_run_from_config | Passed | 0.04s |
