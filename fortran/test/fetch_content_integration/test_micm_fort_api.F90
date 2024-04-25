program test_micm_fort_api
  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic
  use micm_core, only: micm_t, mapping_t
  use musica_util, only: assert, error_t_c

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )

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
  type(error_t_c)               :: error

  time_step = 200
  temperature = 272.5
  pressure = 101253.4
  num_concentrations = 5
  concentrations = (/ 0.75, 0.4, 0.8, 0.01, 0.02 /)
  config_path = "configs/chapman"
  num_user_defined_reaction_rates = 3
  user_defined_reaction_rates = (/ 0.1, 0.2, 0.3 /)


  write(*,*) "[test micm fort api] Creating MICM solver..."
  micm => micm_t(config_path, error)
  ASSERT_EQ( error%code_, 0_c_int )

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
  ASSERT_EQ( error%code_, 0_c_int )

  write(*,*) "[test micm fort api] After solving, concentrations", concentrations

  string_value = micm%get_species_property_string( "O3", "__long name", error )
  ASSERT_EQ( error%code_, 0_c_int )
  ASSERT_EQ( string_value, "ozone" )
  double_value = micm%get_species_property_double( "O3", "molecular weight [kg mol-1]", error )
  ASSERT_EQ( error%code_, 0_c_int )
  ASSERT_EQ( double_value, 0.048_c_double )
  int_value = micm%get_species_property_int( "O3", "__atoms", error )
  ASSERT_EQ( error%code_, 0_c_int )
  ASSERT_EQ( int_value, 3_c_int )
  bool_value = micm%get_species_property_bool( "O3", "__do advect", error )
  ASSERT_EQ( error%code_, 0_c_int )
  ASSERT( logical( bool_value ) )

  string_value = micm%get_species_property_string( "O3", "missing property", error )
  ASSERT_NE( error%code_, 0_c_int )
  double_value = micm%get_species_property_double( "O3", "missing property", error )
  ASSERT_NE( error%code_, 0_c_int )
  int_value = micm%get_species_property_int( "O3", "missing property", error )
  ASSERT_NE( error%code_, 0_c_int )
  bool_value = micm%get_species_property_bool( "O3", "missing property", error )
  ASSERT_NE( error%code_, 0_c_int )
  deallocate( micm )
  micm => micm_t( "configs/invalid", error )
  ASSERT_NE( error%code_, 0_c_int )
  ASSERT( .not. associated( micm ) )

  write(*,*) "[test micm fort api] Finished."

end program
