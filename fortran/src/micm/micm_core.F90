module micm_core

   use iso_c_binding, only: c_ptr, c_char, c_int, c_double, c_null_char
   implicit none

   public :: micm_t
   private

   interface

      subroutine create_micm_c(micm) bind(C, name="create_micm")
         import c_ptr
         type(c_ptr), intent(out) :: micm
      end subroutine create_micm_c

      subroutine delete_micm_c(micm) bind(C, name="delete_micm")
         import c_ptr
         type(c_ptr), intent(inout) :: micm
      end subroutine delete_micm_c

      function micm_create_solver_c(micm, config_path) result(res) bind(C, name="micm_create_solver")
         import c_ptr, c_char, c_int
         type(c_ptr) :: micm
         character(kind=c_char), intent(in) :: config_path(*)
         integer(kind=c_int) :: res
      end function micm_create_solver_c

      subroutine micm_solve_c(micm, time_step, temperature, pressure, num_concentrations, concentrations) bind(C, name="micm_solve")
         import c_ptr, c_double, c_int
         type(c_ptr), intent(inout) :: micm
         real(kind=c_double), intent(in) :: time_step
         real(kind=c_double), intent(in) :: temperature
         real(kind=c_double), intent(in) :: pressure
         integer(kind=c_int), intent(in) :: num_concentrations
         real(kind=c_double), intent(inout) :: concentrations(num_concentrations)
      end subroutine micm_solve_c
   end interface

   type :: micm_t
      private
      type(c_ptr) :: ptr
   contains
      ! Create a solver from configure file
      procedure :: create_solver => micm_create_solver
      ! Solve the chemical system
      procedure :: solve => micm_solve
      ! Deallocate the micm instance
      final :: finalize
   end type micm_t

   interface micm_t
      procedure create_micm
   end interface micm_t

contains

   function create_micm()
      type(micm_t)  :: create_micm
      call create_micm_c(create_micm%ptr)
   end function create_micm

   integer function micm_create_solver(this, config_path)
      class(micm_t)      :: this
      character(len=*), intent(in)   :: config_path
      character(len=1, kind=c_char)  :: c_config_path(len_trim(config_path)+1)
      integer                        :: n, i

      n = len_trim(config_path)
      do i = 1, n
         c_config_path(i) = config_path(i:i)
      end do
      c_config_path(n+1) = c_null_char

      micm_create_solver = micm_create_solver_c(this%ptr, c_config_path)
   end function micm_create_solver

   subroutine micm_solve(this, time_step, temperature, pressure, num_concentrations, concentrations)
      class(micm_t)      :: this
      real(c_double), intent(in)     :: time_step
      real(c_double), intent(in)     :: temperature
      real(c_double), intent(in)     :: pressure
      integer(c_int), intent(in)     :: num_concentrations
      real(c_double), intent(inout)  :: concentrations(*)
      call micm_solve_c(this%ptr, time_step, temperature, pressure, num_concentrations, concentrations)
   end subroutine micm_solve

   subroutine finalize(this)
      type(micm_t), intent(inout)  :: this
      call delete_micm_c(this%ptr)
   end subroutine finalize

end module micm_core
