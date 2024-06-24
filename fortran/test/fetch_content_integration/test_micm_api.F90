! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program test_micm_api

  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic
  use musica_micm, only: micm_t, solver_stats_t, get_micm_version
  use musica_util, only: assert, error_t, mapping_t, string_t

#include "micm/util/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )

  implicit none

  call test_api()

contains

  subroutine test_api()

    type(string_t)                :: micm_version
    type(micm_t), pointer         :: micm
    real(c_double)                :: time_step
    real(c_double)                :: temperature
    real(c_double)                :: pressure
    real(c_double)                :: air_density
    integer(c_int)                :: num_concentrations, num_user_defined_reaction_rates
    real(c_double), dimension(5)  :: concentrations 
    real(c_double), dimension(3)  :: user_defined_reaction_rates 
    character(len=256)            :: config_path
    character(len=:), allocatable :: string_value
    real(c_double)                :: double_value
    integer(c_int)                :: int_value
    logical(c_bool)               :: bool_value
    type(string_t)                :: solver_state
    type(solver_stats_t)          :: solver_stats
    type(error_t)                 :: error
    real(c_double), parameter     :: GAS_CONSTANT = 8.31446261815324_c_double ! J mol-1 K-1
    integer                       :: i
    
    time_step = 200
    temperature = 272.5
    pressure = 101253.4
    air_density = pressure / ( GAS_CONSTANT * temperature )
    num_concentrations = 5
    concentrations = (/ 0.75, 0.4, 0.8, 0.01, 0.02 /)
    config_path = "configs/chapman"
    num_user_defined_reaction_rates = 3
    user_defined_reaction_rates = (/ 0.1, 0.2, 0.3 /)

    micm_version = get_micm_version()
    print *, "[test micm fort api] MICM version ", micm_version%get_char_array()

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, error)
    ASSERT( error%is_success() )

    do i = 1, size( micm%species_ordering )
      associate(the_mapping => micm%species_ordering(i))
      print *, "Species Name:", the_mapping%name(), ", Index:", the_mapping%index()
      end associate
    end do
    do i = 1, size( micm%user_defined_reaction_rates )
      associate(the_mapping => micm%user_defined_reaction_rates(i))
      print *, "User Defined Reaction Rate Name:", the_mapping%name(), ", Index:", the_mapping%index()
      end associate
    end do

    write(*,*) "[test micm fort api] Initial concentrations", concentrations

    write(*,*) "[test micm fort api] Solving starts..."
    call micm%solve(time_step, temperature, pressure,  air_density, num_concentrations, concentrations, &
        num_user_defined_reaction_rates, user_defined_reaction_rates, solver_state, solver_stats, error)
    ASSERT( error%is_success() )

    write(*,*) "[test micm fort api] After solving, concentrations: ", concentrations
    write(*,*) "[test micm fort api] Solver state: ", solver_state%get_char_array()
    write(*,*) "[test micm fort api] Function calls: ", solver_stats%function_calls()
    write(*,*) "[test micm fort api] Jacobian updates: ", solver_stats%jacobian_updates()
    write(*,*) "[test micm fort api] Number of steps: ", solver_stats%number_of_steps()
    write(*,*) "[test micm fort api] Accepted: ", solver_stats%accepted()
    write(*,*) "[test micm fort api] Rejected: ", solver_stats%rejected()
    write(*,*) "[test micm fort api] Decompositions: ", solver_stats%decompositions()
    write(*,*) "[test micm fort api] Solves: ", solver_stats%solves()
    write(*,*) "[test micm fort api] Singular: ", solver_stats%singular()
    write(*,*) "[test micm fort api] Final time: ", solver_stats%final_time()

    string_value = micm%get_species_property_string( "O3", "__long name", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( string_value, "ozone" )
    deallocate( string_value )
    double_value = micm%get_species_property_double( "O3", "molecular weight [kg mol-1]", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( double_value, 0.048_c_double )
    int_value = micm%get_species_property_int( "O3", "__atoms", error )
    ASSERT( error%is_success() )
    ASSERT_EQ( int_value, 3_c_int )
    bool_value = micm%get_species_property_bool( "O3", "__do advect", error )
    ASSERT( error%is_success() )
    ASSERT( logical( bool_value ) )

    string_value = micm%get_species_property_string( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    double_value = micm%get_species_property_double( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    int_value = micm%get_species_property_int( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    bool_value = micm%get_species_property_bool( "O3", "missing property", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    deallocate( micm )
    micm => micm_t( "configs/invalid", error )
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_CONFIGURATION, \
                      MICM_CONFIGURATION_ERROR_CODE_INVALID_FILE_PATH ) )
    ASSERT( .not. associated( micm ) )

    write(*,*) "[test micm fort api] Finished."

  end subroutine test_api

end program
