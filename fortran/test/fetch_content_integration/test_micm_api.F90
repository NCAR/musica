! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program test_micm_api

  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic
  use iso_fortran_env, only: real64
  use musica_util, only: assert, error_t, mapping_t, string_t, find_mapping_index
  use musica_micm, only: micm_t, solver_stats_t, get_micm_version, is_cuda_available
  use musica_micm, only: Rosenbrock, RosenbrockStandardOrder, BackwardEuler, BackwardEulerStandardOrder, CudaRosenbrock
  use musica_state, only: conditions_t, state_t

#include "micm/util/error.hpp"
#include "musica/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )
#define ASSERT_NEAR( a, b, tol ) call assert( abs(a - b) < abs(a + b) * tol, __FILE__, __LINE__ )
#define ASSERT_GT( a, b ) call assert( a > b, __FILE__, __LINE__ )

  implicit none

  type :: ArrheniusReaction
    real(real64) :: A_ = 1.0
    real(real64) :: B_ = 0.0
    real(real64) :: C_ = 0.0
    real(real64) :: D_ = 300.0
    real(real64) :: E_ = 0.0
  end type ArrheniusReaction

  call test_api()
  call test_multiple_grid_cell_standard_Rosenbrock()
  call test_multiple_grid_cell_standard_BackwardEuler()
  call test_api_v1_parser()
  call test_multiple_grid_cell_vector_Rosenbrock()
  call test_multiple_grid_cell_vector_BackwardEuler()
  call test_multiple_grid_cell_cuda_Rosenbrock()
  
