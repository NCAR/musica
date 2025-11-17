! Copyright (C) 2023-2025 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_radiator_map

  use iso_c_binding,           only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_radiator,           only : radiator_t
  use tuvx_radiator_warehouse, only : radiator_warehouse_t
  use musica_string,           only : string_t

  implicit none

  private

  integer, parameter :: ERROR_NONE = 0
  integer, parameter :: ERROR_UNALLOCATED_RADIATOR_MAP = 3301
  integer, parameter :: ERROR_RADIATOR_NOT_FOUND = 3302
  integer, parameter :: ERROR_RADIATOR_TYPE_MISMATCH = 3303
  integer, parameter :: ERROR_INDEX_OUT_OF_BOUNDS = 3304
  integer, parameter :: ERROR_UNALLOCATED_RADIATOR = 3305
  integer, parameter :: ERROR_UNALLOCATED_RADIATOR_UPDATER = 3306
  integer, parameter :: INTERNAL_RADIATOR_MAP_ERROR = 3399

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_radiator_map(error_code) result(radiator_map) &
      bind(C, name="InternalCreateRadiatorMap")
    use iso_c_binding, only: c_ptr, c_int, c_null_ptr
    use tuvx_radiator_warehouse, only: radiator_warehouse_t

    ! arguments
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: radiator_map

    ! variables
    class(radiator_warehouse_t), pointer :: f_radiator_warehouse

    error_code = ERROR_NONE
    f_radiator_warehouse => radiator_warehouse_t()
    select type(f_radiator_warehouse)
    type is(radiator_warehouse_t)
      radiator_map = c_loc(f_radiator_warehouse)
    class default
      error_code = INTERNAL_RADIATOR_MAP_ERROR
      radiator_map = c_null_ptr
    end select

  end function internal_create_radiator_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_radiator_map(radiator_map, error_code) &
      bind(C, name="InternalDeleteRadiatorMap")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer
    use tuvx_radiator_warehouse, only: radiator_warehouse_t

    ! arguments
    type(c_ptr), value,  intent(in)  :: radiator_map
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(radiator_warehouse_t), pointer :: f_radiator_warehouse

    error_code = ERROR_NONE
    call c_f_pointer(radiator_map, f_radiator_warehouse)
    deallocate(f_radiator_warehouse)

