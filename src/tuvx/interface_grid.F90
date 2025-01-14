! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_grid

  use tuvx_grid,           only : grid_t
  
  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_grid(grid_name, grid_name_length, units, &
      units_length, num_sections, error_code) &
      bind(C, name="InternalCreateGrid") result(grid)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_char, c_loc, c_size_t, c_int
    use musica_string, only: string_t
    use tuvx_grid_from_host, only: grid_from_host_t

    ! arguments
    type(c_ptr) :: grid
    character(kind=c_char, len=1), dimension(*), intent(in) :: grid_name
    integer(kind=c_size_t), intent(in), value :: grid_name_length
    character(kind=c_char, len=1), dimension(*), intent(in) :: units
    integer(kind=c_size_t), intent(in), value :: units_length
    integer(kind=c_size_t), intent(in), value :: num_sections
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_from_host_t), pointer :: f_grid
    type(string_t) :: f_name, f_units
    integer :: i

    allocate(character(len=grid_name_length) :: f_name%val_)
    do i = 1, grid_name_length
      f_name%val_(i:i) = grid_name(i)
    end do

    allocate(character(len=units_length) :: f_units%val_)
    do i = 1, units_length
      f_units%val_(i:i) = units(i)
    end do

    f_grid => grid_from_host_t(f_name, f_units, int(num_sections))
    grid = c_loc(f_grid)

  end function internal_create_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_grid(grid, error_code) &
    bind(C, name="InternalDeleteGrid")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

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

  function internal_get_grid_updater(grid, error_code) &
      bind(C, name="InternalGetGridUpdater") result(updater)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_loc, c_int
    use tuvx_grid_from_host, only: grid_from_host_t, grid_updater_t

    ! arguments
    type(c_ptr), value, intent(in) :: grid
    integer(kind=c_int), intent(out) :: error_code

    ! output
    type(c_ptr) :: updater

    ! variables
    type(grid_from_host_t), pointer :: f_grid
    type(grid_updater_t), pointer :: f_updater
  
    call c_f_pointer(grid, f_grid)
    allocate(f_updater, source = grid_updater_t(f_grid))
    updater = c_loc(f_updater)

  end function internal_get_grid_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_grid_updater(updater, error_code) &
      bind(C, name="InternalDeleteGridUpdater")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_grid_from_host, only: grid_updater_t

    ! arguments
    type(c_ptr), value, intent(in) :: updater
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_updater_t), pointer :: f_updater

    call c_f_pointer(updater, f_updater)
    if (associated(f_updater)) then
      deallocate(f_updater)
    end if

  end subroutine internal_delete_grid_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_num_sections(updater, error_code) &
      bind(C, name="InternalGetNumSections") result(num_sections)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use tuvx_grid_from_host, only: grid_updater_t

    ! arguments
    type(c_ptr), value,  intent(in)  :: updater
    integer(kind=c_int), intent(out) :: error_code

    ! output
    integer(kind=c_size_t) :: num_sections

    ! variables
    type(grid_updater_t), pointer :: f_updater

    call c_f_pointer(updater, f_updater)
    num_sections = f_updater%grid_%size( )

  end function internal_get_num_sections
  
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_edges(grid_updater, edges, num_edges, error_code) &
      bind(C, name="InternalSetEdges")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_grid_from_host, only: grid_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: grid_updater
    type(c_ptr), value, intent(in)            :: edges
    integer(kind=c_size_t), intent(in), value :: num_edges
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(grid_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_edges(:)

    call c_f_pointer(grid_updater, f_updater)
    call c_f_pointer(edges, f_edges, [num_edges])

    if (size(f_updater%grid_%edge_) /= num_edges) then
      error_code = 1
      return
    end if
    f_updater%grid_%edge_(:) = f_edges(:)
    f_updater%grid_%delta_(:) = f_edges(2:num_edges) - f_edges(1:num_edges-1)

  end subroutine internal_set_edges

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_edges(grid_updater, edges, num_edges, error_code) &
      bind(C, name="InternalGetEdges")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_grid_from_host, only: grid_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: grid_updater
    type(c_ptr), value, intent(in)            :: edges
    integer(kind=c_size_t), intent(in), value :: num_edges
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(grid_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_edges(:)

    call c_f_pointer(grid_updater, f_updater)
    call c_f_pointer(edges, f_edges, [num_edges])

    if (size(f_updater%grid_%edge_) /= num_edges) then
      error_code = 1
      return
    end if
    f_edges(:) = f_updater%grid_%edge_(:)

  end subroutine internal_get_edges

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_midpoints(grid_updater, midpoints, num_midpoints, &
      error_code) bind(C, name="InternalSetMidpoints")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_grid_from_host, only: grid_updater_t

    ! arguments
    type(c_ptr), value, intent(in)         :: grid_updater
    type(c_ptr), value, intent(in)         :: midpoints
    integer(kind=c_int), intent(in), value :: num_midpoints
    integer(kind=c_int), intent(out)       :: error_code

    ! variables
    type(grid_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_midpoints(:)

    call c_f_pointer(grid_updater, f_updater)
    call c_f_pointer(midpoints, f_midpoints, [num_midpoints])

    if (size(f_updater%grid_%mid_) /= num_midpoints) then
      error_code = 1
      return
    end if
    f_updater%grid_%mid_(:) = f_midpoints(:)

  end subroutine internal_set_midpoints

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_midpoints(grid_updater, midpoints, num_midpoints, &
      error_code) bind(C, name="InternalGetMidpoints")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_grid_from_host, only: grid_updater_t

    ! arguments
    type(c_ptr), value, intent(in)         :: grid_updater
    type(c_ptr), value, intent(in)         :: midpoints
    integer(kind=c_int), intent(in), value :: num_midpoints
    integer(kind=c_int), intent(out)       :: error_code

    ! variables
    type(grid_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_midpoints(:)

    call c_f_pointer(grid_updater, f_updater)
    call c_f_pointer(midpoints, f_midpoints, [num_midpoints])

    if (size(f_updater%grid_%mid_) /= num_midpoints) then
      error_code = 1
      return
    end if
    f_midpoints(:) = f_updater%grid_%mid_(:)

  end subroutine internal_get_midpoints

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_grid