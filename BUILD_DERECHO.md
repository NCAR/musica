# Building MUSICA with MICM, TUV-x, and CARMA Enabled

Build performed on NCAR Derecho, 2026-01-30.

## 1. Environment / Loaded Modules

Set up the Derecho module environment:

```bash
module --force purge
module load ncarenv/25.10
module load gcc/14.3.0
module load ncarcompilers/1.2.0
module load cray-mpich/8.1.32
module load netcdf/4.9.3
module load parallel-netcdf/1.14.1
module load openblas/0.3.30
```

Note: GCC 14+ is required because `mechanism_configuration` uses the C++20
`<format>` header. The `ncarcompilers` wrapper may auto-resolve to version
1.1.0; this is expected and works correctly.

## 2. Create the build directory

```bash
mkdir -p build
cd build
```

## 3. Configure with CMake

Set `CMAKE_INSTALL_PREFIX` to the directory where you want MUSICA installed,
e.g. `/glade/work/$USER/packages`:

```bash
cmake .. \
  -DMUSICA_ENABLE_MICM=ON \
  -DMUSICA_ENABLE_TUVX=ON \
  -DMUSICA_ENABLE_CARMA=ON \
  -DMUSICA_ENABLE_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/glade/work/$USER/packages
```

### Notes on options

- `MUSICA_ENABLE_MICM=ON` and `MUSICA_ENABLE_TUVX=ON` are actually the
  defaults, but are set explicitly here for clarity.
- `MUSICA_ENABLE_CARMA=ON` requires BLAS/LAPACK (provided by the
  `openblas` module). To build without CARMA, set this to `OFF` and
  omit the `openblas` module.
- Dependencies (MICM v3.11.0, TUV-x v0.14.0, CARMA develop-carma-box,
  MechanismConfiguration v1.1.1, GoogleTest) are fetched automatically
  via CMake FetchContent.

### Configuration summary output

```
MUSICA Version:     0.14.4
Build type:         Release
C Compiler:         gcc (14.3.0)
C++ Compiler:       g++ (14.3.0)
Fortran Compiler:   gfortran
MPI:                OFF
OPENMP:             OFF
CXX Interface:      ON
Fortran Interface:  OFF
MICM:               ON (v3.11.0)
TUV-X:              ON (v0.14.0)
CARMA:              ON (develop-carma-box)
```

## 4. Build

```bash
make -j4
```

Build completes at 100% producing:

- `lib64/libmusica.a` -- the MUSICA static library
- Test binaries: `test_util`, `test_micm_wrapper`, `test_micm_c_api`,
  `test_parser`, `test_tuvx_c_api`, `test_tuvx_run_from_config`,
  `test_carma_c_api`

## 5. Run tests

```bash
ctest --output-on-failure
```

All 7 tests passed (total time 5.99s):

| # | Test | Result | Time |
|---|------|--------|------|
| 1 | util | Passed | 1.18s |
| 2 | micm_wrapper | Passed | 0.02s |
| 3 | micm_c_api | Passed | 0.04s |
| 4 | parser | Passed | 0.21s |
| 5 | tuvx_c_api | Passed | 4.48s |
| 6 | tuvx_run_from_config | Passed | 0.03s |
| 7 | carma_c_api | Passed | 0.02s |

## 6. Install

```bash
make install
```

Installs to the path set by `CMAKE_INSTALL_PREFIX` (e.g. `/glade/work/$USER/packages`):

```
<install-prefix>/
├── include/
│   ├── micm/          # MICM headers
│   └── musica/        # MUSICA headers, TUV-x and CARMA Fortran modules
│       ├── micm/
│       ├── tuvx/
│       ├── carma/
│       └── fortran/   # .mod files for Fortran interfaces
├── lib64/
│   ├── libmusica.a
│   ├── libmechanism_configuration.a
│   ├── cmake/musica/  # CMake package config files
│   └── pkgconfig/     # pkg-config file (musica.pc)
```

To use the installed library in another CMake project:

```cmake
find_package(musica REQUIRED)
target_link_libraries(my_target musica::musica)
```

Or via pkg-config:

```bash
export PKG_CONFIG_PATH=<install-prefix>/lib64/pkgconfig:$PKG_CONFIG_PATH
pkg-config --cflags --libs musica
```
