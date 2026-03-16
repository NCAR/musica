program test_tuvx_version

  use musica_util, only: string_t
  use musica_tuvx, only: get_tuvx_version
  implicit none

  type(string_t) :: tuvx_version

  tuvx_version = get_tuvx_version()
  print *, "TUVX version: ", tuvx_version%value_

end program test_tuvx_version
