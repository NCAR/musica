! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_profile_map

  use iso_c_binding,       only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_profile,        only : profile_t
  use tuvx_profile_warehouse, only : profile_warehouse_t
  use musica_tuvx_util,    only : to_f_string, string_t_c
  use musica_string,       only : string_t
  
  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile(profile_map, c_profile_name, c_profile_name_length, &
      c_profile_units, c_profile_units_length, error_code) &
      result(profile_ptr) bind(C, name="InternalGetProfile")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t

    ! arguments
    type(c_ptr), intent(in), value                   :: profile_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_name
    integer(kind=c_size_t), value                    :: c_profile_name_length
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_units
    integer(kind=c_size_t), value                    :: c_profile_units_length
    integer(kind=c_int), intent(out)                 :: error_code

    ! variables
    type(profile_t), pointer         :: profile
    type(profile_warehouse_t), pointer :: profile_warehouse
    character(len=:), allocatable   :: f_profile_name
    character(len=:), allocatable   :: f_profile_units
    integer                         :: i

    ! result
    type(c_ptr) :: profile_ptr

    allocate(character(len=c_profile_name_length) :: f_profile_name)
    do i = 1, c_profile_name_length
      f_profile_name(i:i) = c_profile_name(i)
    end do

    allocate(character(len=c_profile_units_length) :: f_profile_units)
    do i = 1, c_profile_units_length
      f_profile_units(i:i) = c_profile_units(i)
    end do

    call c_f_pointer(profile_map, profile_warehouse)

    profile => profile_warehouse%get_profile(f_profile_name, f_profile_units)

    profile_ptr = c_loc(profile)

  end function internal_get_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_profile_map
