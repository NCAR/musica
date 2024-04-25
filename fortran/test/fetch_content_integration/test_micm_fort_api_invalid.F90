program test_micm_api
  use, intrinsic :: iso_c_binding
  use musica_util, only: assert

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )

  call test_micm_fort_api_invalid()

contains

  subroutine test_micm_fort_api_invalid()
    use musica_util, only: error_t_c
    use micm_core, only: micm_t

    implicit none

    type(micm_t), pointer         :: micm
    character(len=7)              :: config_path
    type(error_t_c)               :: error

    config_path = "invalid_config"

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, error)
    ASSERT_NE( error%code_, 0_c_int )

  end subroutine

end program
