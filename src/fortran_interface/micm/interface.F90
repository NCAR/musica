!< \section arg_table_micm
!! \htmlinclude micm.html

   interface

      type(c_funptr) function get_solver(filepath) bind(c)
         import :: c_char, c_funptr
         character(len=1, kind=c_char), dimension(*), intent(in) :: filepath
      end function get_solver

      subroutine solver(state, state_size, time_step) bind(c)
         import :: c_ptr, c_double, c_int64_t
         real(c_double), dimension(*) :: state
         integer(c_int64_t), value :: state_size
         integer(c_int64_t), value :: time_step
      end subroutine

   end interface