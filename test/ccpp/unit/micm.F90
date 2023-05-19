program micm_ccpp_api_test

  use micm

  implicit none

  call test_api()

contains

  subroutine test_api()
    call micm_init()
  end subroutine test_api

end program micm_ccpp_api_test