program demo

    use musica_util, only: string_t
    use musica,      only: get_musica_version
    use musica_micm, only: get_micm_version

    implicit none

    type(string_t) :: micm_version
    type(string_t) :: musica_version
    
    musica_version = get_musica_version()
    micm_version = get_micm_version()

    print *, "MUSICA version: ", musica_version%value_
    print *, "MICM version ", micm_version%get_char_array()

end program demo
