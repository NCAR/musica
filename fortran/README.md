# MUSICA Fortran Interface

MUSICA provides a Fortran interface for atmospheric chemistry modeling, enabling integration with Fortran-based atmospheric models.

## Installation

```bash
spack install musica
```

### Prerequisites

Before building the Fortran interface, you need to install the following dependencies:

- CMake (>= 3.21)
- A Fortran compiler (gfortran, ifort, etc.)
- pkg-config
- NetCDF-C
- NetCDF-Fortran
- BLAS (only needed when building CARMA, `MUSICA_ENABLE_CARMA=ON`, which is the default)
- LAPACK (only needed when building CARMA, `MUSICA_ENABLE_CARMA=ON`, which is the default)

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/NCAR/musica.git
   cd musica
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure with CMake (enable Fortran interface):
   ```bash
   cmake .. \
     -DCMAKE_BUILD_TYPE=Release \
     -DMUSICA_BUILD_FORTRAN_INTERFACE=ON
   ```

   Or use ccmake for interactive configuration:
   ```bash
   ccmake ..
   ```

4. Build and install:
   ```bash
   make -j
   make install
   ```

### CMake Options

The following CMake options control Fortran interface features:

| Option | Default | Description |
| ------ | ------- | ----------- |
| `MUSICA_BUILD_FORTRAN_INTERFACE` | `ON` | Build the Fortran interface |
| `MUSICA_ENABLE_MICM` | `ON` | Include MICM (chemistry solver) |
| `MUSICA_ENABLE_TUVX` | `ON` | Include TUV-x (photolysis calculator) |
| `MUSICA_ENABLE_TESTS` | `ON` | Build test executables |

## Usage Example

Here's a simple example demonstrating a Fortran box model.
Run it from the root of the MUSICA repository, so that the path to `configs/v1/chapman/config.json` is correct.

```fortran
program musica_example
  use iso_fortran_env, only: real64
  use musica_micm,     only: micm_t, solver_stats_t, RosenbrockStandardOrder
  use musica_state,    only: state_t
  use musica_util,     only: error_t, string_t

  implicit none

  real(real64), parameter :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1
  real(real64), parameter :: TIME_STEP    = 200.0_real64            ! s

  type(micm_t),  pointer :: micm
  type(state_t), pointer :: state
  type(error_t)          :: error
  type(string_t)         :: solver_state
  type(solver_stats_t)   :: solver_stats
  integer :: i, O2, O3, jO2, jO3a, jO3b
  integer :: species_stride, rate_stride

  ! Create the solver from a mechanism configuration
  micm => micm_t("configs/v1/chapman/config.json", RosenbrockStandardOrder, error)
  call check(error, "create MICM solver")

  ! Create a state for one grid cell
  state => micm%get_state(1, error)
  call check(error, "create state")

  ! Look up the position of each species and rate parameter
  O2   = state%species_ordering%index("O2", error)
  call check(error, "find O2")
  O3   = state%species_ordering%index("O3", error)
  call check(error, "find O3")
  jO2  = state%rate_parameters_ordering%index("PHOTO.jo2_b", error)
  call check(error, "find PHOTO.jo2_b")
  jO3a = state%rate_parameters_ordering%index("PHOTO.jo3_a", error)
  call check(error, "find PHOTO.jo3_a")
  jO3b = state%rate_parameters_ordering%index("PHOTO.jo3_b", error)
  call check(error, "find PHOTO.jo3_b")

  ! The state arrays are 1-D. With one grid cell, only the variable stride matters.
  species_stride = state%species_strides%variable
  rate_stride    = state%rate_parameters_strides%variable

  ! Environmental conditions
  state%conditions(1)%temperature = 272.5_real64     ! K
  state%conditions(1)%pressure    = 101253.4_real64  ! Pa
  state%conditions(1)%air_density = state%conditions(1)%pressure &
                                    / (GAS_CONSTANT * state%conditions(1)%temperature) ! mol m-3

  ! Initial concentrations (mol m-3)
  state%concentrations(1 + (O2 - 1) * species_stride) = 0.75_real64
  state%concentrations(1 + (O3 - 1) * species_stride) = 0.0_real64

  ! Photolysis rate constants (s-1)
  state%rate_parameters(1 + (jO2  - 1) * rate_stride) = 1.0e-9_real64
  state%rate_parameters(1 + (jO3a - 1) * rate_stride) = 1.13e-9_real64
  state%rate_parameters(1 + (jO3b - 1) * rate_stride) = 5.8e-8_real64

  do i = 1, 10
    call micm%solve(TIME_STEP, state, solver_state, solver_stats, error)
    call check(error, "solve")
    write(*,'(A,I3,A,ES12.4)') "Step ", i, "  O3 = ", &
        state%concentrations(1 + (O3 - 1) * species_stride)
  end do

  deallocate(state)
  deallocate(micm)

contains

  subroutine check(err, what)
    type(error_t),    intent(in) :: err
    character(len=*), intent(in) :: what
    type(string_t) :: message
    if (.not. err%is_success()) then
      message = err%message()
      write(*,*) "Error: ", what, ": ", message%get_char_array()
      error stop 1
    end if
  end subroutine check

end program musica_example
```

Output:
```
Step   1  O3 =   3.0000E-07
Step   2  O3 =   6.0000E-07
Step   3  O3 =   9.0000E-07
Step   4  O3 =   1.2000E-06
Step   5  O3 =   1.5000E-06
Step   6  O3 =   1.8000E-06
Step   7  O3 =   2.1000E-06
Step   8  O3 =   2.4000E-06
Step   9  O3 =   2.7000E-06
Step  10  O3 =   3.0000E-06
```

### Linking Your Fortran Code

The Fortran interface is provided as a separate `musica-fortran` package.
It automatically includes the C++ library, so only one package call is needed.

In your CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_project LANGUAGES Fortran)

# MUSICA is a C++ library; a Fortran-only project must enable CXX or the link fails
enable_language(CXX)

find_package(musica-fortran REQUIRED)
add_executable(my_program my_program.f90)
target_link_libraries(my_program musica::musica-fortran)

set_target_properties(my_program PROPERTIES LINKER_LANGUAGE Fortran)
```

Point CMake at the install with `-DCMAKE_PREFIX_PATH=<musica prefix>`. This also
defines `musica::musica` if you need the C++ library directly.

A project that already calls `find_package(musica)` does not need the second call.
When MUSICA is built with the Fortran interface, `find_package(musica)` also defines
`musica::musica-fortran`.

## Development

### Building Tests

The Fortran interface includes comprehensive tests. To build and run them:

```bash
cd build
cmake .. -DMUSICA_BUILD_FORTRAN_INTERFACE=ON
make
ctest
# or
make test
```

### Running Individual Tests

After building, test executables are in the build directory:

```bash
# Run MICM API test
./test_micm_api

# Run TUV-x API test
./test_tuvx_api
```

### Code Style

Fortran code in MUSICA follows these conventions:

- Use modern Fortran (Fortran 2003+) features
- Implicit none in all program units
- Use modules to organize code
- Follow the existing naming conventions (lowercase with underscores)

### Integration with Host Models

The Fortran interface is designed for easy integration with existing atmospheric models. The key steps are:

1. Initialize MICM/TUV-x solvers during model initialization
2. Create and maintain state objects for each grid cell or column
3. Call solvers at each time step with appropriate environmental conditions
4. Extract updated concentrations and rates

## More Information

- [Full Documentation](https://ncar.github.io/musica/index.html)
- [Contributing Guide](../CONTRIBUTING.md)
