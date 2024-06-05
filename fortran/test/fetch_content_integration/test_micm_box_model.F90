program test_micm_box_model

    use, intrinsic :: iso_c_binding
    use, intrinsic :: ieee_arithmetic

    use musica_util, only: error_t, string_t, mapping_t
    use musica_micm, only: micm_t

    implicit none

    call box_model()

contains

    subroutine box_model()

        character(len=256) :: config_path

        real(c_double) :: time_step
        real(c_double) :: temperature
        real(c_double) :: pressure

        integer(c_int) :: num_concentrations = 3
        real(c_double), dimension(3) :: concentrations

        integer(c_int) :: num_user_defined_reaction_rates = 0
        real(c_double), dimension(:), allocatable :: user_defined_reaction_rates 

        type(error_t) :: error
        type(micm_t), pointer :: micm

        integer :: i

        config_path = "configs/analytical"

        time_step = 200
        temperature = 273.0
        pressure = 1.0e5

        concentrations = (/ 1.0, 1.0, 1.0 /)

        write(*,*) "Creating MICM solver..."
        micm => micm_t(config_path, error)

        do i = 1, size( micm%species_ordering )
            associate(the_mapping => micm%species_ordering(i))
            print *, "Species Name:", the_mapping%name(), ", Index:", the_mapping%index()
            end associate
        end do

        write(*,*) "Solving starts..."
        call micm%solve(time_step, temperature, pressure, num_concentrations, concentrations, &
            num_user_defined_reaction_rates, user_defined_reaction_rates, error)
        write(*,*) "After solving, concentrations", concentrations

    end subroutine box_model

end program test_micm_box_model
