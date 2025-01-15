! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_radiator_map

  use iso_c_binding,           only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_radiator,           only : radiator_t
  use tuvx_radiator_warehouse, only : radiator_warehouse_t
  use musica_string,           only : string_t

  implicit none

  private

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

    f_radiator_warehouse => radiator_warehouse_t()
    select type(f_radiator_warehouse)
    type is(radiator_warehouse_t)
      radiator_map = c_loc(f_radiator_warehouse)
      error_code = 0
    class default
      error_code = 1
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

    call c_f_pointer(radiator_map, f_radiator_warehouse)
    deallocate(f_radiator_warehouse)
    error_code = 0

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

    call c_f_pointer(radiator_map, f_radiator_warehouse)
    call c_f_pointer(radiator, f_radiator)

    error_code = 0
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

    allocate(character(len=c_radiator_name_length) :: f_radiator_name)
    do i = 1, c_radiator_name_length
      f_radiator_name(i:i) = c_radiator_name(i)
    end do

    call c_f_pointer(radiator_map, radiator_warehouse)

    if (.not. radiator_warehouse%exists(f_radiator_name)) then
      error_code = 1
      radiator_ptr = c_null_ptr
    else
      f_radiator_ptr => radiator_warehouse%get_radiator(f_radiator_name)
      allocate(f_radiator, source = f_radiator_ptr)
      nullify(f_radiator_ptr)

      select type(f_radiator)
      type is(radiator_from_host_t)
        error_code = 0
        radiator_ptr = c_loc(f_radiator)
      class default
        error_code = 1
        deallocate(f_radiator)
        radiator_ptr = c_null_ptr
      end select
    end if

  end function internal_get_radiator

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

    error_code = 0
    allocate(f_updater)
    f_updater = f_radiator_warehouse%get_updater(f_radiator)
    updater = c_loc(f_updater)

  end function internal_get_radiator_updater_from_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_radiator_map