module micm_core

  use iso_c_binding
  implicit none

  public :: micm_t
  private

  interface
    function create_micm_c() bind(C, name="create_micm")
      use iso_c_binding
      implicit none
      type(c_ptr)  :: create_micm_c
    end function

    subroutine delete_micm_c(micm_t) bind(C, name="delete_micm")
      use iso_c_binding
      implicit none
      type(c_ptr), value  :: micm_t
    end subroutine
    
    function micm_create_solver_c(micm_t, config_path) bind(C, name="micm_create_solver")
      use iso_c_binding
      implicit none
      type(c_ptr), intent(in), value             :: micm_t
      character(len=1, kind=c_char), intent(in)  :: config_path(*)
      integer(c_int)                             :: micm_create_solver_c
    end function

    subroutine micm_solve_c(micm_t, time_step, temperature, pressure, num_concentrations, concentrations) &
                            bind(C, name="micm_solve")
      use iso_c_binding
      implicit none
      type(c_ptr), intent(in), value     :: micm_t
      real(c_double), intent(in), value  :: time_step
      real(c_double), intent(in), value  :: temperature
      real(c_double), intent(in), value  :: pressure
      integer(c_int), intent(in), value  :: num_concentrations
      real(c_double), intent(inout)      :: concentrations(*)
    end subroutine
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
    use iso_c_binding
    implicit none
    ! Constructor of micm objects
    type(micm_t)  :: create_micm
    create_micm%ptr = create_micm_c()
  end function create_micm

  integer function micm_create_solver(this, config_path)
    use iso_c_binding
    implicit none
    class(micm_t), intent(in)      :: this
    character(len=*), intent(in)   :: config_path
    character(len=1, kind=c_char)  :: c_config_path(len_trim(config_path)+1)
    integer                        :: n, i

    n = len_trim(config_path)
    do i = 1, n
      c_config_path(i) = config_path(i:i)
    end do
    c_config_path(n+1) = C_NULL_CHAR

    micm_create_solver = micm_create_solver_c(this%ptr, c_config_path)
  end function micm_create_solver

  subroutine micm_solve(this, time_step, temperature, pressure, num_concentrations, concentrations)
    use iso_c_binding
    implicit none
    class(micm_t), intent(in)      :: this
    real(c_double), intent(in)     :: time_step
    real(c_double), intent(in)     :: temperature
    real(c_double), intent(in)     :: pressure
    integer(c_int), intent(in)     :: num_concentrations
    real(c_double), intent(inout)  :: concentrations(*)
    call micm_solve_c(this%ptr, time_step, temperature, pressure, num_concentrations, concentrations)
  end subroutine micm_solve

  subroutine finalize(this)
    use iso_c_binding
    implicit none
    type(micm_t), intent(inout)  :: this
    call delete_micm_c(this%ptr)
  end subroutine finalize

end module micm_core