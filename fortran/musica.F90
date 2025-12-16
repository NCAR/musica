! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica
   use iso_c_binding

   implicit none

   private
   public :: get_musica_version

   interface
    subroutine get_musica_version_c(musica_version) bind(C, name="MusicaVersion")
      use musica_util, only: string_t_c
      type(string_t_c), intent(out)             :: musica_version
    end subroutine get_musica_version_c
   end interface

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the MUSICA version
  function get_musica_version() result(value)
    use musica_util, only: string_t, string_t_c
    type(string_t)   :: value
    type(string_t_c) :: string_c
    call get_musica_version_c(string_c)
    value = string_t(string_c)        
  end function get_musica_version

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica