module micm
   use iso_c_binding

   implicit none

   interface
      type(c_funptr) function get_solver(filepath) bind(c)
         import :: c_char, c_funptr
         character(len=1, kind=c_char), dimension(*), intent(in) :: filepath
      end function get_solver
   end interface

   interface
      subroutine solver(state, state_size, time_step) bind(c)
         import :: c_ptr, c_double, c_int64_t
         real(c_double), dimension(*) :: state
         integer(c_int64_t), value :: state_size
         integer(c_int64_t), value :: time_step
      end subroutine
   end interface

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


   subroutine micm_timestep_init
   end subroutine micm_timestep_init

   subroutine micm_run
   end subroutine micm_run

   subroutine micm_timestep_final
   end subroutine micm_timestep_final

   subroutine micm_final
   end subroutine micm_final

end module micm
