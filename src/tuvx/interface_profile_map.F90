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

  integer, parameter :: ERROR_NONE = 0
  integer, parameter :: ERROR_UNALLOCATED_PROFILE_MAP = 201
  integer, parameter :: ERROR_UNALLOCATED_PROFILE = 202
  integer, parameter :: ERROR_UNALLOCATED_PROFILE_UPDATER = 203
  integer, parameter :: ERROR_PROFILE_NAME_NOT_FOUND = 204
  integer, parameter :: ERROR_PROFILE_UNITS_MISMATCH = 205
  integer, parameter :: ERROR_PROFILE_TYPE_MISMATCH = 206
  integer, parameter :: ERROR_INDEX_OUT_OF_BOUNDS = 207
  integer, parameter :: INTERNAL_PROFILE_MAP_ERROR = 299

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

    error_code = ERROR_NONE
    f_profile_warehouse => profile_warehouse_t()
    select type(f_profile_warehouse)
    type is(profile_warehouse_t)
      profile_map = c_loc(f_profile_warehouse)
    class default
      error_code = INTERNAL_PROFILE_MAP_ERROR
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

    error_code = ERROR_NONE
    call c_f_pointer(profile_map, f_profile_warehouse)
    deallocate(f_profile_warehouse)

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

    error_code = ERROR_NONE
    call c_f_pointer(profile_map, f_profile_warehouse)
    call c_f_pointer(profile, f_profile)
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
    integer                            :: i, i_profile

    ! result
    type(c_ptr) :: profile_ptr

    error_code = ERROR_NONE
    allocate(character(len=c_profile_name_length) :: f_profile_name)
    do i = 1, c_profile_name_length
      f_profile_name(i:i) = c_profile_name(i)
    end do

    allocate(character(len=c_profile_units_length) :: f_profile_units)
    do i = 1, c_profile_units_length
      f_profile_units(i:i) = c_profile_units(i)
    end do

    call c_f_pointer(profile_map, profile_warehouse)

    if (.not. allocated(profile_warehouse%profiles_)) then
      error_code = ERROR_UNALLOCATED_PROFILE_MAP
      profile_ptr = c_null_ptr
      return
    end if

    f_profile => null()
    do i_profile = 1, size(profile_warehouse%profiles_)
      if (profile_warehouse%profiles_(i_profile)%val_%handle_ == &
          f_profile_name) then
        if (profile_warehouse%profiles_(i_profile)%val_%units_ /= &
          f_profile_units) then
          error_code = ERROR_PROFILE_UNITS_MISMATCH
          profile_ptr = c_null_ptr
          return
        end if
        allocate(f_profile, source=profile_warehouse%profiles_(i_profile)%val_)
        exit
      end if
    end do
    if (.not. associated(f_profile)) then
      error_code = ERROR_PROFILE_NAME_NOT_FOUND
      profile_ptr = c_null_ptr
      return
    end if

    select type(f_profile)
    type is(profile_from_host_t)
      profile_ptr = c_loc(f_profile)
    class default
      error_code = ERROR_PROFILE_TYPE_MISMATCH
      deallocate(f_profile)
      profile_ptr = c_null_ptr
    end select

  end function internal_get_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile_by_index(profile_map, index, error_code) &
      result(profile_ptr) bind(C, name="InternalGetProfileByIndex")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_null_ptr, c_loc
    use tuvx_profile_from_host, only: profile_from_host_t

    ! arguments
    type(c_ptr), intent(in), value :: profile_map
    integer(kind=c_int), value     :: index
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    class(profile_t), pointer          :: f_profile
    type(profile_warehouse_t), pointer :: profile_warehouse

    ! result
    type(c_ptr) :: profile_ptr

    error_code = ERROR_NONE
    call c_f_pointer(profile_map, profile_warehouse)

    if (.not. allocated(profile_warehouse%profiles_)) then
      error_code = ERROR_UNALLOCATED_PROFILE_MAP
      profile_ptr = c_null_ptr
      return
    end if

    if (index < 0 .or. index >= size(profile_warehouse%profiles_)) then
      error_code = ERROR_INDEX_OUT_OF_BOUNDS
      profile_ptr = c_null_ptr
      return
    end if

    f_profile => null()
    allocate(f_profile, source=profile_warehouse%profiles_(index+1)%val_)

    select type(f_profile)
    type is(profile_from_host_t)
      profile_ptr = c_loc(f_profile)
    class default
      error_code = ERROR_PROFILE_TYPE_MISMATCH
      deallocate(f_profile)
      profile_ptr = c_null_ptr
    end select

  end function internal_get_profile_by_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  integer function internal_get_number_of_profiles(profile_map, error_code) &
      bind(C, name="InternalGetNumberOfProfiles")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_profile_warehouse, only: profile_warehouse_t

    ! arguments
    type(c_ptr), intent(in), value :: profile_map
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_warehouse_t), pointer :: f_profile_warehouse

    error_code = ERROR_NONE
    call c_f_pointer(profile_map, f_profile_warehouse)

    if (.not. allocated(f_profile_warehouse%profiles_)) then
      error_code = ERROR_UNALLOCATED_PROFILE_MAP
      internal_get_number_of_profiles = -1
      return
    end if
    internal_get_number_of_profiles = size(f_profile_warehouse%profiles_)

  end function internal_get_number_of_profiles

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_remove_profile(profile_map, c_profile_name, &
      c_profile_name_length, c_profile_units, c_profile_units_length, &
      error_code) bind(C, name="InternalRemoveProfile")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t
    use tuvx_profile_warehouse, only: profile_warehouse_t
    
    ! arguments
    type(c_ptr), intent(in), value                   :: profile_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_name
    integer(kind=c_size_t), value                    :: c_profile_name_length
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_units
    integer(kind=c_size_t), value                    :: c_profile_units_length
    integer(kind=c_int), intent(out)                 :: error_code

    ! variables
    type(profile_warehouse_t), pointer :: profile_warehouse
    character(len=:), allocatable      :: f_profile_name
    character(len=:), allocatable      :: f_profile_units
    integer                            :: i

    error_code = ERROR_NONE
    allocate(character(len=c_profile_name_length) :: f_profile_name)
    do i = 1, c_profile_name_length
      f_profile_name(i:i) = c_profile_name(i)
    end do

    allocate(character(len=c_profile_units_length) :: f_profile_units)
    do i = 1, c_profile_units_length
      f_profile_units(i:i) = c_profile_units(i)
    end do

    call c_f_pointer(profile_map, profile_warehouse)
    if (.not. allocated(profile_warehouse%profiles_)) then
      error_code = ERROR_UNALLOCATED_PROFILE_MAP
      return
    end if
    do i = 1, size(profile_warehouse%profiles_)
      if (profile_warehouse%profiles_(i)%val_%handle_ == f_profile_name) then
        if (profile_warehouse%profiles_(i)%val_%units_ /= f_profile_units) then
          error_code = ERROR_PROFILE_UNITS_MISMATCH
          return
        end if
        call internal_remove_profile_by_index(profile_map, &
            int(i - 1, kind=c_size_t), error_code)
        return
      end if
    end do
    error_code = ERROR_PROFILE_NAME_NOT_FOUND

  end subroutine internal_remove_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_remove_profile_by_index(profile_map, index, &
      error_code) bind(C, name="InternalRemoveProfileByIndex")
    use iso_c_binding, only: c_int, c_size_t, c_ptr, c_f_pointer
    use tuvx_profile_warehouse, only: profile_warehouse_t
    use tuvx_profile, only: profile_ptr

    ! arguments
    type(c_ptr),            value, intent(in)  :: profile_map
    integer(kind=c_size_t), value, intent(in)  :: index
    integer(kind=c_int),           intent(out) :: error_code

    type(profile_warehouse_t), pointer :: f_profile_warehouse
    integer :: i, j
    type(profile_ptr), allocatable :: temp_profiles(:)

    error_code = ERROR_NONE
    call c_f_pointer(profile_map, f_profile_warehouse)

    if (.not. allocated(f_profile_warehouse%profiles_)) then
      error_code = ERROR_UNALLOCATED_PROFILE_MAP
      return
    end if

    if (index < 0 .or. index >= size(f_profile_warehouse%profiles_)) then
      error_code = ERROR_INDEX_OUT_OF_BOUNDS
      return
    end if

    allocate(temp_profiles(size(f_profile_warehouse%profiles_)-1))
    j = 1
    do i = 1, size(f_profile_warehouse%profiles_)
      if (i-1 /= index) then
        temp_profiles(j)%val_ => f_profile_warehouse%profiles_(i)%val_
        f_profile_warehouse%profiles_(i)%val_ => null()
        j = j + 1
      else
        if(associated(f_profile_warehouse%profiles_(i)%val_)) then
          deallocate(f_profile_warehouse%profiles_(i)%val_)
          f_profile_warehouse%profiles_(i)%val_ => null()
        end if
      end if
    end do
    deallocate(f_profile_warehouse%profiles_)
    allocate(f_profile_warehouse%profiles_(size(temp_profiles)))
    do i = 1, size(temp_profiles)
      f_profile_warehouse%profiles_(i)%val_ => temp_profiles(i)%val_
      temp_profiles(i)%val_ => null()
    end do
    deallocate(temp_profiles)

  end subroutine internal_remove_profile_by_index

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
