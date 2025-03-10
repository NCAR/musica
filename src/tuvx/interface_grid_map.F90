! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_grid_map

  use iso_c_binding,       only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_grid_warehouse, only : grid_warehouse_t
  use tuvx_grid,           only : grid_t
  use musica_string,       only : string_t

  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_grid_map(error_code) result(grid_map) &
      bind(C, name="InternalCreateGridMap")
    use iso_c_binding, only: c_ptr, c_int, c_null_ptr
    use tuvx_grid_warehouse, only: grid_warehouse_t

    ! arguments
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: grid_map

    ! variables
    class(grid_warehouse_t), pointer :: f_grid_warehouse

    f_grid_warehouse => grid_warehouse_t()
    select type(f_grid_warehouse)
    type is(grid_warehouse_t)
      grid_map = c_loc(f_grid_warehouse)
      error_code = 0
    class default
      error_code = 1
      grid_map = c_null_ptr
    end select

  end function internal_create_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_grid_map(grid_map, error_code) &
      bind(C, name="InternalDeleteGridMap")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer
    use tuvx_grid_warehouse, only: grid_warehouse_t

    ! arguments
    type(c_ptr), intent(in), value :: grid_map
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_warehouse_t), pointer :: f_grid_warehouse

    call c_f_pointer(grid_map, f_grid_warehouse)
    deallocate(f_grid_warehouse)
    error_code = 0

  end subroutine internal_delete_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_add_grid(grid_map, grid, error_code) &
      bind(C, name="InternalAddGrid")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer
    use tuvx_grid_warehouse, only: grid_warehouse_t
    use tuvx_grid_from_host, only: grid_from_host_t

    ! arguments
    type(c_ptr), intent(in), value :: grid_map
    type(c_ptr), intent(in), value :: grid
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_warehouse_t), pointer :: f_grid_warehouse
    type(grid_from_host_t), pointer :: f_grid

    call c_f_pointer(grid_map, f_grid_warehouse)
    call c_f_pointer(grid, f_grid)

    error_code = 0
    call f_grid_warehouse%add(f_grid)

  end subroutine internal_add_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function interal_get_grid(grid_map, c_grid_name, c_grid_name_length, &
      c_grid_units, c_grid_units_length, error_code) &
      result(grid_ptr) bind(C, name="InternalGetGrid")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t, &
                              c_null_ptr, c_loc
    use tuvx_grid_from_host, only: grid_from_host_t
  
    ! arguments
    type(c_ptr), intent(in), value                   :: grid_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_grid_name
    integer(kind=c_size_t), value                    :: c_grid_name_length
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_grid_units
    integer(kind=c_size_t), value                    :: c_grid_units_length
    integer(kind=c_int), intent(out)                 :: error_code
  
    ! variables
    class(grid_t), pointer          :: f_grid
    type(grid_warehouse_t), pointer :: grid_warehouse
    character(len=:), allocatable   :: f_grid_name
    character(len=:), allocatable   :: f_grid_units
    integer                         :: i

    ! result
    type(c_ptr) :: grid_ptr

    allocate(character(len=c_grid_name_length) :: f_grid_name)
    do i = 1, c_grid_name_length
      f_grid_name(i:i) = c_grid_name(i)
    end do

    allocate(character(len=c_grid_units_length) :: f_grid_units)
    do i = 1, c_grid_units_length
      f_grid_units(i:i) = c_grid_units(i)
    end do
  
    call c_f_pointer(grid_map, grid_warehouse)

    f_grid => grid_warehouse%get_grid(f_grid_name, f_grid_units)

    select type(f_grid) 
    type is(grid_from_host_t)
      error_code = 0
      grid_ptr = c_loc(f_grid)
    class default
      error_code = 1
      deallocate(f_grid)
      grid_ptr = c_null_ptr
    end select
  
  end function interal_get_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_grid_updater_from_map(grid_map, grid, error_code) &
      result(updater) bind(C, name="InternalGetGridUpdaterFromMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_loc
    use tuvx_grid_warehouse, only: grid_warehouse_t
    use tuvx_grid_from_host, only: grid_from_host_t, grid_updater_t

    ! arguments
    type(c_ptr), intent(in), value :: grid_map
    type(c_ptr), intent(in), value :: grid
    integer(kind=c_int), intent(out) :: error_code

    ! output
    type(c_ptr) :: updater

    ! variables
    type(grid_warehouse_t), pointer :: f_grid_warehouse
    type(grid_from_host_t), pointer :: f_grid
    type(grid_updater_t), pointer :: f_updater

    call c_f_pointer(grid_map, f_grid_warehouse)
    call c_f_pointer(grid, f_grid)

    error_code = 0
    allocate(f_updater)
    f_updater = f_grid_warehouse%get_updater(f_grid)
    updater = c_loc(f_updater)

  end function internal_get_grid_updater_from_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_grid_map
