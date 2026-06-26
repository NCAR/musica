! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! Tutorial Chapter 4: Create solvers for multiple mechanisms that run in the same host application
!
! This program demonstrates how two independent MICM solver instances, each
! configured for a different chemical mechanism, can coexist and run within
! the same Fortran host application.  Each solver has its own state, its own
! species set, and its own reaction set.  They are advanced through time inside
! the same simulation loop, exactly as they would be in a real atmospheric model
! that couples different chemistry modules.
!
! Mechanism 1 – configs/v0/analytical
!   Six species (A, B, C, D, E, F), two Arrhenius reactions, and two
!   user-defined-rate reactions.
!
! Mechanism 2 – configs/v1/analytical/config.json
!   Three species (A, B, C) and two pure Arrhenius reactions.
!
! Both solvers use the standard-ordered Rosenbrock method.

program test_multiple_mechanisms

  use, intrinsic :: iso_c_binding
  use iso_fortran_env, only: real64
  use musica_util, only: assert, error_t, string_t, mapping_t
  use musica_micm, only: micm_t, solver_stats_t
  use musica_micm, only: RosenbrockStandardOrder
  use musica_state, only: conditions_t, state_t

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )

  implicit none

  call multiple_mechanisms_example()

contains

  !> Runs two independent MICM solvers for different mechanisms in one application
  subroutine multiple_mechanisms_example()

    ! --- Mechanism 1 variables --------------------------------------------------
    character(len=256)    :: config_path_1
    type(micm_t), pointer :: micm_1
    type(state_t), pointer :: state_1
    integer               :: num_species_1

    ! --- Mechanism 2 variables --------------------------------------------------
    character(len=256)    :: config_path_2
    type(micm_t), pointer :: micm_2
    type(state_t), pointer :: state_2
    integer               :: num_species_2

    ! --- Shared simulation variables --------------------------------------------
    real(real64), parameter :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1
    real(real64)            :: time_step
    real(real64)            :: temperature, pressure, air_density
    type(string_t)          :: solver_state_str
    type(solver_stats_t)    :: solver_stats
    type(error_t)           :: error
    integer                 :: i, num_grid_cells

    num_grid_cells = 1
    time_step      = 200.0_real64   ! seconds
    temperature    = 272.5_real64   ! K
    pressure       = 101253.3_real64 ! Pa
    air_density    = pressure / (GAS_CONSTANT * temperature)

    ! =========================================================================
    ! Step 1 – Create Solver 1 (six-species analytical mechanism)
    ! =========================================================================
    config_path_1 = "configs/v0/analytical"
    write(*,*) "Creating Solver 1 from: ", trim(config_path_1)
    micm_1 => micm_t(config_path_1, RosenbrockStandardOrder, error)
    ASSERT( error%is_success() )

    state_1 => micm_1%get_state(num_grid_cells, error)
    ASSERT( error%is_success() )

    ! Report the species in Mechanism 1
    num_species_1 = state_1%species_ordering%size()
    write(*,'(A,I0,A)') "  Mechanism 1 has ", num_species_1, " species:"
    do i = 1, num_species_1
      write(*,'(A,A,A,I0)') "    ", trim(state_1%species_ordering%name(i)), &
                             "  index=", state_1%species_ordering%index(i)
    end do

    ! =========================================================================
    ! Step 2 – Create Solver 2 (three-species v1 analytical mechanism)
    ! =========================================================================
    config_path_2 = "configs/v1/analytical/config.json"
    write(*,*) "Creating Solver 2 from: ", trim(config_path_2)
    micm_2 => micm_t(config_path_2, RosenbrockStandardOrder, error)
    ASSERT( error%is_success() )

    state_2 => micm_2%get_state(num_grid_cells, error)
    ASSERT( error%is_success() )

    ! Report the species in Mechanism 2
    num_species_2 = state_2%species_ordering%size()
    write(*,'(A,I0,A)') "  Mechanism 2 has ", num_species_2, " species:"
    do i = 1, num_species_2
      write(*,'(A,A,A,I0)') "    ", trim(state_2%species_ordering%name(i)), &
                             "  index=", state_2%species_ordering%index(i)
    end do

    ! The two mechanisms are independent: they may share species names but each
    ! solver manages its own internal state.
    ASSERT( num_species_1 /= num_species_2 )

    ! =========================================================================
    ! Step 3 – Set initial conditions for both states
    ! =========================================================================

    ! State 1 conditions
    state_1%conditions(1)%temperature = temperature
    state_1%conditions(1)%pressure    = pressure
    state_1%conditions(1)%air_density = air_density

    associate( var_stride => state_1%species_strides%variable )
      do i = 1, num_species_1
        state_1%concentrations(i * var_stride) = 1.0_real64
      end do
    end associate

    ! State 1 user-defined rate parameters (two reactions in v0/analytical)
    associate( var_stride => state_1%rate_parameters_strides%variable )
      state_1%rate_parameters(1)                = 0.001_real64
      state_1%rate_parameters(1 + var_stride)   = 0.002_real64
    end associate

    ! State 2 conditions
    state_2%conditions(1)%temperature = temperature
    state_2%conditions(1)%pressure    = pressure
    state_2%conditions(1)%air_density = air_density

    associate( var_stride => state_2%species_strides%variable )
      do i = 1, num_species_2
        state_2%concentrations(i * var_stride) = 1.0_real64
      end do
    end associate

    ! =========================================================================
    ! Step 4 – Advance both solvers for one time step in the same loop
    ! =========================================================================
    write(*,*) ""
    write(*,*) "Advancing both mechanisms by one time step..."

    call micm_1%solve(time_step, state_1, solver_state_str, solver_stats, error)
    ASSERT( error%is_success() )
    write(*,*) "  Solver 1 result: ", trim(solver_state_str%value_)

    call micm_2%solve(time_step, state_2, solver_state_str, solver_stats, error)
    ASSERT( error%is_success() )
    write(*,*) "  Solver 2 result: ", trim(solver_state_str%value_)

    ! =========================================================================
    ! Step 5 – Print final concentrations for both mechanisms
    ! =========================================================================
    write(*,*) ""
    write(*,*) "Final concentrations – Mechanism 1:"
    associate( var_stride => state_1%species_strides%variable )
      do i = 1, num_species_1
        write(*,'(A,A,A,F12.6)') "    ", &
              trim(state_1%species_ordering%name(i)), " = ", &
              state_1%concentrations(i * var_stride)
      end do
    end associate

    write(*,*) ""
    write(*,*) "Final concentrations – Mechanism 2:"
    associate( var_stride => state_2%species_strides%variable )
      do i = 1, num_species_2
        write(*,'(A,A,A,F12.6)') "    ", &
              trim(state_2%species_ordering%name(i)), " = ", &
              state_2%concentrations(i * var_stride)
      end do
    end associate

    write(*,*) ""
    write(*,*) "Both mechanisms solved successfully in the same host application!"

    ! =========================================================================
    ! Clean up
    ! =========================================================================
    deallocate(state_1)
    deallocate(micm_1)
    deallocate(state_2)
    deallocate(micm_2)

  end subroutine multiple_mechanisms_example

end program test_multiple_mechanisms
