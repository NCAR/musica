program test_musica_version

  use musica,      only: get_musica_version
  use musica_util, only: string_t

  type(string_t) :: musica_version

  musica_version = get_musica_version()
  print *, "MUSICA version: ", musica_version%value_

end program test_musica_version