! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface

  use iso_c_binding,       only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_core,           only : core_t
  use tuvx_grid_warehouse, only : grid_warehouse_t
  use tuvx_grid,           only : grid_t
  use tuvx_profile,        only : profile_t
  use tuvx_profile_warehouse, only : profile_warehouse_t
  use musica_tuvx_util,    only : to_f_string, string_t_c
  use musica_string,       only : string_t
  use tuvx_grid_warehouse, only : grid_warehouse_t

  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function internal_create_tuvx(c_config_path, config_path_length, error_code) bind(C, name="InternalCreateTuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      ! arguments
      character(kind=c_char), dimension(*), intent(in) :: c_config_path
      integer(kind=c_size_t), value                    :: config_path_length
      integer(kind=c_int), intent(out)                 :: error_code

      ! local variables
      character(len=:), allocatable :: f_config_path
      type(c_ptr)                   :: internal_create_tuvx
      type(core_t), pointer         :: core
      type(string_t)                :: musica_config_path
      integer                       :: i

      allocate(character(len=config_path_length) :: f_config_path)
      do i = 1, config_path_length
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

  function internal_get_profile_map(tuvx, error_code) result(profile_map_ptr) bind(C, name="InternalGetProfileMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

    ! arguments
    type(c_ptr), intent(in), value   :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: profile_map_ptr

    ! variables
    type(core_t), pointer :: core
    type(profile_warehouse_t), pointer :: profile_warehouse

    call c_f_pointer(tuvx, core)
    profile_warehouse => core%get_profile_warehouse()
    
    profile_map_ptr = c_loc(profile_warehouse)

  end function internal_get_profile_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile(profile_map, c_profile_name, c_profile_name_length, &
      c_profile_units, c_profile_units_length, error_code) &
      result(profile_ptr) bind(C, name="InternalGetProfile")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_char, c_size_t

    ! arguments
    type(c_ptr), intent(in), value                   :: profile_map
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_name
    integer(kind=c_size_t), value                    :: c_profile_name_length
    character(len=1, kind=c_char), dimension(*), intent(in) :: c_profile_units
    integer(kind=c_size_t), value                    :: c_profile_units_length
    integer(kind=c_int), intent(out)                 :: error_code

    ! variables
    type(profile_t), pointer         :: profile
    type(profile_warehouse_t), pointer :: profile_warehouse
    character(len=:), allocatable   :: f_profile_name
    character(len=:), allocatable   :: f_profile_units
    integer                         :: i

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

    profile => profile_warehouse%get_profile(f_profile_name, f_profile_units)

    profile_ptr = c_loc(profile)

  end function internal_get_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_profile(profile, error_code) bind(C, name="InternalDeleteProfile")
    use iso_c_binding, only: c_ptr, c_f_pointer

    ! arguments
    type(c_ptr), value, intent(in) :: profile
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_t), pointer :: f_profile

    call c_f_pointer(profile, f_profile)
    if (associated(f_profile)) then
      deallocate(f_profile)
    end if

  end subroutine internal_delete_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_edge_values(profile, edge_values, num_edge_values, error_code) bind(C, name="InternalSetEdgeValues")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double

    ! arguments
    type(c_ptr), value, intent(in)                :: profile
    real(kind=c_double), intent(in), dimension(*) :: edge_values
    integer(kind=c_int), intent(in), value        :: num_edge_values
    integer(kind=c_int), intent(out)              :: error_code

    ! variables
    type(profile_t), pointer :: f_profile

    call c_f_pointer(profile, f_profile)

    f_profile%edge_val_ = edge_values(1:num_edge_values)

  end subroutine internal_set_edge_values

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_midpoint_values(profile, midpoint_values, num_midpoint_values, &
    error_code) bind(C, name="InternalSetMidpointValues")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double

    ! arguments
    type(c_ptr), value, intent(in)                :: profile
    real(kind=c_double), intent(in), dimension(*) :: midpoint_values
    integer(kind=c_int), intent(in), value        :: num_midpoint_values
    integer(kind=c_int), intent(out)              :: error_code

    ! variables
    type(profile_t), pointer :: f_profile

    call c_f_pointer(profile, f_profile)

    f_profile%mid_val_ = midpoint_values(1:num_midpoint_values)

  end subroutine internal_set_midpoint_values

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_layer_densities(profile, layer_densities, num_layer_densities, &
    error_code) bind(C, name="InternalSetLayerDensities")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double

    ! arguments
    type(c_ptr), value, intent(in)                :: profile
    real(kind=c_double), intent(in), dimension(*) :: layer_densities
    integer(kind=c_int), intent(in), value        :: num_layer_densities
    integer(kind=c_int), intent(out)              :: error_code

    ! variables
    type(profile_t), pointer :: f_profile

    call c_f_pointer(profile, f_profile)

    f_profile%layer_dens_ = layer_densities(1:num_layer_densities)
    f_profile%exo_layer_dens_ = layer_densities(num_layer_densities)

  end subroutine internal_set_layer_densities

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_exo_layer_density(profile, exo_layer_density, error_code) &
    bind(C, name="InternalSetExoLayerDensity")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double

    ! arguments
    type(c_ptr), value, intent(in) :: profile
    real(kind=c_double), intent(in) :: exo_layer_density
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_t), pointer :: f_profile

    call c_f_pointer(profile, f_profile)

    f_profile%exo_layer_dens_(size(f_profile%exo_layer_dens_)) = exo_layer_density

  end subroutine internal_set_exo_layer_density

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_calculate_exo_layer_density(profile, scale_height, error_code) &
    bind(C, name="InternalCalculateExoLayerDensity")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double

    ! arguments
    type(c_ptr), value, intent(in) :: profile
    real(kind=c_double), intent(in) :: scale_height ! [[m]
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_t), pointer :: f_profile

    call c_f_pointer(profile, f_profile)

    f_profile%exo_layer_dens_(size(f_profile%exo_layer_dens_)) = &
    f_profile%layer_dens_(size(f_profile%layer_dens_)) * scale_height * 100.0 ! m to cm

  end subroutine internal_calculate_exo_layer_density

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface
