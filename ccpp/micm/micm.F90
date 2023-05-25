module micm
   use iso_c_binding

   implicit none

   procedure(solver), pointer :: fsolver

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

   !> \section arg_table_micm_init Argument Table
   !! \htmlinclude micm_init.html
   subroutine micm_init(filepath, errmsg, errflg)
      ! Arguments
      character(len=512), intent(out) :: errmsg
      integer, intent(out) :: errflg
      character(len=*), intent(in) :: filepath

      ! Local variables
      type(c_funptr) :: csolver_func_pointer

      ! Convert Fortran character array to C character array
      character(len=len(filepath)+1, kind=c_char) :: c_filepath
      c_filepath = transfer(filepath, c_filepath)

      csolver_func_pointer = get_solver(c_filepath)
      call c_f_procpointer(csolver_func_pointer, fsolver)
   end subroutine micm_init

   !> \section arg_table_micm_timestep_init Argument Table
   !! \htmlinclude micm_timestep_init.html
   subroutine micm_timestep_init(errmsg, errflg)
      character(len=512),intent(out):: errmsg
      integer,           intent(out):: errflg

   end subroutine micm_timestep_init

   !> \section arg_table_micm_run Argument Table
   !! \htmlinclude micm_run.html
   subroutine micm_run(state, state_size, time_step, errmsg, errflg)
      character(len=512),intent(out):: errmsg
      integer,           intent(out):: errflg

      real(c_double), dimension(:), allocatable :: state
      integer(c_int64_t) :: state_size 
      integer(c_int64_t) :: time_step

      call fsolver(state, state_size, time_step)
   end subroutine micm_run

   !> \section arg_table_micm_timestep_final Argument Table
   !! \htmlinclude micm_timestep_final.html
   subroutine micm_timestep_final(errmsg, errflg)
      character(len=512),intent(out):: errmsg
      integer,           intent(out):: errflg

   end subroutine micm_timestep_final

   !> \section arg_table_micm_final Argument Table
   !! \htmlinclude micm_final.html
   subroutine micm_final(errmsg, errflg)
      character(len=512),intent(out):: errmsg
      integer,           intent(out):: errflg

   end subroutine micm_final

end module micm
