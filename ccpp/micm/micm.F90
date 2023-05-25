!< \section arg_table_micm
!! \htmlinclude micm.html
module micm
   use iso_c_binding

   implicit none

   private
   public :: micm_init, micm_run

   procedure(solver), pointer :: fsolver

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

contains

   !> \section arg_table_micm_init Argument Table
   !! \htmlinclude micm_init.html
   subroutine micm_init(filepath, errmsg, errflg)
      ! Arguments
      character(len=512), intent(out)  :: errmsg
      integer, intent(out)             :: errflg
      character(len=*), intent(in)     :: filepath

      ! Local variables
      type(c_funptr)                   :: csolver_func_pointer
      ! Convert Fortran character array to C character array
      character(len=len(filepath)+1, kind=c_char) :: c_filepath

      errmsg = ''
      errflg = 0

      c_filepath = transfer(filepath, c_filepath)

      csolver_func_pointer = get_solver(c_filepath)
      call c_f_procpointer(csolver_func_pointer, fsolver)
   end subroutine micm_init

   !> \section arg_table_micm_run Argument Table
   !! \htmlinclude micm_run.html
   subroutine micm_run(state, state_size, time_step, errmsg, errflg)
      real(c_double), dimension(:), allocatable :: state
      integer(c_int64_t)                        :: state_size
      integer(c_int64_t)                        :: time_step
      character(len=512),intent(out)            :: errmsg
      integer,           intent(out)            :: errflg

      errmsg = ''
      errflg = 0

      call fsolver(state, state_size, time_step)

   end subroutine micm_run

end module micm
