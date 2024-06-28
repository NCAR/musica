! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_grid_map

  use iso_c_binding,       only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_grid_warehouse, only : grid_warehouse_t
  use tuvx_grid,           only : grid_t
  use musica_tuvx_util,    only : to_f_string, string_t_c
  use musica_string,       only : string_t

  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function interal_get_grid(grid_map, c_grid_name, c_grid_name_length, c_grid_units, c_grid_units_length, error_code) &
        result(grid_ptr) bind(C, name="InternalGetGrid")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t
    
      ! arguments
      type(c_ptr), intent(in), value                   :: grid_map
      character(len=1, kind=c_char), dimension(*), intent(in) :: c_grid_name
      integer(kind=c_size_t), value                    :: c_grid_name_length
      character(len=1, kind=c_char), dimension(*), intent(in) :: c_grid_units
      integer(kind=c_size_t), value                    :: c_grid_units_length
      integer(kind=c_int), intent(out)                 :: error_code
    
      ! variables
      type(grid_t), pointer           :: grid
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

      grid => grid_warehouse%get_grid(f_grid_name, f_grid_units)
    
      grid_ptr = c_loc(grid)
    
    end function interal_get_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_grid_map
