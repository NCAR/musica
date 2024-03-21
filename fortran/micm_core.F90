module micm_core

   use iso_c_binding, only: c_ptr, c_char, c_int, c_double, c_null_char
   implicit none

   public :: micm_t
   private

   interface
      function create_micm_c(config_path, error_code) bind(C, name="create_micm")
         import c_ptr, c_int, c_char
         character(kind=c_char), intent(in) :: config_path(*)
         integer(kind=c_int), intent(out)   :: error_code
         type(c_ptr)                        :: create_micm_c
      end function create_micm_c

      subroutine delete_micm_c(micm) bind(C, name="delete_micm")
         import c_ptr
         type(c_ptr), intent(in) :: micm
      end subroutine delete_micm_c

      subroutine micm_solve_c(micm, time_step, temperature, pressure, num_concentrations, concentrations) bind(C, name="micm_solve")
         import c_ptr, c_double, c_int
         type(c_ptr), value, intent(in)         :: micm
         real(kind=c_double), value, intent(in) :: time_step
         real(kind=c_double), value, intent(in) :: temperature
         real(kind=c_double), value, intent(in) :: pressure
         integer(kind=c_int), value, intent(in) :: num_concentrations
         real(kind=c_double), intent(inout)     :: concentrations(num_concentrations)
      end subroutine micm_solve_c
   end interface

   type :: micm_t
      private
      type(c_ptr) :: ptr
   contains
      ! Solve the chemical system
      procedure :: solve
      ! Deallocate the micm instance
      final :: finalize
   end type micm_t

   interface micm_t
      procedure constructor
   end interface micm_t

contains

   function constructor(config_path, errcode)  result( this )
      type(micm_t), pointer         :: this
      character(len=*), intent(in)  :: config_path
      integer, intent(out)          :: errcode
      character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
      integer                       :: n, i

      allocate( this )

      n = len_trim(config_path)
      do i = 1, n
         c_config_path(i) = config_path(i:i)
      end do
      c_config_path(n+1) = c_null_char

      this%ptr = create_micm_c(c_config_path, errcode)

      if (errcode /= 0) then
         return
      end if
   end function constructor

   subroutine solve(this, time_step, temperature, pressure, num_concentrations, concentrations)
      class(micm_t)                 :: this
      real(c_double), intent(in)    :: time_step
      real(c_double), intent(in)    :: temperature
      real(c_double), intent(in)    :: pressure
      integer(c_int), intent(in)    :: num_concentrations
      real(c_double), intent(inout) :: concentrations(*)
      call micm_solve_c(this%ptr, time_step, temperature, pressure, num_concentrations, concentrations)
   end subroutine solve

   subroutine finalize(this)
      type(micm_t), intent(inout) :: this
      call delete_micm_c(this%ptr)
   end subroutine finalize

end module micm_core
