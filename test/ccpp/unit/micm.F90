program micm_ccpp_api_test

   use micm

   implicit none

   call test_api()

contains

   subroutine test_api()
      character(len=512) :: errmsg
      integer :: errflg

      character(len=*), parameter :: filepath = "somepath.json"

      real(kind=8), dimension(:), allocatable :: state
      integer(kind=8) :: state_size = 5
      integer(kind=8) :: time_step = 1

      allocate(state(state_size))
      state = 1

      call micm_init(filepath, errmsg, errflg)
      call micm_run(state, state_size, time_step, errmsg, errflg)
   end subroutine test_api

end program micm_ccpp_api_test
