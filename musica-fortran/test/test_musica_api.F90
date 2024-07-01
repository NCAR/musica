program test
    use iso_c_binding
    use micm_core

    implicit none
    
    type(micm_t)                   :: micm
    real(c_double)                 :: temperature      
    real(c_double)                 :: pressure
    real(c_double)                 :: time_step
    integer                        :: num_concentrations
    real(c_double), dimension(5)  :: concentrations
    integer                        :: errcode

    temperature = 10d0
    pressure = 20d0
    time_step = 1d0
    num_concentrations = 5
    concentrations = (/ 0.75, 0.4, 0.8, 0.01, 0.02 /)

    write(*,*) "  * [Fortran] Creating MICM..."
    micm = micm_t("chapman")

    write(*,*) "  * [Fortran] Creating solver..."
    errcode = micm%create_solver()

    if (errcode == 0) then
        write(*,*) "  * [Fortran] Initial temp", temperature
        write(*,*) "  * [Fortran] Initial pressure", pressure
        write(*,*) "  * [Fortran] Initial time_step", time_step
        write(*,*) "  * [Fortran] Initial number of concentrations", num_concentrations
        write(*,*) "  * [Fortran] Initial concentrations", concentrations

        write(*,*) "  * [Fortran] Solving starts..."
        call micm%solve(temperature, pressure, time_step, num_concentrations, concentrations)

        write(*,*) "  * [Fortran] After solving, concentrations", concentrations
    else
        write(*,*) "  * [Fortran] Failed in creating solver"
        stop 3
    endif

end program