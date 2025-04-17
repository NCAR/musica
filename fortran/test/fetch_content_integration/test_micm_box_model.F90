program test_micm_box_model

  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic

  use iso_fortran_env, only : real64
  use musica_util, only: error_t, string_t, mapping_t
  use musica_micm, only: micm_t, solver_stats_t
  use musica_micm, only: Rosenbrock, RosenbrockStandardOrder
  use musica_state, only: conditions_t, state_t

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
    real(real64), target :: concentrations(6)

    type(string_t)       :: solver_state
    type(solver_stats_t) :: solver_stats
    type(error_t)        :: error

    type(micm_t), pointer :: micm
    type(state_t), pointer :: state

    integer :: i

    config_path = "configs/analytical"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1

    write(*,*) "Creating MICM solver..."
    micm => micm_t(config_path, solver_type, num_grid_cells, error)  

    write(*,*) "Creating State..."    
    state => micm%get_state(error)

    time_step = 200

    state%conditions(1)%temperature = 273.0
    state%conditions(1)%pressure    = 1.0e5
    state%conditions(1)%air_density = state%conditions(1)%pressure / (GAS_CONSTANT * state%conditions(1)%temperature)

    state%concentrations = reshape((/ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 /), shape=[6,1])
    state%rates = reshape((/ 0.001, 0.002 /), shape=[2,1])
    
    do i = 1, state%species_ordering%size()
      print *, "Species Name:", state%species_ordering%name(i), &
               ", Index:", state%species_ordering%index(i)
    end do

    write(*,*) "Solving starts..."
    print *, "Montek TEST: ", c_loc(state%conditions)
    call micm%solve(time_step, state, solver_state, solver_stats, error)
    write(*,*) "After solving, concentrations", state%concentrations
    deallocate( micm )
    deallocate( state )
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

    type(string_t)       :: solver_state
    type(solver_stats_t) :: solver_stats
    type(error_t)        :: error

    type(micm_t), pointer :: micm
    type(state_t), pointer :: state

    integer :: i

    config_path = "configs/analytical"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1

    time_step = 200

    write(*,*) "Creating MICM solver..."
    micm => micm_t(config_path, solver_type, num_grid_cells, error)
    
    write(*,*) "Creating State..."
    state => micm%get_state(error)

    state%conditions(1)%temperature = 273.0
    state%conditions(1)%pressure    = 1.0e5
    state%conditions(1)%air_density = state%conditions(1)%pressure / (GAS_CONSTANT * state%conditions(1)%temperature)

    state%concentrations = reshape((/ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 /), shape=[6,1])
    state%rates = reshape((/ 0.001, 0.002 /), shape=[2,1])

    do i = 1, state%species_ordering%size()
      print *, "Species Name:", state%species_ordering%name(i), &
               ", Index:", state%species_ordering%index(i)
    end do

    write(*,*) "Solving starts..."
    call micm%solve(time_step, state, solver_state, solver_stats, error)
    write(*,*) "After solving, concentrations", state%concentrations

    deallocate( micm )
    deallocate( state )
  end subroutine box_model_c_ptrs

end program test_micm_box_model
