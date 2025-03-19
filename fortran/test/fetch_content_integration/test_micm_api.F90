! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program test_micm_api

  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic
  use iso_fortran_env, only: real64
  use musica_micm, only: micm_t, solver_stats_t, get_micm_version
  use musica_micm, only: Rosenbrock, RosenbrockStandardOrder, BackwardEuler, BackwardEulerStandardOrder
  use musica_util, only: assert, error_t, mapping_t, string_t, find_mapping_index

#include "micm/util/error.hpp"
#include "musica/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )
#define ASSERT_NEAR( a, b, tol ) call assert( abs(a - b) < abs(a + b) * tol, __FILE__, __LINE__ )

#ifndef MICM_VECTOR_MATRIX_SIZE
  #define MICM_VECTOR_MATRIX_SIZE 4
#endif

  implicit none

  type :: ArrheniusReaction
    real(real64) :: A_ = 1.0
    real(real64) :: B_ = 0.0
    real(real64) :: C_ = 0.0
    real(real64) :: D_ = 300.0
    real(real64) :: E_ = 0.0
  end type ArrheniusReaction

  call test_api()
  call test_multiple_grid_cell_vector_Rosenbrock()
  call test_multiple_grid_cell_standard_Rosenbrock()
  call test_multiple_grid_cell_vector_BackwardEuler()
  call test_multiple_grid_cell_standard_BackwardEuler()
  call test_api_v1_parser()

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

    type(string_t)                :: micm_version
    type(micm_t), pointer         :: micm
    real(real64)                  :: time_step
    real(real64)                  :: temperature(1)
    real(real64)                  :: pressure(1)
    real(real64)                  :: air_density(1)
    real(real64), dimension(4,1)  :: concentrations 
    real(real64), dimension(3,1)  :: user_defined_reaction_rates 
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
    
    config_path = "configs/chapman"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1
    time_step = 200

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, solver_type, num_grid_cells, error)
    ASSERT( error%is_success() )

    O2_index = micm%species_ordering%index( "O2", error )
    ASSERT( error%is_success() )
    O_index = micm%species_ordering%index( "O", error )
    ASSERT( error%is_success() )
    O1D_index = micm%species_ordering%index( "O1D", error )
    ASSERT( error%is_success() )
    O3_index = micm%species_ordering%index( "O3", error )
    ASSERT( error%is_success() )

    jO2_index = micm%user_defined_reaction_rates%index( "PHOTO.jO2", error )
    ASSERT( error%is_success() )
    jO3a_index = micm%user_defined_reaction_rates%index( "PHOTO.jO3->O", error )
    ASSERT( error%is_success() )
    jO3b_index = micm%user_defined_reaction_rates%index( "PHOTO.jO3->O1D", error )
    ASSERT( error%is_success() )

    temperature(1) = 272.5
    pressure(1) = 101253.4
    air_density(:) = pressure(:) / ( GAS_CONSTANT * temperature(:) )

    concentrations(O2_index,1) = 0.75
    concentrations(O_index,1) = 0.0
    concentrations(O1D_index,1) = 0.0
    concentrations(O3_index,1) = 0.0000081
    user_defined_reaction_rates(jO2_index,1) = 2.7e-19
    user_defined_reaction_rates(jO3a_index,1) = 1.13e-9
    user_defined_reaction_rates(jO3b_index,1) = 5.8e-8
    
    micm_version = get_micm_version()
    print *, "[test micm fort api] MICM version ", micm_version%get_char_array()

    do i = 1, micm%species_ordering%size()
      print *, "Species Name:", micm%species_ordering%name(i), &
               ", Index:", micm%species_ordering%index(i)
    end do
    do i = 1, micm%user_defined_reaction_rates%size()
      print *, "User Defined Reaction Rate Name:", micm%user_defined_reaction_rates%name(i), &
               ", Index:", micm%user_defined_reaction_rates%index(i)
    end do

    write(*,*) "[test micm fort api] Initial concentrations", concentrations

    write(*,*) "[test micm fort api] Solving starts..."
    call micm%solve(time_step, temperature, pressure,  air_density, concentrations, &
        user_defined_reaction_rates, solver_state, solver_stats, error)
    ASSERT( error%is_success() )

    write(*,*) "[test micm fort api] After solving, concentrations: ", concentrations
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
    micm => micm_t( "configs/invalid", solver_type, num_grid_cells, error )
    ASSERT( error%is_error( MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED ) )
    ASSERT( .not. associated( micm ) )

    write(*,*) "[test micm fort api] Finished."

  end subroutine test_api

  subroutine test_vector_multiple_grid_cells(micm, NUM_GRID_CELLS, time_step, test_accuracy)

    type(micm_t), pointer, intent(inout) :: micm
    integer,               intent(in)    :: NUM_GRID_CELLS
    real(real64),          intent(in)    :: time_step
    real,                  intent(in)    :: test_accuracy

    integer, parameter        :: NUM_SPECIES = 6
    integer, parameter        :: NUM_USER_DEFINED_REACTION_RATES = 2
    ! set up arrays to pass to MICM as slices to ensure contiguous memory is passed to c functions
    real(real64), target      :: temperature(2,NUM_GRID_CELLS)
    real(real64), target      :: temperature_c_ptrs(NUM_GRID_CELLS)
    real(real64), target      :: pressure(2,NUM_GRID_CELLS)
    real(real64), target      :: pressure_c_ptrs(NUM_GRID_CELLS)
    real(real64), target      :: air_density(3,NUM_GRID_CELLS)
    real(real64), target      :: air_density_c_ptrs(NUM_GRID_CELLS)
    real(real64), target      :: concentrations(4,NUM_GRID_CELLS,NUM_SPECIES)
    real(real64), target      :: concentrations_c_ptrs(NUM_GRID_CELLS,NUM_SPECIES)
    real(real64), target      :: initial_concentrations(4,NUM_GRID_CELLS,NUM_SPECIES)
    real(real64), target      :: user_defined_reaction_rates(3,NUM_GRID_CELLS,NUM_USER_DEFINED_REACTION_RATES)
    real(real64), target      :: user_defined_reaction_rates_c_ptrs(NUM_GRID_CELLS,NUM_USER_DEFINED_REACTION_RATES)
    type(string_t)            :: solver_state
    type(solver_stats_t)      :: solver_stats
    integer                   :: solver_type
    type(error_t)             :: error
    real(real64), parameter   :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1
    integer                   :: A_index, B_index, C_index, D_index, E_index, F_index
    integer                   :: R1_index, R2_index
    real(real64)              :: initial_A, initial_C, initial_D, initial_F
    real(real64)              :: k1, k2, k3, k4
    real(real64)              :: A, B, C, D, E, F
    integer                   :: i_cell
    real                      :: temp
    type(ArrheniusReaction)   :: r1, r2

    A_index = micm%species_ordering%index( "A", error )
    ASSERT( error%is_success() )
    B_index = micm%species_ordering%index( "B", error )
    ASSERT( error%is_success() )
    C_index = micm%species_ordering%index( "C", error )
    ASSERT( error%is_success() )
    D_index = micm%species_ordering%index( "D", error )
    ASSERT( error%is_success() )
    E_index = micm%species_ordering%index( "E", error )
    ASSERT( error%is_success() )
    F_index = micm%species_ordering%index( "F", error )
    ASSERT( error%is_success() )

    R1_index = micm%user_defined_reaction_rates%index( "USER.reaction 1", error )
    ASSERT( error%is_success() )
    R2_index = micm%user_defined_reaction_rates%index( "USER.reaction 2", error )
    ASSERT( error%is_success() )

    temperature(:,:) = 1.0e300_real64
    pressure(:,:) = 1.0e300_real64
    air_density(:,:) = 1.0e300_real64
    concentrations(:,:,:) = 1.0e300_real64
    user_defined_reaction_rates(:,:,:) = 1.0e300_real64
    do i_cell = 1, NUM_GRID_CELLS
      call random_number( temp )
      temperature(2,i_cell) = 265.0 + temp * 20.0
      call random_number( temp )
      pressure(2,i_cell) = 100753.3 + temp * 1000.0
      air_density(2,i_cell) = pressure(2,i_cell) / ( GAS_CONSTANT * temperature(2,i_cell) )
      call random_number( temp )
      concentrations(2,i_cell,A_index) = 0.7 + temp * 0.1
      concentrations(2,i_cell,B_index) = 0.0
      call random_number( temp )
      concentrations(2,i_cell,C_index) = 0.35 + temp * 0.1
      call random_number( temp )
      concentrations(2,i_cell,D_index) = 0.75 + temp * 0.1
      concentrations(2,i_cell,E_index) = 0.0
      call random_number( temp )
      concentrations(2,i_cell,F_index) = 0.05 + temp * 0.1
      call random_number( temp )
      user_defined_reaction_rates(2,i_cell,R1_index) = 0.0005 + temp * 0.0001
      call random_number( temp )
      user_defined_reaction_rates(2,i_cell,R2_index) = 0.0015 + temp * 0.0001
    end do
    initial_concentrations(:,:,:) = concentrations(:,:,:)
    concentrations_c_ptrs(:,:) = concentrations(2,:,:)
    user_defined_reaction_rates_c_ptrs(:,:) = user_defined_reaction_rates(2,:,:)
    temperature_c_ptrs(:) = temperature(2,:)
    pressure_c_ptrs(:) = pressure(2,:)
    air_density_c_ptrs(:) = air_density(2,:)

    ! solve by passing fortran arrays
    call micm%solve(time_step, temperature(2,:), pressure(2,:), air_density(2,:), &
                    concentrations(2,:,:), user_defined_reaction_rates(2,:,:), &
                    solver_state, solver_stats, error)
    ASSERT( error%is_success() )
    ASSERT_EQ(solver_state%get_char_array(), "Converged")

    ! solve by passing C pointers
    call micm%solve(time_step, c_loc(temperature_c_ptrs), c_loc(pressure_c_ptrs), &
                    c_loc(air_density_c_ptrs), c_loc(concentrations_c_ptrs), &
                    c_loc(user_defined_reaction_rates_c_ptrs), &
                    solver_state, solver_stats, error)
    ASSERT( error%is_success() )
    ASSERT_EQ(solver_state%get_char_array(), "Converged")

    ! check concentrations
    do i_cell = 1, NUM_GRID_CELLS
      ASSERT_EQ(concentrations(2,i_cell,A_index), concentrations_c_ptrs(i_cell,A_index))
      ASSERT_EQ(concentrations(2,i_cell,B_index), concentrations_c_ptrs(i_cell,B_index))
      ASSERT_EQ(concentrations(2,i_cell,C_index), concentrations_c_ptrs(i_cell,C_index))
      ASSERT_EQ(concentrations(2,i_cell,D_index), concentrations_c_ptrs(i_cell,D_index))
      ASSERT_EQ(concentrations(2,i_cell,E_index), concentrations_c_ptrs(i_cell,E_index))
      ASSERT_EQ(concentrations(2,i_cell,F_index), concentrations_c_ptrs(i_cell,F_index))
    end do

    r1%A_ = 0.004
    r1%C_ = 50.0
    r2%A_ = 0.012
    r2%B_ = -2.0
    r2%C_ = 75.0
    r2%D_ = 50.0
    r2%E_ = 1.0e-6

    do i_cell = 1, NUM_GRID_CELLS
      initial_A = initial_concentrations(2,i_cell,A_index)
      initial_C = initial_concentrations(2,i_cell,C_index)
      initial_D = initial_concentrations(2,i_cell,D_index)
      initial_F = initial_concentrations(2,i_cell,F_index)
      k1 = user_defined_reaction_rates(2,i_cell,R1_index)
      k2 = user_defined_reaction_rates(2,i_cell,R2_index)
      k3 = calculate_arrhenius( r1, temperature(2,i_cell), pressure(2,i_cell) )
      k4 = calculate_arrhenius( r2, temperature(2,i_cell), pressure(2,i_cell) )
      A = initial_A * exp( -k3 * time_step )
      B = initial_A * (k3 / (k4 - k3)) * (exp(-k3 * time_step) - exp(-k4 * time_step))
      C = initial_C + initial_A * (1.0 + (k3 * exp(-k4 * time_step) - k4 * exp(-k3 * time_step)) / (k4 - k3))
      D = initial_D * exp( -k1 * time_step )
      E = initial_D * (k1 / (k2 - k1)) * (exp(-k1 * time_step) - exp(-k2 * time_step))
      F = initial_F + initial_D * (1.0 + (k1 * exp(-k2 * time_step) - k2 * exp(-k1 * time_step)) / (k2 - k1))
      ASSERT_NEAR(concentrations(2,i_cell,A_index), A, test_accuracy)
      ASSERT_NEAR(concentrations(2,i_cell,B_index), B, test_accuracy)
      ASSERT_NEAR(concentrations(2,i_cell,C_index), C, test_accuracy)
      ASSERT_NEAR(concentrations(2,i_cell,D_index), D, test_accuracy)
      ASSERT_NEAR(concentrations(2,i_cell,E_index), E, test_accuracy)
      ASSERT_NEAR(concentrations(2,i_cell,F_index), F, test_accuracy)
    end do

  end subroutine test_vector_multiple_grid_cells

  subroutine test_standard_multiple_grid_cells(micm, NUM_GRID_CELLS, time_step, test_accuracy)

    type(micm_t), pointer, intent(inout) :: micm
    integer,               intent(in)    :: NUM_GRID_CELLS
    real(real64),          intent(in)    :: time_step
    real,                  intent(in)    :: test_accuracy

    integer, parameter        :: NUM_SPECIES = 6
    integer, parameter        :: NUM_USER_DEFINED_REACTION_RATES = 2
    real(real64), target      :: temperature(NUM_GRID_CELLS)
    real(real64), target      :: pressure(NUM_GRID_CELLS)
    real(real64), target      :: air_density(NUM_GRID_CELLS)
    real(real64), target      :: concentrations(NUM_SPECIES, NUM_GRID_CELLS)
    real(real64), target      :: concentrations_c_ptrs(NUM_SPECIES, NUM_GRID_CELLS)
    real(real64), target      :: initial_concentrations(NUM_SPECIES, NUM_GRID_CELLS)
    real(real64), target      :: user_defined_reaction_rates(NUM_USER_DEFINED_REACTION_RATES, NUM_GRID_CELLS)
    type(string_t)            :: solver_state
    type(solver_stats_t)      :: solver_stats
    integer(c_int)            :: solver_type
    type(error_t)             :: error
    real(real64), parameter   :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1
    integer                   :: A_index, B_index, C_index, D_index, E_index, F_index
    integer                   :: R1_index, R2_index
    real(real64)              :: initial_A, initial_C, initial_D, initial_F
    real(real64)              :: k1, k2, k3, k4
    real(real64)              :: A, B, C, D, E, F
    integer                   :: i_cell
    real                      :: temp
    type(ArrheniusReaction)   :: r1, r2

    A_index = micm%species_ordering%index( "A", error )
    ASSERT( error%is_success() )
    B_index = micm%species_ordering%index( "B", error )
    ASSERT( error%is_success() )
    C_index = micm%species_ordering%index( "C", error )
    ASSERT( error%is_success() )
    D_index = micm%species_ordering%index( "D", error )
    ASSERT( error%is_success() )
    E_index = micm%species_ordering%index( "E", error )
    ASSERT( error%is_success() )
    F_index = micm%species_ordering%index( "F", error )
    ASSERT( error%is_success() )

    R1_index = micm%user_defined_reaction_rates%index( "USER.reaction 1", error )
    ASSERT( error%is_success() )
    R2_index = micm%user_defined_reaction_rates%index( "USER.reaction 2", error )
    ASSERT( error%is_success() )

    do i_cell = 1, NUM_GRID_CELLS
      call random_number( temp )
      temperature(i_cell) = 265.0 + temp * 20.0
      call random_number( temp )
      pressure(i_cell) = 100753.3 + temp * 1000.0
      air_density(i_cell) = pressure(i_cell) / ( GAS_CONSTANT * temperature(i_cell) )
      call random_number( temp )
      concentrations(A_index, i_cell) = 0.7 + temp * 0.1
      concentrations(B_index, i_cell) = 0.0
      call random_number( temp )
      concentrations(C_index, i_cell) = 0.35 + temp * 0.1
      call random_number( temp )
      concentrations(D_index, i_cell) = 0.75 + temp * 0.1
      concentrations(E_index, i_cell) = 0.0
      call random_number( temp )
      concentrations(F_index, i_cell) = 0.05 + temp * 0.1
      call random_number( temp )
      user_defined_reaction_rates(R1_index, i_cell) = 0.0005 + temp * 0.0001
      call random_number( temp )
      user_defined_reaction_rates(R2_index, i_cell) = 0.0015 + temp * 0.0001
    end do
    initial_concentrations(:,:) = concentrations(:,:)
    concentrations_c_ptrs(:,:) = concentrations(:,:)

    ! solve by passing fortran arrays
    call micm%solve(time_step, temperature, pressure, air_density, concentrations, &
                    user_defined_reaction_rates, solver_state, solver_stats, error)
    ASSERT( error%is_success() )
    ASSERT_EQ(solver_state%get_char_array(), "Converged")

    ! solve by passing C pointers
    call micm%solve(time_step, c_loc(temperature), c_loc(pressure), c_loc(air_density), &
                    c_loc(concentrations_c_ptrs), c_loc(user_defined_reaction_rates), &
                    solver_state, solver_stats, error)
    ASSERT( error%is_success() )
    ASSERT_EQ(solver_state%get_char_array(), "Converged")

    ! check concentrations
    do i_cell = 1, NUM_GRID_CELLS
      ASSERT_EQ(concentrations(A_index, i_cell), concentrations_c_ptrs(A_index, i_cell))
      ASSERT_EQ(concentrations(B_index, i_cell), concentrations_c_ptrs(B_index, i_cell))
      ASSERT_EQ(concentrations(C_index, i_cell), concentrations_c_ptrs(C_index, i_cell))
      ASSERT_EQ(concentrations(D_index, i_cell), concentrations_c_ptrs(D_index, i_cell))
      ASSERT_EQ(concentrations(E_index, i_cell), concentrations_c_ptrs(E_index, i_cell))
      ASSERT_EQ(concentrations(F_index, i_cell), concentrations_c_ptrs(F_index, i_cell))
    end do

    r1%A_ = 0.004
    r1%C_ = 50.0
    r2%A_ = 0.012
    r2%B_ = -2.0
    r2%C_ = 75.0
    r2%D_ = 50.0
    r2%E_ = 1.0e-6

    do i_cell = 1, NUM_GRID_CELLS
      initial_A = initial_concentrations(A_index, i_cell)
      initial_C = initial_concentrations(C_index, i_cell)
      initial_D = initial_concentrations(D_index, i_cell)
      initial_F = initial_concentrations(F_index, i_cell)
      k1 = user_defined_reaction_rates(R1_index, i_cell)
      k2 = user_defined_reaction_rates(R2_index, i_cell)
      k3 = calculate_arrhenius( r1, temperature(i_cell), pressure(i_cell) )
      k4 = calculate_arrhenius( r2, temperature(i_cell), pressure(i_cell) )
      A = initial_A * exp( -k3 * time_step )
      B = initial_A * (k3 / (k4 - k3)) * (exp(-k3 * time_step) - exp(-k4 * time_step))
      C = initial_C + initial_A * (1.0 + (k3 * exp(-k4 * time_step) - k4 * exp(-k3 * time_step)) / (k4 - k3))
      D = initial_D * exp( -k1 * time_step )
      E = initial_D * (k1 / (k2 - k1)) * (exp(-k1 * time_step) - exp(-k2 * time_step))
      F = initial_F + initial_D * (1.0 + (k1 * exp(-k2 * time_step) - k2 * exp(-k1 * time_step)) / (k2 - k1))
      ASSERT_NEAR(concentrations(A_index, i_cell), A, test_accuracy)
      ASSERT_NEAR(concentrations(B_index, i_cell), B, test_accuracy)
      ASSERT_NEAR(concentrations(C_index, i_cell), C, test_accuracy)
      ASSERT_NEAR(concentrations(D_index, i_cell), D, test_accuracy)
      ASSERT_NEAR(concentrations(E_index, i_cell), E, test_accuracy)
      ASSERT_NEAR(concentrations(F_index, i_cell), F, test_accuracy)
    end do

  end subroutine test_standard_multiple_grid_cells

  subroutine test_multiple_grid_cell_vector_Rosenbrock()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 200
    real, parameter           :: test_accuracy = 5.0e-3

    num_grid_cells = MICM_VECTOR_MATRIX_SIZE
    micm => micm_t( "configs/analytical", Rosenbrock, num_grid_cells, error )
    ASSERT( error%is_success() )

    call test_vector_multiple_grid_cells( micm, num_grid_cells, time_step, test_accuracy )

    deallocate( micm )

  end subroutine test_multiple_grid_cell_vector_Rosenbrock

  subroutine test_multiple_grid_cell_standard_Rosenbrock()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 200
    real,         parameter :: test_accuracy = 5.0e-3

    num_grid_cells = 3
    micm => micm_t( "configs/analytical", RosenbrockStandardOrder, num_grid_cells, error )
    ASSERT( error%is_success() )

    call test_standard_multiple_grid_cells( micm, num_grid_cells, time_step, test_accuracy )

    deallocate( micm )

  end subroutine test_multiple_grid_cell_standard_Rosenbrock

  subroutine test_multiple_grid_cell_vector_BackwardEuler()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 10
    real,         parameter :: test_accuracy = 0.1

    num_grid_cells = MICM_VECTOR_MATRIX_SIZE
    micm => micm_t( "configs/analytical", BackwardEuler, num_grid_cells, error )
    ASSERT( error%is_success() )

    call test_vector_multiple_grid_cells( micm, num_grid_cells, time_step, test_accuracy )

    deallocate( micm )

  end subroutine test_multiple_grid_cell_vector_BackwardEuler

  subroutine test_multiple_grid_cell_standard_BackwardEuler()

    type(micm_t), pointer :: micm
    integer               :: num_grid_cells
    type(error_t)         :: error

    real(real64), parameter :: time_step = 10
    real,         parameter :: test_accuracy = 0.1

    num_grid_cells = 3
    micm => micm_t( "configs/analytical", BackwardEulerStandardOrder, num_grid_cells, error )
    ASSERT( error%is_success() )

    call test_standard_multiple_grid_cells( micm, num_grid_cells, time_step, test_accuracy )

    deallocate( micm )

  end subroutine test_multiple_grid_cell_standard_BackwardEuler

  subroutine test_api_v1_parser()

    type(string_t)                :: micm_version
    type(micm_t), pointer         :: micm
    real(real64)                  :: time_step
    real(real64)                  :: temperature(1)
    real(real64)                  :: pressure(1)
    real(real64)                  :: air_density(1)
    real(real64), dimension(4,1)  :: concentrations 
    real(real64), dimension(3,1)  :: user_defined_reaction_rates 
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
    micm => micm_t(config_path, solver_type, num_grid_cells, error)
    ASSERT( error%is_success() )

    O2_index = micm%species_ordering%index( "O2", error )
    ASSERT( error%is_success() )
    O_index = micm%species_ordering%index( "O", error )
    ASSERT( error%is_success() )
    O1D_index = micm%species_ordering%index( "O1D", error )
    ASSERT( error%is_success() )
    O3_index = micm%species_ordering%index( "O3", error )
    ASSERT( error%is_success() )

    jO2_index = micm%user_defined_reaction_rates%index( "PHOTO.jO2", error )
    ASSERT( error%is_success() )
    jO3a_index = micm%user_defined_reaction_rates%index( "PHOTO.jO3->O", error )
    ASSERT( error%is_success() )
    jO3b_index = micm%user_defined_reaction_rates%index( "PHOTO.jO3->O1D", error )
    ASSERT( error%is_success() )

    temperature(1) = 272.5
    pressure(1) = 101253.4
    air_density(:) = pressure(:) / ( GAS_CONSTANT * temperature(:) )

    concentrations(O2_index,1) = 0.75
    concentrations(O_index,1) = 0.0
    concentrations(O1D_index,1) = 0.0
    concentrations(O3_index,1) = 0.0000081
    user_defined_reaction_rates(jO2_index,1) = 2.7e-19
    user_defined_reaction_rates(jO3a_index,1) = 1.13e-9
    user_defined_reaction_rates(jO3b_index,1) = 5.8e-8
    
    micm_version = get_micm_version()
    print *, "[test micm fort api] MICM version ", micm_version%get_char_array()

    do i = 1, micm%species_ordering%size()
      print *, "Species Name:", micm%species_ordering%name(i), &
               ", Index:", micm%species_ordering%index(i)
    end do
    do i = 1, micm%user_defined_reaction_rates%size()
      print *, "User Defined Reaction Rate Name:", micm%user_defined_reaction_rates%name(i), &
               ", Index:", micm%user_defined_reaction_rates%index(i)
    end do

    write(*,*) "[test micm fort api] Initial concentrations", concentrations

    write(*,*) "[test micm fort api] Solving starts..."
    call micm%solve(time_step, temperature, pressure,  air_density, concentrations, &
        user_defined_reaction_rates, solver_state, solver_stats, error)
    ASSERT( error%is_success() )

    write(*,*) "[test micm fort api] After solving, concentrations: ", concentrations
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
    micm => micm_t( "configs/invalid", solver_type, num_grid_cells, error )
    ASSERT( error%is_error( MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED ) )
    ASSERT( .not. associated( micm ) )

    write(*,*) "[test micm fort api] Finished."

  end subroutine test_api_v1_parser

end program
