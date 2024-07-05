program test_micm_box_model

    use, intrinsic :: iso_c_binding
    use, intrinsic :: ieee_arithmetic

    use musica_util, only: error_t, string_t, mapping_t
    use musica_micm, only: micm_t, solver_stats_t

    implicit none

    call box_model()

contains

    subroutine box_model()

        character(len=256) :: config_path
        integer(c_int)     :: solver_type

        real(c_double), parameter :: GAS_CONSTANT = 8.31446261815324_c_double ! J mol-1 K-1

        real(c_double) :: time_step
        real(c_double) :: temperature
        real(c_double) :: pressure
        real(c_double) :: air_density

        integer(c_int) :: num_concentrations = 3
        real(c_double), dimension(3) :: concentrations

        integer(c_int) :: num_user_defined_reaction_rates = 0
        real(c_double), dimension(:), allocatable :: user_defined_reaction_rates 

        type(string_t)       :: solver_state
        type(solver_stats_t) :: solver_stats
        type(error_t)        :: error

        type(micm_t), pointer :: micm

        integer :: i

        config_path = "configs/analytical"
        solver_type = 2

        time_step = 200
        temperature = 273.0
        pressure = 1.0e5
        air_density = pressure / (GAS_CONSTANT * temperature)

        concentrations = (/ 1.0, 1.0, 1.0 /)

        write(*,*) "Creating MICM solver..."
        micm => micm_t(config_path, solver_type, error)

        do i = 1, size( micm%species_ordering )
            associate(the_mapping => micm%species_ordering(i))
            print *, "Species Name:", the_mapping%name(), ", Index:", the_mapping%index()
            end associate
        end do

        write(*,*) "Solving starts..."
        ! call micm%solve(time_step, temperature, pressure, num_concentrations, concentrations, &
        !    num_user_defined_reaction_rates, user_defined_reaction_rates, error)
        call micm%solve(time_step, temperature, pressure, air_density, num_concentrations, concentrations, &
            num_user_defined_reaction_rates, user_defined_reaction_rates, solver_state, solver_stats, error)
        write(*,*) "After solving, concentrations", concentrations

        deallocate( micm )

    end subroutine box_model

end program test_micm_box_model
