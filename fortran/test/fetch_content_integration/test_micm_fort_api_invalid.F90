program test_micm_api
  use, intrinsic :: iso_c_binding
  use musica_util, only: assert

#include "micm/util/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )

  implicit none

  call test_micm_fort_api_invalid()

contains

  subroutine test_micm_fort_api_invalid()
    use musica_util, only: error_t
    use musica_micm, only: micm_t

    implicit none

    type(micm_t), pointer         :: micm
    character(len=7)              :: config_path
    type(error_t)                 :: error

    config_path = "invalid_config"

    write(*,*) "[test micm fort api] Creating MICM solver..."
    micm => micm_t(config_path, error)
    ASSERT( error%is_error( MICM_ERROR_CATEGORY_CONFIGURATION, \
                      MICM_CONFIGURATION_ERROR_CODE_INVALID_FILE_PATH ) )

  end subroutine

end program
