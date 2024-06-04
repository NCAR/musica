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

        real(c_double), dimension(3) :: concentrations

        type(error_t) :: error

        type(micm_t), pointer :: micm

        config_path = ""

        write(*,*) "Creating MICM solver..."
        micm => micm_t(config_path, error)

    end subroutine box_model

end program test_micm_box_model
