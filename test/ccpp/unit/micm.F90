program micm_ccpp_api_test

   use micm

   implicit none

   ! use ccpp_kinds, only:  kind_phys
   ! //TODO: figure out how to connect the test to ccpp to use their actual kind
   integer, parameter :: kind_phys = kind(8)

   call test_api()

contains

   subroutine test_api()
      character(len=512) :: errmsg
      integer :: errflg

      character(len=*), parameter :: filepath = "somepath.json"

      real(kind=kind_phys), dimension(:), allocatable :: state
      integer(kind=8) :: state_size = 5
      integer(kind=8) :: time_step = 1

      allocate(state(state_size))
      state = 1

      call micm_init(filepath, errmsg, errflg)
      call micm_run(state, state_size, time_step, errmsg, errflg)
   end subroutine test_api

end program micm_ccpp_api_test
