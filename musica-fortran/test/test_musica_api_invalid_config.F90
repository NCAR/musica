program test
    use iso_c_binding
    use micm_core

    implicit none
    
    type(micm_t) :: micm

    real(c_double) :: temperature      
    real(c_double) :: pressure
    real(c_double) :: time_step
    real(c_double), dimension(10) :: concentrations
    integer(c_size_t) :: num_concentrations
    integer :: errcode

    temperature = 10d0
    pressure = 20d0
    time_step = 1d0
    concentrations = (/ 1d0, 2d0, 3d0, 4d0, 5d0, 6d0, 7d0, 8d0, 9d0, 10d0 /)
    num_concentrations = 10

    write(*,*) "  * [Fortran] Creating MICM..."
    micm = micm_t("invalid_config")

    write(*,*) "  * [Fortran] Creating solver..."
    errcode = micm%create_solver()

    if (errcode == 1) then
        write(*,*) "  * [Fortran] Failed in creating solver"
    else
        write(*,*) "  * [Fortran] Initial temp", temperature
        write(*,*) "  * [Fortran] Initial pressure", pressure
        write(*,*) "  * [Fortran] Initial time_step", time_step
        write(*,*) "  * [Fortran] Initial concentrations", concentrations
        write(*,*) "  * [Fortran] Initial number of concentrations", num_concentrations

        write(*,*) "  * [Fortran] Solving starts..."
        call micm%solve(temperature, pressure, time_step, concentrations, num_concentrations)

        write(*,*) "  * [Fortran] After solving, concentrations", concentrations
    endif

    write(*,*) "  * [Fortran] Exiting..."

end program