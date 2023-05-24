module micm_solver_interface
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

end module micm_solver_interface
