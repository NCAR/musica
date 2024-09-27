program demo

    use musica_util, only: string_t
    use musica_micm, only: get_micm_version
    implicit none

    type(string_t) :: micm_version
    
    micm_version = get_micm_version()
    print *, "MICM version ", micm_version%get_char_array()

end program demo
