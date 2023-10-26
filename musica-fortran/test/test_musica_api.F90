program test
    use iso_c_binding
    use musica_micm
    implicit none
    type(micm_t) :: m

    real(c_double) :: temperature      
    real(c_double) :: pressure
    real(c_double) :: time_step
    real(c_double), dimension(10) :: concentrations
    integer(c_size_t) :: num_concentrations

    temperature = 10d0
    pressure = 20d0
    time_step = 1d0
    concentrations = (/ 1d0, 2d0, 3d0, 4d0, 5d0, 6d0, 7d0, 8d0, 9d0, 10d0 /)
    num_concentrations = 10

    write(*,*) "  * [Fortran] Creating MICM"
    m = micm_t("micm_config")

    write(*,*) "  * [Fortran] Creating solver"
    write(*,*) "  * [Fortran] Solver creating status indicates ", m%create_solver(), " (1 is success, else failure) "

    write(*,*) "  * [Fortran] Initial temp", temperature
    write(*,*) "  * [Fortran] Initial pressure", pressure
    write(*,*) "  * [Fortran] Initial time_step", time_step
    write(*,*) "  * [Fortran] Initial concentrations", concentrations
    write(*,*) "  * [Fortran] Initial number of concentrations", num_concentrations

    write(*,*) "  * [Fortran] Starting to solve"
    call m%solve(temperature, pressure, time_step, concentrations, num_concentrations)
    write(*,*) "  * [Fortran] After solving, concentrations", concentrations

    write(*,*) "  * [Fortran] Calling destructor for MICM"
    call m%delete

end program