program test_simple

  use musica_micm, only: get_micm_version
  use musica_util, only: string_t

  type(string_t) :: micm_version

  micm_version = get_micm_version()
  print *, "MICM version: ", micm_version%value_

end program test_simple