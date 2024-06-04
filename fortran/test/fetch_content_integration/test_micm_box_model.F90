program test_micm_box_model

    use, intrinsic :: iso_c_binding
    use, intrinsic :: ieee_arithmetic

    use musica_micm, only: micm_t
    use musica_util, only: error_t, string_t, mapping_t

    implicit none

    call box_model()

contains

    subroutine box_model()

        real(c_double) :: time_step
        real(c_double) :: temperature
        real(c_double) :: pressure

        real(c_double), dimension(3) :: concentrations

    end subroutine box_model

end program test_micm_box_model
