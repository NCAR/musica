module micm
    use iso_c_binding

    private
    public :: micm_t

    include "micm_c_def.F90"

    type micm_t
        private
        type(c_ptr) :: ptr
    contains
        procedure :: delete => delete_micm
        procedure :: create_solver => micm_create_solver
        procedure :: solve => micm_solve
    end type

    interface micm_t
        procedure create_micm
    end interface

contains
    function create_micm(config_path)
        implicit none
        type(micm_t) :: create_micm
        character(len=*), intent(in) :: config_path
        character(len=1, kind=C_CHAR) :: c_config_path(len_trim(config_path) + 1)
        integer :: N, i

        ! Converting Fortran string to C string
        N = len_trim(config_path)
        do i = 1, N
            c_config_path(i) = config_path(i:i)
        end do
        c_config_path(N + 1) = C_NULL_CHAR

        create_micm%ptr = create_micm_c(c_config_path)
    end function

    subroutine delete_micm(this)
        implicit none
        class(micm_t) :: this
        call delete_micm_c(this%ptr)
    end subroutine

    integer function micm_create_solver(this)
        implicit none
        class(micm_t), intent(in) :: this
        micm_create_solver = micm_create_solver_c(this%ptr)
    end function

    subroutine micm_solve(this, temperature, pressure, time_step, concentrations, num_concentrations)
        implicit none
        class(micm_t), intent(in) :: this
        real(c_double), intent(in) :: temperature
        real(c_double), intent(in) :: pressure
        real(c_double), intent(in) :: time_step
        real(c_double), dimension(*), intent(inout) :: concentrations
        integer(c_size_t), intent(in) :: num_concentrations
        call micm_solve_c(this%ptr, temperature, pressure, time_step, concentrations, num_concentrations)
    end subroutine
    
end module