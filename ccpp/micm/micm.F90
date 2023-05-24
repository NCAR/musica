module micm
   use iso_c_binding
   use micm_solver_interface, only: get_solver, solver

   implicit none

contains

   subroutine micm_init()
      type(c_funptr) :: csolver_func_pointer
      procedure(solver), pointer :: fsolver
      character(len=*, kind=c_char), parameter :: filepath = "hello.txt"

      real(c_double), dimension(:), allocatable :: state
      integer(c_int64_t) :: state_size = 5
      integer(c_int64_t) :: time_step = 1

      allocate(state(state_size))

      state = 1

      csolver_func_pointer = get_solver(filepath)
      call c_f_procpointer(csolver_func_pointer, fsolver)

      call fsolver(state, state_size, time_step)
   end subroutine micm_init

end module micm
