interface   
    function create_micm_c(config_path) bind(C, name="create_micm")
        use iso_c_binding
        implicit none
        type(c_ptr) :: create_micm_c
        character(len=1, kind=C_CHAR), intent(in) :: config_path(*)
    end function

    subroutine delete_micm_c(micm) bind(C, name="delete_micm")
        use iso_c_binding
        implicit none
        type(c_ptr), value :: micm
    end subroutine
    
    function micm_create_solver_c(micm) bind(C, name="micm_create_solver")
        use iso_c_binding
        implicit none
        integer(c_int) :: micm_create_solver_c  ! TODO(jiwon) return value?
        type(c_ptr), intent(in), value :: micm
    end function

    subroutine micm_solve_c(micm, temperature, pressure, time_step, concentrations, num_concentrations) bind(C, name="micm_solve")
        use iso_c_binding
        implicit none
        type(c_ptr), intent(in), value :: micm
        real(c_double), value :: temperature   
        real(c_double), value :: pressure
        real(c_double), value :: time_step
        real(c_double), dimension(*), intent(inout) :: concentrations
        integer(c_size_t), value, intent(in) :: num_concentrations
    end subroutine
end interface
