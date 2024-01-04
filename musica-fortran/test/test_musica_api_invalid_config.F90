program test
    use iso_c_binding
    use micm_core

    implicit none
    
    type(micm_t)                   :: micm
    real(c_double)                 :: temperature      
    real(c_double)                 :: pressure
    real(c_double)                 :: time_step
    integer                        :: num_concentrations
    real(c_double), dimension(10)  :: concentrations
    integer                        :: errcode

    temperature = 10d0
    pressure = 20d0
    time_step = 1d0
    num_concentrations = 10
    concentrations = (/ 1d0, 2d0, 3d0, 4d0, 5d0, 6d0, 7d0, 8d0, 9d0, 10d0 /)

    write(*,*) "  * [Fortran] Creating MICM..."
    micm = micm_t("invalid_config")

    write(*,*) "  * [Fortran] Creating solver..."
    errcode = micm%create_solver()

    if (errcode == 1) then
        write(*,*) "  * [Fortran] Failed in creating solver"
        write(*,*) "  * [Fortran] Expected failure. Error code: ", errcode
        stop 0
    else
        write(*,*) "  * [Fortran] Unexpected error code: ", errcode
        stop 3
    endif

end program