contains

  function calculate_arrhenius( reaction, temperature, pressure ) result( rate )
    type(ArrheniusReaction), intent(in) :: reaction
    real(real64), intent(in) :: temperature
    real(real64), intent(in) :: pressure
    real(real64) :: rate
    rate = reaction%A_ * exp( reaction%C_ / temperature ) &
           * (temperature / reaction%D_) ** reaction%B_ &
           * (1.0 + reaction%E_ * pressure)
  end function calculate_arrhenius

  subroutine test_api()

    type(string_t)                        :: micm_version
    type(micm_t), pointer                 :: micm
    type(state_t), pointer                :: state
    real(real64)                          :: time_step
    character(len=256)                    :: config_path
    integer                               :: solver_type
    integer                               :: num_grid_cells
    character(len=:), allocatable         :: string_value
    real(real64)                          :: double_value
    integer(c_int)                        :: int_value
    logical(c_bool)                       :: bool_value
    type(string_t)                        :: solver_state
    type(solver_stats_t)                  :: solver_stats
    type(error_t)                         :: error
    real(real64), parameter               :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1
    integer                               :: i
    integer                               :: O2_index, O_index, O1D_index, O3_index
    integer                               :: jO2_index, jO3a_index, jO3b_index
    
    config_path = "configs/chapman"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1
    time_step = 200

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, solver_type, error)

    write(*,*) "Creating State..."
    state => micm%get_state(num_grid_cells, error)

    ASSERT( error%is_success() )

    O2_index = state%species_ordering%index( "O2", error )
    ASSERT( error%is_success() )
    O_index = state%species_ordering%index( "O", error )
    ASSERT( error%is_success() )
    O1D_index = state%species_ordering%index( "O1D", error )
    ASSERT( error%is_success() )
    O3_index = state%species_ordering%index( "O3", error )
    ASSERT( error%is_success() )

    jO2_index = state%rate_parameters_ordering%index( "PHOTO.jO2", error )
    ASSERT( error%is_success() )
    jO3a_index = state%rate_parameters_ordering%index( "PHOTO.jO3->O", error )
    ASSERT( error%is_success() )
    jO3b_index = state%rate_parameters_ordering%index( "PHOTO.jO3->O1D", error )
    ASSERT( error%is_success() )

    state%conditions(1)%temperature = 272.5
    state%conditions(1)%pressure = 101253.4
    state%conditions(1)%air_density = state%conditions(1)%pressure / ( GAS_CONSTANT * state%conditions(1)%temperature )

    associate( var_stride => state%species_strides%variable ) 
      state%concentrations(  1 + (O2_index   - 1) * var_stride ) = 0.75
      state%concentrations(  1 + (O_index    - 1) * var_stride ) = 0.0
      state%concentrations(  1 + (O1D_index  - 1) * var_stride ) = 0.0
      state%concentrations(  1 + (O3_index   - 1) * var_stride ) = 0.0000081
    end associate
    associate( var_stride => state%rate_parameters_strides%variable )
      state%rate_parameters( 1 + (jO2_index  - 1) * var_stride ) = 2.7e-19
      state%rate_parameters( 1 + (jO3a_index - 1) * var_stride ) = 1.13e-9
      state%rate_parameters( 1 + (jO3b_index - 1) * var_stride ) = 5.8e-8
    end associate
    
    micm_version = get_micm_version()
    print *, "[test micm fort api] MICM version ", micm_version%get_char_array()

    do i = 1, state%species_ordering%size()
      print *, "Species Name:", state%species_ordering%name(i), &
               ", Index:", state%species_ordering%index(i)
    end do
    do i = 1, state%rate_parameters_ordering%size()
      print *, "User Defined Reaction Rate Name:", state%rate_parameters_ordering%name(i), &
               ", Index:", state%rate_parameters_ordering%index(i)
    end do

    write(*,*) "[test micm fort api] Initial concentrations", state%concentrations

    write(*,*) "[test micm fort api] Solving starts..."
    call micm%solve(time_step, state, solver_state, solver_stats, error)
    ASSERT( error%is_success() )

    write(*,*) "[test micm fort api] After solving, concentrations: ", state%concentrations
    write(*,*) "[test micm fort api] Solver state: ", solver_state%get_char_array()
    ASSERT_EQ( solver_state%get_char_array(), "Converged" )
    write(*,*) "[test micm fort api] Function calls: ", solver_stats%function_calls()
    write(*,*) "[test micm fort api] Jacobian updates: ", solver_stats%jacobian_updates()
    write(*,*) "[test micm fort api] Number of steps: ", solver_stats%number_of_steps()
    write(*,*) "[test micm fort api] Accepted: ", solver_stats%accepted()
    write(*,*) "[test micm fort api] Rejected: ", solver_stats%rejected()
    write(*,*) "[test micm fort api] Decompositions: ", solver_stats%decompositions()
    write(*,*) "[test micm fort api] Solves: ", solver_stats%solves()
    write(*,*) "[test micm fort api] Final time: ", solver_stats%final_time()

    string_value = micm%get_species_property_string( "O3", "__long name", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( string_value, "ozone" )
    deallocate( string_value )
    double_value = micm%get_species_property_double( "O3", "molecular weight [kg mol-1]", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( double_value, 0.048_real64 )
    int_value = micm%get_species_property_int( "O3", "__atoms", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( int_value, 3 )
    bool_value = micm%get_species_property_bool( "O3", "__do advect", error )
    ASSERT( error%is_success() )
    ASSERT( logical( bool_value ) )

    string_value = micm%get_species_property_string( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    double_value = micm%get_species_property_double( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    int_value = micm%get_species_property_int( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    bool_value = micm%get_species_property_bool( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    deallocate(micm)
    deallocate(state)
    micm => micm_t( "configs/invalid", solver_type, error )
    ASSERT( error%is_error( MUSICA_ERROR_CATEGORY_PARSING, MUSICA_PARSE_INVALID_CONFIG_FILE ) )
    ASSERT( .not. associated( micm ) )

    write(*,*) "[test micm fort api] Finished."

  end subroutine test_api

  subroutine test_solver(micm, number_of_grid_cells, time_step, test_accuracy)

    type(micm_t), intent(inout) :: micm
    integer,      intent(in)    :: number_of_grid_cells
    real(real64), intent(in)    :: time_step
    real(real64), intent(in)    :: test_accuracy

    integer, parameter        :: NUM_SPECIES = 6
    integer, parameter        :: NUM_RATE_PARAMETERS = 2
    real(real64), parameter   :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1

    type(error_t)             :: error
    type(state_t), pointer    :: state_1, state_2, state
    integer                   :: max_grid_cells, state_1_size, state_2_size, state_size
    integer                   :: A_index, B_index, C_index, D_index, E_index, F_index
    integer                   :: R1_index, R2_index
    real, allocatable         :: initial_concentrations(:,:)
    real                      :: rand_num
    type(string_t)            :: solver_state
    type(solver_stats_t)      :: solver_stats
    integer                   :: i_state, i_cell, i_species
    type(ArrheniusReaction)   :: arr1, arr2
    real(real64)              :: initial_A, initial_C, initial_D, initial_F
    real(real64)              :: k1, k2, k3, k4
    real(real64)              :: A, B, C, D, E, F

    max_grid_cells = micm%get_maximum_number_of_grid_cells( )
    ASSERT_GT( max_grid_cells, 0 )
    state_1_size = min( number_of_grid_cells, max_grid_cells )
    state_2_size = mod( (number_of_grid_cells - state_1_size), max_grid_cells )
    state_1 => micm%get_state( state_1_size, error )
    ASSERT( error%is_success() )
    state_2 => null()
    if ( state_2_size > 0 ) then
      state_2 => micm%get_state( state_2_size, error )
      ASSERT( error%is_success() )
    end if

    ! Get the species indices in the concentration array
    A_index = state_1%species_ordering%index( "A", error )
    ASSERT( error%is_success() )
    B_index = state_1%species_ordering%index( "B", error )
    ASSERT( error%is_success() )
    C_index = state_1%species_ordering%index( "C", error )
    ASSERT( error%is_success() )
    D_index = state_1%species_ordering%index( "D", error )
    ASSERT( error%is_success() )
    E_index = state_1%species_ordering%index( "E", error )
    ASSERT( error%is_success() )
    F_index = state_1%species_ordering%index( "F", error )
    ASSERT( error%is_success() )

    ! Get the rate parameter indices in the rate parameter array
    R1_index = state_1%rate_parameters_ordering%index( "USER.reaction 1", error )
    ASSERT( error%is_success() )
    R2_index = state_1%rate_parameters_ordering%index( "USER.reaction 2", error )
    ASSERT( error%is_success() )

    do i_state = 1, ceiling( real(number_of_grid_cells) / state_1_size)

      ! Get the right state and set dimensions
      state_size = min(number_of_grid_cells - (i_state - 1) * state_1_size, state_1_size)
      if ( state_size == state_1_size ) then
        state => state_1
      else
        state => state_2
        ASSERT( associated( state_2 ) )
      end if
      ASSERT_EQ( state%number_of_grid_cells, state_size )
      ASSERT_EQ( state%number_of_species, NUM_SPECIES )
      ASSERT_EQ( state%number_of_rate_parameters, NUM_RATE_PARAMETERS )

      ! Set the initial conditions
      allocate( initial_concentrations( state_size, NUM_SPECIES ) )
      do i_cell = 1, state_size
        call random_number( rand_num )
        state%conditions(i_cell)%temperature = 275.0 + rand_num * 20.0 - 10.0
        call random_number( rand_num )
        state%conditions(i_cell)%pressure = 101253.3 + rand_num * 1000.0
        state%conditions(i_cell)%air_density = state%conditions(i_cell)%pressure / &
            ( GAS_CONSTANT * state%conditions(i_cell)%temperature )
        associate( cell_stride => state%species_strides%grid_cell, &
                   var_stride => state%species_strides%variable )
          call random_number( rand_num )
          state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( A_index - 1 ) * var_stride ) = &
              0.75 + rand_num * 0.1 - 0.05
          call random_number( rand_num )
          state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( B_index - 1 ) * var_stride ) = &
              0.0
          call random_number( rand_num )
          state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( C_index - 1 ) * var_stride ) = &
              0.4 + rand_num * 0.1 - 0.05
          call random_number( rand_num )
          state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( D_index - 1 ) * var_stride ) = &
              0.8 + rand_num * 0.1 - 0.05
          call random_number( rand_num )
          state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( E_index - 1 ) * var_stride ) = &
              0.0
          call random_number( rand_num )
          state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( F_index - 1 ) * var_stride ) = &
              0.1 + rand_num * 0.1 - 0.05
          do i_species = 1, NUM_SPECIES
            initial_concentrations( i_cell, i_species ) = &
                state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( i_species - 1 ) * var_stride )
          end do
        end associate
        associate( cell_stride => state%rate_parameters_strides%grid_cell, &
                   var_stride => state%rate_parameters_strides%variable )
          call random_number( rand_num )
          state%rate_parameters( 1 + ( i_cell - 1 ) * cell_stride + ( R1_index - 1 ) * var_stride ) = &
              0.001 + rand_num * 0.001 - 0.0005
          call random_number( rand_num )
          state%rate_parameters( 1 + ( i_cell - 1 ) * cell_stride + ( R2_index - 1 ) * var_stride ) = &
              0.002 + rand_num * 0.001 - 0.0005
        end associate
      end do

      call micm%solve(time_step, state, solver_state, solver_stats, error)
      ASSERT( error%is_success() )
      ASSERT_EQ(solver_state%get_char_array(), "Converged")

      arr1%A_ = 0.004
      arr1%C_ = 50.0
      arr2%A_ = 0.012
      arr2%B_ = -2.0
      arr2%C_ = 75.0
      arr2%D_ = 50.0
      arr2%E_ = 1.0e-6

      do i_cell = 1, state_size
        initial_A = initial_concentrations( i_cell, A_index )
        initial_C = initial_concentrations( i_cell, C_index )
        initial_D = initial_concentrations( i_cell, D_index )
        initial_F = initial_concentrations( i_cell, F_index )
        associate( cell_stride => state%rate_parameters_strides%grid_cell, &
                   var_stride => state%rate_parameters_strides%variable )
          k1 = state%rate_parameters( 1 + ( i_cell - 1 ) * cell_stride + ( R1_index - 1 ) * var_stride )
          k2 = state%rate_parameters( 1 + ( i_cell - 1 ) * cell_stride + ( R2_index - 1 ) * var_stride )
        end associate
        k3 = calculate_arrhenius( arr1, state%conditions(i_cell)%temperature, &
            state%conditions(i_cell)%pressure )
        k4 = calculate_arrhenius( arr2, state%conditions(i_cell)%temperature, &
            state%conditions(i_cell)%pressure )
        A = initial_A * exp( -k3 * time_step )
        B = initial_A * (k3 / (k4 - k3)) * &
            (exp(-k3 * time_step) - exp(-k4 * time_step))
        C = initial_C + initial_A * &
            (1.0 + (k3 * exp(-k4 * time_step) - k4 * exp(-k3 * time_step)) / (k4 - k3))
        D = initial_D * exp( -k1 * time_step )
        E = initial_D * (k1 / (k2 - k1)) * &
            (exp(-k1 * time_step) - exp(-k2 * time_step))
        F = initial_F + initial_D * &
            (1.0 + (k1 * exp(-k2 * time_step) - k2 * exp(-k1 * time_step)) / (k2 - k1))
        associate( cell_stride => state%species_strides%grid_cell, &
                   var_stride  => state%species_strides%variable )
          ASSERT_NEAR( state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( A_index - 1 ) * var_stride ), A, test_accuracy )
          ASSERT_NEAR( state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( B_index - 1 ) * var_stride ), B, test_accuracy )
          ASSERT_NEAR( state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( C_index - 1 ) * var_stride ), C, test_accuracy )
          ASSERT_NEAR( state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( D_index - 1 ) * var_stride ), D, test_accuracy )
          ASSERT_NEAR( state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( E_index - 1 ) * var_stride ), E, test_accuracy )
          ASSERT_NEAR( state%concentrations( 1 + ( i_cell - 1 ) * cell_stride + ( F_index - 1 ) * var_stride ), F, test_accuracy )
        end associate
      end do
      deallocate( initial_concentrations )
    end do
    deallocate( state_1 )
    if ( associated( state_2 ) ) deallocate( state_2 )
  end subroutine test_solver

  subroutine test_multiple_grid_cell_vector_Rosenbrock()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 200
    real(real64), parameter :: test_accuracy = 5.0e-3
    integer                 :: max_grid_cells

    micm => micm_t( "configs/analytical", Rosenbrock, error )
    ASSERT( error%is_success() )
    max_grid_cells = micm%get_maximum_number_of_grid_cells( )
    ASSERT_GT( max_grid_cells, 0 )
    do num_grid_cells = 1, max_grid_cells * 3, floor( real(max_grid_cells) / 3 )
      call test_solver( micm, num_grid_cells, time_step, test_accuracy )
    end do
    deallocate( micm )

  end subroutine test_multiple_grid_cell_vector_Rosenbrock

  subroutine test_multiple_grid_cell_standard_Rosenbrock()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 200
    real(real64), parameter :: test_accuracy = 5.0e-3
    integer                 :: max_grid_cells

    micm => micm_t( "configs/analytical", RosenbrockStandardOrder, error )
    ASSERT( error%is_success() )
    max_grid_cells = micm%get_maximum_number_of_grid_cells( )
    ASSERT_GT( max_grid_cells, 1e8 )
    do num_grid_cells = 1, 20, 5
      call test_solver( micm, num_grid_cells, time_step, test_accuracy )
    end do
    deallocate( micm )

  end subroutine test_multiple_grid_cell_standard_Rosenbrock

  subroutine test_multiple_grid_cell_vector_BackwardEuler()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 10
    real(real64), parameter :: test_accuracy = 0.1
    integer                 :: max_grid_cells

    micm => micm_t( "configs/analytical", BackwardEuler, error )
    ASSERT( error%is_success() )
    max_grid_cells = micm%get_maximum_number_of_grid_cells( )
    ASSERT_GT( max_grid_cells, 0 )
    do num_grid_cells = 1, max_grid_cells * 3, floor( real(max_grid_cells) / 3 )
      call test_solver( micm, num_grid_cells, time_step, test_accuracy )
    end do
    deallocate( micm )

  end subroutine test_multiple_grid_cell_vector_BackwardEuler

  subroutine test_multiple_grid_cell_standard_BackwardEuler()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 10
    real(real64), parameter :: test_accuracy = 0.1
    integer                 :: max_grid_cells

    micm => micm_t( "configs/analytical", BackwardEulerStandardOrder, error )
    ASSERT( error%is_success() )
    max_grid_cells = micm%get_maximum_number_of_grid_cells( )
    ASSERT_GT( max_grid_cells, 1e8 )
    do num_grid_cells = 1, 20, 5
      call test_solver( micm, num_grid_cells, time_step, test_accuracy )
    end do
    deallocate( micm )

  end subroutine test_multiple_grid_cell_standard_BackwardEuler

  subroutine test_multiple_grid_cell_cuda_Rosenbrock()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 200
    real(real64), parameter :: test_accuracy = 5.0e-3
    integer                 :: max_grid_cells

    logical :: cuda_available

    cuda_available = is_cuda_available(error)
    ASSERT( error%is_success() )

    if ( .not. cuda_available ) then
      write(*,*) "CUDA is not available, skipping test."
      return
    end if

    micm => micm_t( "configs/analytical", CudaRosenbrock, error )
    ASSERT( error%is_success() )
    max_grid_cells = micm%get_maximum_number_of_grid_cells( )
    ASSERT_GT( max_grid_cells, 0 )
    do num_grid_cells = 1, max_grid_cells * 3, floor( real(max_grid_cells) / 3 )
      call test_solver( micm, num_grid_cells, time_step, test_accuracy )
    end do
    deallocate( micm )

  end subroutine test_multiple_grid_cell_cuda_Rosenbrock

  subroutine test_api_v1_parser()

    type(string_t)                :: micm_version
    type(micm_t), pointer         :: micm
    type(state_t), pointer        :: state
    real(real64)                  :: time_step
    type(conditions_t), target    :: conds(1)
    character(len=256)            :: config_path
    integer                       :: solver_type
    integer                       :: num_grid_cells
    character(len=:), allocatable :: string_value
    real(real64)                  :: double_value
    integer(c_int)                :: int_value
    logical(c_bool)               :: bool_value
    type(string_t)                :: solver_state
    type(solver_stats_t)          :: solver_stats
    type(error_t)                 :: error
    real(real64), parameter       :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1
    integer                       :: i
    integer                       :: O2_index, O_index, O1D_index, O3_index
    integer                       :: jO2_index, jO3a_index, jO3b_index
    
    config_path = "configs/v1/chapman/config.json"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1
    time_step = 200

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, solver_type, error)
    ASSERT( error%is_success() )

    write(*,*) "Creating State..."
    state => micm%get_state(num_grid_cells, error)

    O2_index = state%species_ordering%index( "O2", error )
    ASSERT( error%is_success() )
    O_index = state%species_ordering%index( "O", error )
    ASSERT( error%is_success() )
    O1D_index = state%species_ordering%index( "O1D", error )
    ASSERT( error%is_success() )
    O3_index = state%species_ordering%index( "O3", error )
    ASSERT( error%is_success() )

    jO2_index = state%rate_parameters_ordering%index( "PHOTO.jO2", error )
    ASSERT( error%is_success() )
    jO3a_index = state%rate_parameters_ordering%index( "PHOTO.jO3->O", error )
    ASSERT( error%is_success() )
    jO3b_index = state%rate_parameters_ordering%index( "PHOTO.jO3->O1D", error )
    ASSERT( error%is_success() )

    state%conditions(1)%temperature = 72.5
    state%conditions(1)%pressure = 101253.4
    state%conditions(1)%air_density = state%conditions(1)%pressure / ( GAS_CONSTANT * state%conditions(1)%temperature )

    associate( var_stride => state%species_strides%variable )
      state%concentrations( 1 + (O2_index -  1) * var_stride) = 0.75
      state%concentrations( 1 + (O_index   - 1) * var_stride ) = 0.0
      state%concentrations( 1 + (O1D_index - 1) * var_stride ) = 0.0
      state%concentrations( 1 + (O3_index  - 1) * var_stride ) = 0.0000081
    end associate
    associate( var_stride => state%rate_parameters_strides%variable )
      state%rate_parameters(1 + (jO2_index - 1) * var_stride ) = 2.7e-19
      state%rate_parameters(1 + (jO3a_index- 1) * var_stride ) = 1.13e-9
      state%rate_parameters(1 + (jO3b_index- 1) * var_stride ) = 5.8e-8
    end associate
    micm_version = get_micm_version()
    print *, "[test micm fort api] MICM version ", micm_version%get_char_array()

    do i = 1, state%species_ordering%size()
      print *, "Species Name:", state%species_ordering%name(i), &
               ", Index:", state%species_ordering%index(i)
    end do
    do i = 1, state%rate_parameters_ordering%size()
      print *, "User Defined Reaction Rate Name:", state%rate_parameters_ordering%name(i), &
               ", Index:", state%rate_parameters_ordering%index(i)
    end do

    write(*,*) "[test micm fort api] Initial concentrations", state%concentrations

    write(*,*) "[test micm fort api] Solving starts..."
    call micm%solve(time_step, state, solver_state, solver_stats, error)
    ASSERT( error%is_success() )

    write(*,*) "[test micm fort api] After solving, concentrations: ", state%concentrations
    write(*,*) "[test micm fort api] Solver state: ", solver_state%get_char_array()
    ASSERT_EQ( solver_state%get_char_array(), "Converged" )
    write(*,*) "[test micm fort api] Function calls: ", solver_stats%function_calls()
    write(*,*) "[test micm fort api] Jacobian updates: ", solver_stats%jacobian_updates()
    write(*,*) "[test micm fort api] Number of steps: ", solver_stats%number_of_steps()
    write(*,*) "[test micm fort api] Accepted: ", solver_stats%accepted()
    write(*,*) "[test micm fort api] Rejected: ", solver_stats%rejected()
    write(*,*) "[test micm fort api] Decompositions: ", solver_stats%decompositions()
    write(*,*) "[test micm fort api] Solves: ", solver_stats%solves()
    write(*,*) "[test micm fort api] Final time: ", solver_stats%final_time()

    string_value = micm%get_species_property_string( "O3", "__long name", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( string_value, "ozone" )
    deallocate( string_value )
    double_value = micm%get_species_property_double( "O3", "molecular weight [kg mol-1]", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( double_value, 0.048_real64 )
    int_value = micm%get_species_property_int( "O3", "__atoms", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( int_value, 3 )
    bool_value = micm%get_species_property_bool( "O3", "__do advect", error )
    ASSERT( error%is_success() )
    ASSERT( logical( bool_value ) )

    string_value = micm%get_species_property_string( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    double_value = micm%get_species_property_double( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    int_value = micm%get_species_property_int( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    bool_value = micm%get_species_property_bool( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    deallocate( micm )
    deallocate( state )
    micm => micm_t( "configs/invalid", solver_type, error )
    ASSERT( error%is_error( MUSICA_ERROR_CATEGORY_PARSING, MUSICA_PARSE_INVALID_CONFIG_FILE ) )
    ASSERT( .not. associated( micm ) )

    write(*,*) "[test micm fort api] Finished."

  end subroutine test_api_v1_parser
 
end program
