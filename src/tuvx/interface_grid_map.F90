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

  integer, parameter :: ERROR_NONE = 0
  integer, parameter :: ERROR_UNALLOCATED_GRID_MAP = 100
  integer, parameter :: ERROR_UNALLOCATED_GRID = 101
  integer, parameter :: ERROR_UNALLOCATED_GRID_UPDATER = 102
  integer, parameter :: ERROR_GRID_NAME_NOT_FOUND = 103
  integer, parameter :: ERROR_GRID_UNIT_MISMATCH = 104
  integer, parameter :: ERROR_GRID_TYPE_MISMATCH = 105
  integer, parameter :: ERROR_INDEX_OUT_OF_BOUNDS = 106
  integer, parameter :: INTERNAL_GRID_MAP_ERROR = 199

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
      error_code = ERROR_NONE
    class default
      error_code = INTERNAL_GRID_MAP_ERROR
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
    error_code = ERROR_NONE

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

    error_code = ERROR_NONE
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

    integer :: i_grid

    allocate(character(len=c_grid_name_length) :: f_grid_name)
    do i = 1, c_grid_name_length
      f_grid_name(i:i) = c_grid_name(i)
    end do

    allocate(character(len=c_grid_units_length) :: f_grid_units)
    do i = 1, c_grid_units_length
      f_grid_units(i:i) = c_grid_units(i)
    end do
  
    call c_f_pointer(grid_map, grid_warehouse)

    if (.not.allocated(grid_warehouse%grids_)) then
      error_code = ERROR_UNALLOCATED_GRID_MAP
      grid_ptr = c_null_ptr
      return
    end if

    f_grid => null()
    do i_grid = 1, size(grid_warehouse%grids_)
      if (grid_warehouse%grids_(i_grid)%val_%handle_ == f_grid_name) then
        if (grid_warehouse%grids_(i_grid)%val_%units_ /= f_grid_units) then
          error_code = ERROR_GRID_UNIT_MISMATCH
          grid_ptr = c_null_ptr
          return
        end if
        allocate(f_grid, source=grid_warehouse%grids_(i_grid)%val_)
        exit
      end if
    end do
    
    if (.not.associated(f_grid)) then
      error_code = ERROR_GRID_NAME_NOT_FOUND
      grid_ptr = c_null_ptr
      return
    end if

    select type(f_grid) 
    type is(grid_from_host_t)
      error_code = ERROR_NONE
      grid_ptr = c_loc(f_grid)
    class default
      error_code = ERROR_GRID_TYPE_MISMATCH
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

    error_code = ERROR_NONE
    allocate(f_updater)
    f_updater = f_grid_warehouse%get_updater(f_grid)
    updater = c_loc(f_updater)

  end function internal_get_grid_updater_from_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_grid_by_index(grid_map, index, error_code) &
      result(grid_ptr) bind(C, name="InternalGetGridByIndex")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t, c_null_ptr, &
                              c_loc
    use tuvx_grid_from_host, only: grid_from_host_t
  
    ! arguments
    type(c_ptr), intent(in), value                   :: grid_map
    integer(kind=c_size_t), intent(in), value        :: index
    integer(kind=c_int), intent(out)                 :: error_code
  
    ! variables
    class(grid_t), pointer          :: f_grid
    type(grid_warehouse_t), pointer :: grid_warehouse

    ! result
    type(c_ptr) :: grid_ptr

    call c_f_pointer(grid_map, grid_warehouse)

    if (.not.allocated(grid_warehouse%grids_)) then
      error_code = ERROR_UNALLOCATED_GRID_MAP
      grid_ptr = c_null_ptr
      return
    end if
    if (index < 0 .or. index >= size(grid_warehouse%grids_)) then
      error_code = ERROR_INDEX_OUT_OF_BOUNDS
      grid_ptr = c_null_ptr
      return
    end if
    if (.not.associated(grid_warehouse%grids_(index+1)%val_)) then
      error_code = INTERNAL_GRID_MAP_ERROR
      grid_ptr = c_null_ptr
      return
    end if

    allocate(f_grid, source=grid_warehouse%grids_(index+1)%val_)

    select type(f_grid) 
    type is(grid_from_host_t)
      error_code = ERROR_NONE
      grid_ptr = c_loc(f_grid)
    class default
      error_code = ERROR_GRID_TYPE_MISMATCH
      deallocate(f_grid)
      grid_ptr = c_null_ptr
    end select
  
  end function internal_get_grid_by_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  integer function internal_get_number_of_grids(grid_map, error_code) &
      result(n_grids) bind(C, name="InternalGetNumberOfGrids")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_grid_warehouse, only: grid_warehouse_t

    ! arguments
    type(c_ptr), intent(in), value :: grid_map
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_warehouse_t), pointer :: f_grid_warehouse

    call c_f_pointer(grid_map, f_grid_warehouse)

    if (.not.allocated(f_grid_warehouse%grids_)) then
      error_code = ERROR_UNALLOCATED_GRID_MAP
      n_grids = -1
      return
    end if
    error_code = ERROR_NONE
    n_grids = size(f_grid_warehouse%grids_)

  end function internal_get_number_of_grids

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_remove_grid(grid_map, c_grid_name, c_grid_name_length, &
      c_grid_units, c_grid_units_length, error_code) &
      bind(C, name="InternalRemoveGrid")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer, c_char, c_size_t
    use tuvx_grid_warehouse, only: grid_warehouse_t

    ! arguments
    type(c_ptr), intent(in), value                   :: grid_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_grid_name
    integer(kind=c_size_t), value                    :: c_grid_name_length
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_grid_units
    integer(kind=c_size_t), value                    :: c_grid_units_length
    integer(kind=c_int), intent(out)                 :: error_code

    ! variables
    type(grid_warehouse_t), pointer :: f_grid_warehouse
    character(len=:), allocatable   :: f_grid_name
    character(len=:), allocatable   :: f_grid_units
    integer                         :: i
    
    allocate(character(len=c_grid_name_length) :: f_grid_name)
    do i = 1, c_grid_name_length
      f_grid_name(i:i) = c_grid_name(i)
    end do

    allocate(character(len=c_grid_units_length) :: f_grid_units)
    do i = 1, c_grid_units_length
      f_grid_units(i:i) = c_grid_units(i)
    end do

    call c_f_pointer(grid_map, f_grid_warehouse)

    error_code = ERROR_NONE
    if (.not.allocated(f_grid_warehouse%grids_)) then
      error_code = ERROR_UNALLOCATED_GRID_MAP
      return
    end if
    do i = 1, size(f_grid_warehouse%grids_)
      if (f_grid_warehouse%grids_(i)%val_%handle_ == f_grid_name) then
        if (f_grid_warehouse%grids_(i)%val_%units_ == f_grid_units) then
          call internal_remove_grid_by_index(grid_map, int(i-1, kind=c_size_t), error_code)
          return
        else
          error_code = ERROR_GRID_UNIT_MISMATCH
        end if
      end if
    end do
    error_code = ERROR_GRID_NAME_NOT_FOUND

  end subroutine internal_remove_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_remove_grid_by_index(grid_map, index, error_code) &
      bind(C, name="InternalRemoveGridByIndex")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer, c_size_t
    use tuvx_grid_warehouse, only: grid_warehouse_t
    use tuvx_grid, only: grid_ptr

    ! arguments
    type(c_ptr), intent(in), value :: grid_map
    integer(kind=c_size_t), intent(in), value :: index
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_warehouse_t), pointer :: f_grid_warehouse
    integer :: i, j
    type(grid_ptr), allocatable :: temp_grids(:)

    call c_f_pointer(grid_map, f_grid_warehouse)

    if (.not.allocated(f_grid_warehouse%grids_)) then
      error_code = ERROR_UNALLOCATED_GRID_MAP
      return
    end if
    if (index < 0 .or. index >= size(f_grid_warehouse%grids_)) then
      error_code = ERROR_INDEX_OUT_OF_BOUNDS
      return
    end if

    error_code = ERROR_NONE
    allocate(temp_grids(size(f_grid_warehouse%grids_) - 1))

    j = 1
    do i = 1, size(f_grid_warehouse%grids_)
      if (i-1 /= index) then
        temp_grids(j)%val_ => f_grid_warehouse%grids_(i)%val_
        f_grid_warehouse%grids_(i)%val_ => null()
        j = j + 1
      else
        if (associated(f_grid_warehouse%grids_(i)%val_)) then
          deallocate(f_grid_warehouse%grids_(i)%val_)
          f_grid_warehouse%grids_(i)%val_ => null()
        end if
      end if
    end do

    deallocate(f_grid_warehouse%grids_)
    allocate(f_grid_warehouse%grids_(size(temp_grids)))
    do i = 1, size(temp_grids)
      f_grid_warehouse%grids_(i)%val_ => temp_grids(i)%val_
      temp_grids(i)%val_ => null()
    end do
    deallocate(temp_grids)

  end subroutine internal_remove_grid_by_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_grid_map
