program test_micm_box_model

  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic

  use iso_fortran_env, only : real64
  use musica_util, only: error_t, string_t, mapping_t
  use musica_micm, only: micm_t, solver_stats_t
  use musica_micm, only: Rosenbrock, RosenbrockStandardOrder

  implicit none

  call box_model_arrays()
  call box_model_c_ptrs()

contains

  !> Runs a simple box model using the MICM solver and passing in fortran arrays
  subroutine box_model_arrays()

    character(len=256) :: config_path
    integer            :: solver_type
    integer            :: num_grid_cells

    real(real64), parameter :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1

    real(real64) :: time_step
    real(real64), target :: temperature(1)
    real(real64), target :: pressure(1)
    real(real64), target :: air_density(1)
    real(real64), target :: concentrations(1,6)
    real(real64), target :: user_defined_reaction_rates(1,2)

    type(string_t)       :: solver_state
    type(solver_stats_t) :: solver_stats
    type(error_t)        :: error

    type(micm_t), pointer :: micm

    integer :: i

    config_path = "configs/analytical"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1

    time_step = 200
    temperature(1) = 273.0
    pressure(1) = 1.0e5
    air_density(:) = pressure(:) / (GAS_CONSTANT * temperature(:))

    concentrations(1,:) = (/ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 /)
    user_defined_reaction_rates(1,:) = (/ 0.001, 0.002 /)

    write(*,*) "Creating MICM solver..."
    micm => micm_t(config_path, solver_type, num_grid_cells, error)

    do i = 1, micm%species_ordering%size()
      print *, "Species Name:", micm%species_ordering%name(i), &
               ", Index:", micm%species_ordering%index(i)
    end do

    write(*,*) "Solving starts..."
    call micm%solve(time_step, temperature, pressure, air_density, concentrations, &
        user_defined_reaction_rates, solver_state, solver_stats, error)
    write(*,*) "After solving, concentrations", concentrations

    deallocate( micm )

  end subroutine box_model_arrays

  !> Runs a simple box model using the MICM solver and passing in C pointers
  subroutine box_model_c_ptrs()

    character(len=256) :: config_path
    integer            :: solver_type
    integer            :: num_grid_cells

    real(real64), parameter :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1

    real(real64) :: time_step
    real(real64), target :: temperature(1)
    real(real64), target :: pressure(1)
    real(real64), target :: air_density(1)
    real(real64), target :: concentrations(6)
    real(real64), target :: user_defined_reaction_rates(2)

    type(string_t)       :: solver_state
    type(solver_stats_t) :: solver_stats
    type(error_t)        :: error

    type(micm_t), pointer :: micm

    integer :: i

    config_path = "configs/analytical"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1

    time_step = 200
    temperature(1) = 273.0
    pressure(1) = 1.0e5
    air_density(:) = pressure(:) / (GAS_CONSTANT * temperature(:))

    concentrations = (/ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 /)
    user_defined_reaction_rates = (/ 0.001, 0.002 /)

    write(*,*) "Creating MICM solver..."
    micm => micm_t(config_path, solver_type, num_grid_cells, error)

    do i = 1, micm%species_ordering%size()
      print *, "Species Name:", micm%species_ordering%name(i), &
               ", Index:", micm%species_ordering%index(i)
    end do

    write(*,*) "Solving starts..."
    call micm%solve(time_step, c_loc(temperature), c_loc(pressure), &
                    c_loc(air_density), c_loc(concentrations), &
                    c_loc(user_defined_reaction_rates), &
                    solver_state, solver_stats, error)
    write(*,*) "After solving, concentrations", concentrations

    deallocate( micm )

  end subroutine box_model_c_ptrs

end program test_micm_box_model
