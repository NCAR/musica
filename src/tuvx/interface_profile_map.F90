! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_profile_map

  use iso_c_binding,          only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_profile,           only : profile_t
  use tuvx_profile_warehouse, only : profile_warehouse_t
  use musica_string,          only : string_t
  
  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_profile_map(error_code) result(profile_map) &
      bind(C, name="InternalCreateProfileMap")
    use iso_c_binding, only: c_ptr, c_int, c_null_ptr
    use tuvx_profile_warehouse, only: profile_warehouse_t

    ! arguments
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: profile_map

    ! variables
    class(profile_warehouse_t), pointer :: f_profile_warehouse

    f_profile_warehouse => profile_warehouse_t()
    select type(f_profile_warehouse)
    type is(profile_warehouse_t)
      profile_map = c_loc(f_profile_warehouse)
      error_code = 0
    class default
      error_code = 1
      profile_map = c_null_ptr
    end select

  end function internal_create_profile_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_profile_map(profile_map, error_code) &
      bind(C, name="InternalDeleteProfileMap")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer
    use tuvx_profile_warehouse, only: profile_warehouse_t

    ! arguments
    type(c_ptr), intent(in), value :: profile_map
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_warehouse_t), pointer :: f_profile_warehouse

    call c_f_pointer(profile_map, f_profile_warehouse)
    deallocate(f_profile_warehouse)
    error_code = 0

  end subroutine internal_delete_profile_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_add_profile(profile_map, profile, error_code) &
      bind(C, name="InternalAddProfile")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_profile_warehouse, only: profile_warehouse_t
    use tuvx_profile_from_host, only: profile_from_host_t

    ! arguments
    type(c_ptr), intent(in), value :: profile_map
    type(c_ptr), intent(in), value :: profile
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_warehouse_t), pointer :: f_profile_warehouse
    type(profile_from_host_t), pointer :: f_profile

    call c_f_pointer(profile_map, f_profile_warehouse)
    call c_f_pointer(profile, f_profile)

    error_code = 0
    call f_profile_warehouse%add(f_profile)

  end subroutine internal_add_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile(profile_map, c_profile_name, &
      c_profile_name_length, c_profile_units, c_profile_units_length, &
      error_code) result(profile_ptr) bind(C, name="InternalGetProfile")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t, &
                             c_null_ptr, c_loc
    use tuvx_profile_from_host, only: profile_from_host_t

    ! arguments
    type(c_ptr), intent(in), value                   :: profile_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_name
    integer(kind=c_size_t), value                    :: c_profile_name_length
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_units
    integer(kind=c_size_t), value                    :: c_profile_units_length
    integer(kind=c_int), intent(out)                 :: error_code

    ! variables
    class(profile_t), pointer          :: f_profile
    type(profile_warehouse_t), pointer :: profile_warehouse
    character(len=:), allocatable      :: f_profile_name
    character(len=:), allocatable      :: f_profile_units
    integer                            :: i

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

    f_profile => profile_warehouse%get_profile(f_profile_name, f_profile_units)

    select type(f_profile)
    type is(profile_from_host_t)
      profile_ptr = c_loc(f_profile)
      error_code = 0
    class default
      error_code = 1
      deallocate(f_profile)
      profile_ptr = c_null_ptr
    end select

  end function internal_get_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile_updater_from_map(profile_map, profile, &
      error_code) result(updater) &
      bind(C, name="InternalGetProfileUpdaterFromMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_profile_warehouse, only: profile_warehouse_t
    use tuvx_profile_from_host, only: profile_from_host_t, profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in) :: profile_map
    type(c_ptr), value, intent(in) :: profile
    integer(kind=c_int), intent(out) :: error_code

    ! output
    type(c_ptr) :: updater

    ! variables
    type(profile_warehouse_t), pointer :: f_profile_warehouse
    type(profile_from_host_t), pointer :: f_profile
    type(profile_updater_t), pointer :: f_updater

    call c_f_pointer(profile_map, f_profile_warehouse)
    call c_f_pointer(profile, f_profile)

    error_code = 0
    allocate(f_updater)
    f_updater = f_profile_warehouse%get_updater(f_profile)
    updater = c_loc(f_updater)

  end function internal_get_profile_updater_from_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_profile_map