end subroutine internal_delete_radiator_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_add_radiator(radiator_map, radiator, error_code) &
      bind(C, name="InternalAddRadiator")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer
    use tuvx_radiator_warehouse, only: radiator_warehouse_t
    use tuvx_radiator_from_host, only: radiator_from_host_t

    ! arguments
    type(c_ptr), value,  intent(in)  :: radiator_map
    type(c_ptr), value,  intent(in)  :: radiator
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(radiator_warehouse_t), pointer :: f_radiator_warehouse
    type(radiator_from_host_t), pointer :: f_radiator

    error_code = ERROR_NONE
    call c_f_pointer(radiator_map, f_radiator_warehouse)
    call c_f_pointer(radiator, f_radiator)
    call f_radiator_warehouse%add(f_radiator)

  end subroutine internal_add_radiator

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_radiator(radiator_map, c_radiator_name, & 
      c_radiator_name_length, error_code) &
      result(radiator_ptr) bind(C, name="InternalGetRadiator")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t, &
                            c_null_ptr, c_loc
    use tuvx_radiator_from_host, only: radiator_from_host_t

    ! arguments
    type(c_ptr),                    value, intent(in) :: radiator_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_radiator_name
    integer(kind=c_size_t),         value, intent(in) :: c_radiator_name_length
    integer(kind=c_int),                   intent(out) :: error_code

    ! variables
    class(radiator_t),          pointer :: f_radiator
    class(radiator_t),          pointer :: f_radiator_ptr
    type(radiator_warehouse_t), pointer :: radiator_warehouse
    character(len=:),       allocatable :: f_radiator_name
    integer                             :: i

    ! result
    type(c_ptr) :: radiator_ptr

    error_code = ERROR_NONE
    allocate(character(len=c_radiator_name_length) :: f_radiator_name)
    do i = 1, c_radiator_name_length
      f_radiator_name(i:i) = c_radiator_name(i)
    end do

    call c_f_pointer(radiator_map, radiator_warehouse)

    if (.not. allocated(radiator_warehouse%radiators_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR_MAP
      radiator_ptr = c_null_ptr
      return
    end if

    if (.not. radiator_warehouse%exists(f_radiator_name)) then
      error_code = ERROR_RADIATOR_NOT_FOUND
      radiator_ptr = c_null_ptr
    else
      f_radiator_ptr => radiator_warehouse%get_radiator(f_radiator_name)
      allocate(f_radiator, source = f_radiator_ptr)
      nullify(f_radiator_ptr)

      select type(f_radiator)
      type is(radiator_from_host_t)
        radiator_ptr = c_loc(f_radiator)
      class default
        error_code = ERROR_RADIATOR_TYPE_MISMATCH
        deallocate(f_radiator)
        radiator_ptr = c_null_ptr
      end select
    end if

  end function internal_get_radiator

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_radiator_by_index(radiator_map, index, error_code) &
      result(radiator_ptr) bind(C, name="InternalGetRadiatorByIndex")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_null_ptr, c_loc
    use tuvx_radiator_from_host, only: radiator_from_host_t

    ! arguments
    type(c_ptr),                    value, intent(in)  :: radiator_map
    integer(kind=c_int),            value, intent(in)  :: index
    integer(kind=c_int),                   intent(out) :: error_code

    ! variables
    class(radiator_t),          pointer :: f_radiator
    type(radiator_warehouse_t), pointer :: radiator_warehouse

    ! result
    type(c_ptr) :: radiator_ptr

    error_code = ERROR_NONE
    call c_f_pointer(radiator_map, radiator_warehouse)

    if (.not. allocated(radiator_warehouse%radiators_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR_MAP
      radiator_ptr = c_null_ptr
      return
    end if

    ! underflow of unsigned index is covered by this check
    if (index >= size(radiator_warehouse%radiators_)) then
      error_code = ERROR_INDEX_OUT_OF_BOUNDS
      radiator_ptr = c_null_ptr
      return
    end if
    
    f_radiator => null()
    allocate(f_radiator, source = radiator_warehouse%radiators_(index+1)%val_)

    select type(f_radiator)
    type is(radiator_from_host_t)
      radiator_ptr = c_loc(f_radiator)
    class default
      error_code = ERROR_RADIATOR_TYPE_MISMATCH
      deallocate(f_radiator)
      radiator_ptr = c_null_ptr
    end select

  end function internal_get_radiator_by_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  integer function internal_get_number_of_radiators(radiator_map, error_code) &
      bind(C, name="InternalGetNumberOfRadiators")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_radiator_warehouse, only: radiator_warehouse_t

    ! arguments
    type(c_ptr), value,  intent(in)  :: radiator_map
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(radiator_warehouse_t), pointer :: f_radiator_warehouse

    error_code = ERROR_NONE
    call c_f_pointer(radiator_map, f_radiator_warehouse)

    if (.not. allocated(f_radiator_warehouse%radiators_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR_MAP
      internal_get_number_of_radiators = -1
      return
    end if
    internal_get_number_of_radiators = size(f_radiator_warehouse%radiators_)

  end function internal_get_number_of_radiators

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_remove_radiator(radiator_map, c_radiator_name, &
      c_radiator_name_length, error_code) &
      bind(C, name="InternalRemoveRadiator")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t
    use tuvx_radiator_warehouse, only: radiator_warehouse_t

    ! arguments
    type(c_ptr),                 value, intent(in)  :: radiator_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_radiator_name
    integer(kind=c_size_t),      value, intent(in)  :: c_radiator_name_length
    integer(kind=c_int),                intent(out) :: error_code

    ! variables
    type(radiator_warehouse_t), pointer :: f_radiator_warehouse
    character(len=:),       allocatable :: f_radiator_name
    integer                             :: i

    error_code = ERROR_NONE
    allocate(character(len=c_radiator_name_length) :: f_radiator_name)
    do i = 1, c_radiator_name_length
      f_radiator_name(i:i) = c_radiator_name(i)
    end do

    call c_f_pointer(radiator_map, f_radiator_warehouse)
    if (.not. allocated(f_radiator_warehouse%radiators_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR_MAP
      return
    end if
    do i = 1, size(f_radiator_warehouse%radiators_)
      if (f_radiator_warehouse%radiators_(i)%val_%handle_%val_ == &
          f_radiator_name) then
        call internal_remove_radiator_by_index(radiator_map, &
            int(i - 1, kind=c_size_t), error_code)
        return
      end if
    end do
    error_code = ERROR_RADIATOR_NOT_FOUND

  end subroutine internal_remove_radiator

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_remove_radiator_by_index(radiator_map, index, &
      error_code) bind(C, name="InternalRemoveRadiatorByIndex")
    use iso_c_binding, only: c_ptr, c_size_t, c_f_pointer, c_int
    use tuvx_radiator_warehouse, only: radiator_warehouse_t
    use tuvx_radiator, only: radiator_ptr

    ! arguments
    type(c_ptr),                 value, intent(in)  :: radiator_map
    integer(kind=c_size_t),      value, intent(in)  :: index
    integer(kind=c_int),                intent(out) :: error_code

    ! variables
    type(radiator_warehouse_t), pointer :: f_radiator_warehouse
    integer :: i, j
    type(radiator_ptr), allocatable :: temp_radiators(:)

    error_code = ERROR_NONE
    call c_f_pointer(radiator_map, f_radiator_warehouse)

    if (.not. allocated(f_radiator_warehouse%radiators_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR_MAP
      return
    end if

    ! underflow of unsigned index is covered by this check
    if (index >= size(f_radiator_warehouse%radiators_)) then
      error_code = ERROR_INDEX_OUT_OF_BOUNDS
      return
    end if

    allocate(temp_radiators(size(f_radiator_warehouse%radiators_) - 1))
    j = 1
    do i = 1, size(f_radiator_warehouse%radiators_)
      if (i - 1 /= index) then
        temp_radiators(j) = f_radiator_warehouse%radiators_(i)
        j = j + 1
      else
        if (associated(f_radiator_warehouse%radiators_(i)%val_)) then
          deallocate(f_radiator_warehouse%radiators_(i)%val_)
          f_radiator_warehouse%radiators_(i)%val_ => null()
        end if
      end if
    end do
    deallocate(f_radiator_warehouse%radiators_)
    allocate(f_radiator_warehouse%radiators_(size(temp_radiators)))
    do i = 1, size(temp_radiators)
      f_radiator_warehouse%radiators_(i)%val_ => temp_radiators(i)%val_
      temp_radiators(i)%val_ => null()
    end do
    deallocate(temp_radiators)

  end subroutine internal_remove_radiator_by_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_radiator_updater_from_map(radiator_map, radiator, error_code) &
      result(updater) bind(C, name="InternalGetRadiatorUpdaterFromMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_loc
    use tuvx_radiator_warehouse, only: radiator_warehouse_t
    use tuvx_radiator_from_host, only: radiator_from_host_t, radiator_updater_t

    ! arguments
    type(c_ptr), value,  intent(in)  :: radiator_map
    type(c_ptr), value,  intent(in)  :: radiator
    integer(kind=c_int), intent(out) :: error_code

    ! output
    type(c_ptr) :: updater

    ! variables
    type(radiator_warehouse_t), pointer :: f_radiator_warehouse
    type(radiator_from_host_t), pointer :: f_radiator
    type(radiator_updater_t),   pointer :: f_updater

    call c_f_pointer(radiator_map, f_radiator_warehouse)
    call c_f_pointer(radiator, f_radiator)

    error_code = ERROR_NONE
    allocate(f_updater)
    f_updater = f_radiator_warehouse%get_updater(f_radiator)
    updater = c_loc(f_updater)

  end function internal_get_radiator_updater_from_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_radiator_map