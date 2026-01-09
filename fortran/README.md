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
- BLAS
- LAPACK

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

Here's a simple example demonstrating the Fortran interface for MICM (atmospheric chemistry):

```fortran
program musica_example
  use iso_c_binding
  use iso_fortran_env, only: real64
  use musica_micm, only: micm_t, RosenbrockStandardOrder
  use musica_state, only: state_t
  use musica_util, only: error_t

  implicit none

  type(micm_t), pointer       :: micm
  type(state_t), pointer      :: state
  type(error_t)               :: error
  character(len=256)          :: config_path
  integer                     :: solver_type
  integer                     :: num_grid_cells
  real(real64)                :: time_step
  integer                     :: i
  integer                     :: O2_index, O_index, O1D_index, O3_index
  integer                     :: jO2_index, jO3a_index, jO3b_index

  ! Set up configuration
  config_path = "configs/v0/chapman"
  solver_type = RosenbrockStandardOrder
  num_grid_cells = 1
  time_step = 200.0

  ! Create MICM solver instance
  write(*,*) "Creating MICM solver..."
  micm => micm_t(config_path, solver_type, error)
  if (.not. error%is_success()) then
    write(*,*) "Error creating MICM solver: ", error%get_message()
    stop 1
  end if

  ! Create state for the specified number of grid cells
  write(*,*) "Creating state..."
  state => micm%get_state(num_grid_cells, error)
  if (.not. error%is_success()) then
    write(*,*) "Error creating state: ", error%get_message()
    stop 1
  end if

  ! Get species indices
  O2_index = state%species_ordering%index("O2", error)
  O_index = state%species_ordering%index("O", error)
  O1D_index = state%species_ordering%index("O1D", error)
  O3_index = state%species_ordering%index("O3", error)

  ! Set initial concentrations (mol/m³)
  state%concentrations(O2_index, 1) = 0.75
  state%concentrations(O_index, 1) = 0.0
  state%concentrations(O1D_index, 1) = 0.0
  state%concentrations(O3_index, 1) = 0.0

  ! Set environmental conditions
  state%conditions%temperature(1) = 272.5  ! K
  state%conditions%pressure(1) = 101253.3  ! Pa

  ! Get rate parameter indices
  jO2_index = state%rate_parameters_ordering%index("PHOTO.jO2", error)
  jO3a_index = state%rate_parameters_ordering%index("PHOTO.jO3->O", error)
  jO3b_index = state%rate_parameters_ordering%index("PHOTO.jO3->O1D", error)

  ! Set rate parameters
  state%rate_parameters(jO2_index, 1) = 1.0e-5
  state%rate_parameters(jO3a_index, 1) = 1.0e-4
  state%rate_parameters(jO3b_index, 1) = 1.0e-5

  ! Solve for multiple time steps
  write(*,*) "Solving chemistry..."
  do i = 1, 10
    call micm%solve(state, time_step, error)
    if (.not. error%is_success()) then
      write(*,*) "Error solving: ", error%get_message()
      stop 1
    end if
    
    ! Output results
    write(*,'(A,I3,A,E12.4)') "Step ", i, " O3 = ", state%concentrations(O3_index, 1)
  end do

  ! Clean up
  deallocate(micm)
  deallocate(state)

  write(*,*) "Done!"

end program musica_example
```

### Linking Your Fortran Code

To use MUSICA in your Fortran project, link against the musica-fortran library:

```bash
gfortran my_program.f90 -lmusica-fortran -lmusica -lnetcdff -lnetcdf
```

Or in your CMakeLists.txt:

```cmake
find_package(musica REQUIRED)
target_link_libraries(my_program musica::musica-fortran)
```

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
