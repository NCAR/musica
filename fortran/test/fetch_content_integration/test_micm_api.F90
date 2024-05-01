program combined_micm_tests
  use iso_c_binding
  use musica_micm_core, only: micm_t, mapping_t
  use musica_util, only: assert, error_t_c, is_error, is_success

  implicit none

#include "micm/util/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )

  ! Declarations
  type(micm_t), pointer         :: micm
  real(c_double)                :: time_step
  real(c_double)                :: temperature
  real(c_double)                :: pressure
  integer(c_int)                :: num_concentrations, num_user_defined_reaction_rates
  real(c_double), dimension(5)  :: concentrations 
  real(c_double), dimension(3)  :: user_defined_reaction_rates 
  integer                       :: errcode

  ! Main program
  time_step = 200
  temperature = 272.5
  pressure = 101253.4
  num_concentrations = 5
  concentrations = (/ 0.75, 0.4, 0.8, 0.01, 0.02 /)
  num_user_defined_reaction_rates = 3
  user_defined_reaction_rates = (/ 0.1, 0.2, 0.3 /)

  ! Call the valid test subroutine
  call test_micm_fort_api()

  ! Call the invalid test subroutine
  call test_micm_fort_api_invalid()

contains

  ! Valid MICM solver creation test
  subroutine test_micm_fort_api()
    type(mapping_t)               :: the_mapping
    character(len=:), allocatable :: string_value
    real(c_double)                :: double_value
    integer(c_int)                :: int_value
    logical(c_bool)               :: bool_value
    type(error_t_c)               :: error
    character(len=256)            :: config_path
    integer                       :: i

    config_path = "configs/chapman"

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, error)
    ASSERT( is_success( error ) )

    do i = 1, micm%species_ordering_length
      the_mapping = micm%species_ordering(i)
      print *, "Species Name:", the_mapping%name(:the_mapping%string_length), ", Index:", the_mapping%index
    end do
    do i = 1, micm%user_defined_reaction_rates_length
      the_mapping = micm%user_defined_reaction_rates(i)
      print *, "User Defined Reaction Rate Name:", the_mapping%name(:the_mapping%string_length), ", Index:", the_mapping%index
    end do

    write(*,*) "[test micm fort api] Initial concentrations", concentrations

    write(*,*) "[test micm fort api] Solving starts..."
    call micm%solve(time_step, temperature, pressure, num_concentrations, concentrations, &
                    num_user_defined_reaction_rates, user_defined_reaction_rates, error)
    ASSERT( is_success( error ) )

    write(*,*) "[test micm fort api] After solving, concentrations", concentrations

    string_value = micm%get_species_property_string( "O3", "__long name", error )
    ASSERT( is_success( error ) )
    ASSERT_EQ( string_value, "ozone" )
    double_value = micm%get_species_property_double( "O3", "molecular weight [kg mol-1]", error )
    ASSERT( is_success( error ) )
    ASSERT_EQ( double_value, 0.048_c_double )
    int_value = micm%get_species_property_int( "O3", "__atoms", error )
    ASSERT( is_success( error ) )
    ASSERT_EQ( int_value, 3_c_int )
    bool_value = micm%get_species_property_bool( "O3", "__do advect", error )
    ASSERT( is_success( error ) )
    ASSERT( logical( bool_value ) )

    string_value = micm%get_species_property_string( "O3", "missing property", error )
    ASSERT( is_error( error, MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    double_value = micm%get_species_property_double( "O3", "missing property", error )
    ASSERT( is_error( error, MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    int_value = micm%get_species_property_int( "O3", "missing property", error )
    ASSERT( is_error( error, MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    bool_value = micm%get_species_property_bool( "O3", "missing property", error )
    ASSERT( is_error( error, MICM_ERROR_CATEGORY_SPECIES, \
                      MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND ) )
    deallocate( micm )
    micm => micm_t( "configs/invalid", error )
    ASSERT( is_error( error, MICM_ERROR_CATEGORY_CONFIGURATION, \
                      MICM_CONFIGURATION_ERROR_CODE_INVALID_FILE_PATH ) )
    ASSERT( .not. associated( micm ) )

    write(*,*) "[test micm fort api] Finished."

  end subroutine test_micm_fort_api

  ! Invalid MICM solver creation test
  subroutine test_micm_fort_api_invalid()
    use musica_util, only: error_t_c
    use musica_micm_core, only: micm_t

    implicit none

    type(micm_t), pointer         :: micm
    character(len=7)              :: config_path
    type(error_t_c)               :: error

    config_path = "invalid_config"

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, error)
    ASSERT( is_error( error, MICM_ERROR_CATEGORY_CONFIGURATION, \
                      MICM_CONFIGURATION_ERROR_CODE_INVALID_FILE_PATH ) )

  end subroutine test_micm_fort_api_invalid

end program combined_micm_tests
