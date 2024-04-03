subroutine test_micm_fort_api_invalid()
  use iso_c_binding
  use micm_core, only: micm_t

  implicit none

  type(micm_t), pointer         :: micm
  integer                       :: errcode
  character(len=7)              :: config_path

  config_path = "invalid_config"

  write(*,*) "[test micm fort api] Creating MICM solver..."
  micm => micm_t(config_path, errcode)

  if (errcode /= 0) then
    write(*,*) "[test micm fort api] Failed in creating solver. Expected failure. Error code: ", errcode
    stop 0
  else
    write(*,*) "[test micm fort api] Unexpected error code: ", errcode
    stop 3
  endif

end subroutine

program test_micm_api
  call test_micm_fort_api_invalid()
end program
