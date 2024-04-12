program test_micm_fort_api
  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic
  use micm_core, only: micm_t, mapping_t, set_error_handler_c
  use musica_util, only: assert

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )

  type(micm_t), pointer         :: micm
  real(c_double)                :: time_step
  real(c_double)                :: temperature
  real(c_double)                :: pressure
  integer(c_int)                :: num_concentrations, num_user_defined_reaction_rates
  real(c_double), dimension(5)  :: concentrations 
  real(c_double), dimension(3)  :: user_defined_reaction_rates 
  integer                       :: i
  character(len=256)            :: config_path
  type(mapping_t)               :: the_mapping
  character(len=:), allocatable :: string_value
  real(c_double)                :: double_value
  integer(c_int)                :: int_value
  logical(c_bool)               :: bool_value

  time_step = 200
  temperature = 272.5
  pressure = 101253.4
  num_concentrations = 5
  concentrations = (/ 0.75, 0.4, 0.8, 0.01, 0.02 /)
  config_path = "configs/chapman"
  num_user_defined_reaction_rates = 3
  user_defined_reaction_rates = (/ 0.1, 0.2, 0.3 /)


  write(*,*) "[test micm fort api] Creating MICM solver..."
  micm => micm_t(config_path)

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
                  num_user_defined_reaction_rates, user_defined_reaction_rates)

  write(*,*) "[test micm fort api] After solving, concentrations", concentrations

  string_value = micm%get_species_property_string( "O3", "__long name" )
  ASSERT_EQ( string_value, "ozone" )
  double_value = micm%get_species_property_double( "O3", "molecular weight [kg mol-1]" )
  ASSERT_EQ( double_value, 0.048_c_double )
  int_value = micm%get_species_property_int( "O3", "__atoms" )
  ASSERT_EQ( int_value, 3_c_int )
  bool_value = micm%get_species_property_bool( "O3", "__do advect" )
  ASSERT( logical( bool_value ) )

  call set_error_handler_c( c_funloc( missing_string_property_error_handler ) )
  string_value = micm%get_species_property_string( "O3", "missing property" )
  ASSERT( string_value == "" )
  call set_error_handler_c( c_funloc( missing_double_property_error_handler ) )
  double_value = micm%get_species_property_double( "O3", "missing property" )
  ASSERT( isnan( double_value ) )
  call set_error_handler_c( c_funloc( missing_int_property_error_handler ) )
  int_value = micm%get_species_property_int( "O3", "missing property" )
  ASSERT_EQ( int_value, 0_c_int )
  call set_error_handler_c( c_funloc( missing_bool_property_error_handler ) )
  bool_value = micm%get_species_property_bool( "O3", "missing property" )
  ASSERT( .not. logical( bool_value ) )
  deallocate( micm )
  call set_error_handler_c( c_funloc( bad_configuration_error_handler ) )
  micm => micm_t( "configs/invalid" )
  ASSERT( .not. associated( micm ) )

  write(*,*) "[test micm fort api] Finished."

contains

  subroutine bad_configuration_error_handler( code, message ) bind(c)
    use, intrinsic :: iso_c_binding
    integer(c_int), value, intent(in) :: code
    character(len=1, kind=c_char), intent(in) :: message(*)
    ASSERT_EQ( code, 909039518_c_int )
  end subroutine bad_configuration_error_handler

  subroutine missing_string_property_error_handler( code, message ) bind(c)
    use, intrinsic :: iso_c_binding
    integer(c_int), value, intent(in) :: code
    character(len=1, kind=c_char), intent(in) :: message(*)
    ASSERT_EQ( code, 740788148_c_int )
  end subroutine missing_string_property_error_handler

  subroutine missing_double_property_error_handler( code, message ) bind(c)
    use, intrinsic :: iso_c_binding
    integer(c_int), value, intent(in) :: code
    character(len=1, kind=c_char), intent(in) :: message(*)
    ASSERT_EQ( code, 170573343_c_int )
  end subroutine missing_double_property_error_handler

  subroutine missing_int_property_error_handler( code, message ) bind(c)
    use, intrinsic :: iso_c_binding
    integer(c_int), value, intent(in) :: code
    character(len=1, kind=c_char), intent(in) :: message(*)
    ASSERT_EQ( code, 347900088_c_int )
  end subroutine missing_int_property_error_handler

  subroutine missing_bool_property_error_handler( code, message ) bind(c)
    use, intrinsic :: iso_c_binding
    integer(c_int), value, intent(in) :: code
    character(len=1, kind=c_char), intent(in) :: message(*)
    ASSERT_EQ( code, 509433912_c_int )
  end subroutine missing_bool_property_error_handler

end program
