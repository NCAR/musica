program test_micm_api
  use, intrinsic :: iso_c_binding
  use micm_core, only: set_error_handler_c
  use musica_util, only: assert

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )

  call set_error_handler_c( c_funloc( bad_configuration_error_handler ) )
  call test_micm_fort_api_invalid()

contains

  subroutine test_micm_fort_api_invalid()
    use micm_core, only: micm_t

    implicit none

    type(micm_t), pointer         :: micm
    character(len=7)              :: config_path

    config_path = "invalid_config"

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path)

  end subroutine

  subroutine bad_configuration_error_handler( code, message ) bind(c)
    use, intrinsic :: iso_c_binding
    integer(c_int), value, intent(in) :: code
    character(len=1, kind=c_char), intent(in) :: message(*)
    ASSERT_EQ( code, 909039518_c_int )
  end subroutine bad_configuration_error_handler

end program
