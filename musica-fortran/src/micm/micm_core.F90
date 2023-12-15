module micm_core
    ! Top-level MICM interface

    use iso_c_binding
    
    implicit none

    private
    public :: micm_t

    interface   
        function create_micm_c(config_path) bind(C, name="create_micm")
            use iso_c_binding
            implicit none
            type(c_ptr)                                :: create_micm_c
            character(len=1, kind=C_CHAR), intent(in)  :: config_path(*)
        end function

        subroutine delete_micm_c(micm_t) bind(C, name="delete_micm")
            use iso_c_binding
            implicit none
            type(c_ptr), value  :: micm_t
        end subroutine
        
        function micm_create_solver_c(micm_t) bind(C, name="micm_create_solver")
            use iso_c_binding
            implicit none
            integer(c_int)                  :: micm_create_solver_c
            type(c_ptr), intent(in), value  :: micm_t
        end function

        subroutine micm_solve_c(micm_t, temperature, pressure, time_step, num_concentrations, concentrations) bind(C, name="micm_solve")
            use iso_c_binding
            implicit none
            type(c_ptr), intent(in), value               :: micm_t
            real(c_double), value                        :: temperature   
            real(c_double), value                        :: pressure
            real(c_double), value                        :: time_step
            integer, value, intent(in)                   :: num_concentrations
            real(c_double), dimension(*), intent(inout)  :: concentrations
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

    function create_micm(config_path)
        ! Constructor of micm objects

        type(micm_t)                   :: create_micm
        character(len=*), intent(in)   :: config_path
        character(len=1, kind=C_CHAR)  :: c_config_path(len_trim(config_path) + 1)
        integer                        :: N, i

        ! Convert Fortran string to C string
        N = len_trim(config_path)
        do i = 1, N
            c_config_path(i) = config_path(i:i)
        end do
        c_config_path(N + 1) = C_NULL_CHAR

        create_micm%ptr = create_micm_c(c_config_path)
        
        return
    end function create_micm

    subroutine finalize(this)
        type(micm_t), intent(inout)  :: this
        call delete_micm_c(this%ptr)
    end subroutine finalize

    integer function micm_create_solver(this)
        class(micm_t), intent(in)  :: this
        micm_create_solver = micm_create_solver_c(this%ptr)
    end function micm_create_solver

    subroutine micm_solve(this, temperature, pressure, time_step, num_concentrations, concentrations)
        class(micm_t), intent(in)                    :: this
        real(c_double), intent(in)                   :: temperature
        real(c_double), intent(in)                   :: pressure
        real(c_double), intent(in)                   :: time_step
        integer, intent(in)                          :: num_concentrations
        real(c_double), dimension(*), intent(inout)  :: concentrations
        call micm_solve_c(this%ptr, temperature, pressure, time_step, num_concentrations, concentrations)
    end subroutine micm_solve
    
end module micm_core