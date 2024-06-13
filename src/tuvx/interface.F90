! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface

  use iso_c_binding,       only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_core,           only : core_t
  use tuvx_grid_warehouse, only : grid_warehouse_t
  use tuvx_grid,           only : grid_t
  use musica_tuvx_util,    only : to_f_string, string_t_c, delete_string_c
  use musica_string,       only : string_t
  use tuvx_grid_warehouse, only : grid_warehouse_t

  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function internal_create_tuvx(c_config_path, config_path_len, error_code) bind(C, name="InternalCreateTuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      ! arguments
      character(kind=c_char), dimension(*), intent(in) :: c_config_path
      integer(kind=c_size_t), value                    :: config_path_len
      integer(kind=c_int), intent(out)                 :: error_code

      ! local variables
      character(len=:), allocatable :: f_config_path
      type(c_ptr)                   :: internal_create_tuvx
      type(core_t), pointer         :: core
      type(string_t)                :: musica_config_path
      integer                       :: i

      allocate(character(len=config_path_len) :: f_config_path)
      do i = 1, config_path_len
        f_config_path(i:i) = c_config_path(i)
      end do

      musica_config_path = string_t(f_config_path)

      core => core_t(musica_config_path)

      deallocate(f_config_path)
      error_code = 0

      internal_create_tuvx = c_loc(core)

    end function internal_create_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_delete_tuvx(tuvx, error_code) bind(C, name="InternalDeleteTuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      ! arguments
      type(c_ptr), value, intent(in)   :: tuvx
      integer(kind=c_int), intent(out) :: error_code

      ! local variables
      type(core_t), pointer :: core
    
      call c_f_pointer(tuvx, core)
      if (associated(core)) then
        deallocate(core)
      end if
    end subroutine internal_delete_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function internal_get_grid_map(tuvx, error_code) result(grid_map_ptr) bind(C, name="InternalGetGridMap")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    
      ! arguments
      type(c_ptr), intent(in), value   :: tuvx
      integer(kind=c_int), intent(out) :: error_code
    
      ! result
      type(c_ptr) :: grid_map_ptr
    
      ! variables
      type(core_t), pointer   :: core
      type(grid_warehouse_t), pointer :: grid_warehouse
    
      call c_f_pointer(tuvx, core)
      grid_warehouse => core%get_grid_warehouse()
    
      grid_map_ptr = c_loc(grid_warehouse)
    
    end function internal_get_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_delete_grid_map(grid_map, error_code) bind(C, name="InternalDeleteGridMap")
      use iso_c_binding, only: c_ptr, c_f_pointer
    
      ! arguments
      type(c_ptr), value, intent(in) :: grid_map
      integer(kind=c_int), intent(out) :: error_code
    
      ! variables
      type(grid_warehouse_t), pointer :: grid_warehouse
    
      call c_f_pointer(grid_map, grid_warehouse)
      if (associated(grid_warehouse)) then
        deallocate(grid_warehouse)
      end if
    
    end subroutine internal_delete_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function interal_get_grid(grid_map, grid_name, grid_units, error_code) result(grid_ptr) bind(C, name="InternalGetGrid")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    
      ! arguments
      type(c_ptr), intent(in), value      :: grid_map
      type(string_t_c), intent(in), value :: grid_name
      type(string_t_c), intent(in), value :: grid_units
      integer(kind=c_int), intent(out)    :: error_code
    
      ! result
      type(c_ptr) :: grid_ptr
    
      ! variables
      type(grid_t), pointer           :: grid
      type(grid_warehouse_t), pointer :: grid_warehouse
      character(len=:), allocatable   :: f_grid_name
      character(len=:), allocatable   :: f_grid_units
    
      f_grid_name = to_f_string(grid_name)
      f_grid_units = to_f_string(grid_units)
    
      call c_f_pointer(grid_map, grid_warehouse)

      grid => grid_warehouse%get_grid(f_grid_name, f_grid_units)
    
      grid_ptr = c_loc(grid)
    
    end function interal_get_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_delete_grid(grid, error_code) bind(C, name="InternalDeleteGrid")
      use iso_c_binding, only: c_ptr, c_f_pointer
    
      ! arguments
      type(c_ptr), value, intent(in) :: grid
      integer(kind=c_int), intent(out) :: error_code
    
      ! variables
      type(grid_t), pointer :: f_grid
    
      call c_f_pointer(grid, f_grid)
      if (associated(f_grid)) then
        deallocate(f_grid)
      end if
    
    end subroutine internal_delete_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_set_edges(grid, edges, num_edges, error_code) bind(C, name="InternalSetEdges")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double
    
      ! arguments
      type(c_ptr), value, intent(in)                :: grid
      real(kind=c_double), intent(in), dimension(*) :: edges
      integer(kind=c_int), intent(in)               :: num_edges
      integer(kind=c_int), intent(out)              :: error_code
    
      ! variables
      type(grid_t), pointer :: f_grid
    
      call c_f_pointer(grid, f_grid)

      f_grid%edge_ = edges(1:num_edges)

      f_grid%delta_ = edges(2:num_edges) - edges(1:num_edges-1)

    end subroutine internal_set_edges

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_set_midpoints(grid, midpoints, num_midpoints, error_code) bind(C, name="InternalSetMidpoints")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double
    
      ! arguments
      type(c_ptr), value, intent(in)                :: grid
      real(kind=c_double), intent(in), dimension(*) :: midpoints
      integer(kind=c_int), intent(in)               :: num_midpoints
      integer(kind=c_int), intent(out)              :: error_code
    
      ! variables
      type(grid_t), pointer :: f_grid
    
      call c_f_pointer(grid, f_grid)

      f_grid%mid_ = midpoints(1:num_midpoints)

    end subroutine internal_set_midpoints

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface
