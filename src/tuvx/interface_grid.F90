! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_grid

  use iso_c_binding,       only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_grid,           only : grid_t
  use musica_tuvx_util,    only : to_f_string, string_t_c
  use musica_string,       only : string_t
  
  implicit none

  private

  contains

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
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double, c_size_t
    
      ! arguments
      type(c_ptr), value, intent(in)                :: grid
      real(kind=c_double), intent(in), dimension(*) :: edges
      integer(kind=c_size_t), intent(in), value     :: num_edges
      integer(kind=c_int), intent(out)              :: error_code
    
      ! variables
      type(grid_t), pointer :: f_grid
    
      call c_f_pointer(grid, f_grid)

      f_grid%edge_ = edges(1:num_edges)

      f_grid%delta_ = edges(2:num_edges) - edges(1:num_edges-1)

      f_grid%ncells_ = num_edges - 1

    end subroutine internal_set_edges

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_set_midpoints(grid, midpoints, num_midpoints, error_code) bind(C, name="InternalSetMidpoints")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double
    
      ! arguments
      type(c_ptr), value, intent(in)                :: grid
      real(kind=c_double), intent(in), dimension(*) :: midpoints
      integer(kind=c_int), intent(in), value        :: num_midpoints
      integer(kind=c_int), intent(out)              :: error_code
    
      ! variables
      type(grid_t), pointer :: f_grid
    
      call c_f_pointer(grid, f_grid)

      f_grid%mid_ = midpoints(1:num_midpoints)

    end subroutine internal_set_midpoints

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_grid
