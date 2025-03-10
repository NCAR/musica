! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface

use iso_c_binding,           only : c_ptr, c_loc, c_int, c_size_t, c_char
use tuvx_core,               only : core_t
use tuvx_grid,               only : grid_t
use tuvx_grid_warehouse,     only : grid_warehouse_t
use tuvx_profile_warehouse,  only : profile_warehouse_t
use tuvx_radiator_warehouse, only : radiator_warehouse_t
use musica_string,           only : string_t

implicit none

private

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_tuvx(c_config_path, config_path_length, &
      c_grid_map, c_profile_map, c_radiator_map, number_of_layers, error_code) &
      bind(C, name="InternalCreateTuvx")
    use iso_c_binding, only: c_ptr, c_f_pointer

    ! arguments
    character(kind=c_char), dimension(*), intent(in)  :: c_config_path
    integer(kind=c_size_t), value                     :: config_path_length
    type(c_ptr), value,                   intent(in)  :: c_grid_map
    type(c_ptr), value,                   intent(in)  :: c_profile_map
    type(c_ptr), value,                   intent(in)  :: c_radiator_map
    integer(kind=c_int),                  intent(out) :: number_of_layers
    integer(kind=c_int),                  intent(out) :: error_code
    type(c_ptr)                                       :: internal_create_tuvx

    ! local variables
    character(len=:), allocatable       :: f_config_path
    type(core_t), pointer               :: core
    type(string_t)                      :: musica_config_path
    type(grid_warehouse_t),     pointer :: grid_map
    type(profile_warehouse_t),  pointer :: profile_map
    type(radiator_warehouse_t), pointer :: radiator_map
    type(grid_t), pointer               :: height_grid
    integer                             :: i

    allocate(character(len=config_path_length) :: f_config_path)
    do i = 1, config_path_length
      f_config_path(i:i) = c_config_path(i)
    end do

    musica_config_path = string_t(f_config_path)
    call c_f_pointer(c_grid_map, grid_map)
    call c_f_pointer(c_profile_map, profile_map)
    call c_f_pointer(c_radiator_map, radiator_map)
    core => core_t(musica_config_path, grids = grid_map, &
        profiles = profile_map, radiators = radiator_map)
    height_grid => core%get_grid("height", "km")
    number_of_layers = height_grid%ncells_
    deallocate(height_grid)

    deallocate(f_config_path)
    error_code = 0

    internal_create_tuvx = c_loc(core)

  end function internal_create_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_tuvx(tuvx, error_code) &
      bind(C, name="InternalDeleteTuvx")
    use iso_c_binding, only: c_ptr, c_f_pointer

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! local variables
    type(core_t), pointer :: core
  
    call c_f_pointer(tuvx, core)
    if (associated(core)) then
      deallocate(core)
    end if
  end subroutine internal_delete_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_grid_map(tuvx, error_code) result(grid_map_ptr) &
      bind(C, name="InternalGetGridMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
  
    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code
  
    ! result
    type(c_ptr) :: grid_map_ptr
  
    ! variables
    type(core_t),           pointer :: core
    type(grid_warehouse_t), pointer :: grid_warehouse
  
    call c_f_pointer(tuvx, core)
    grid_warehouse => core%get_grid_warehouse()
  
    grid_map_ptr = c_loc(grid_warehouse)

  end function internal_get_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile_map(tuvx, error_code) result(profile_map_ptr) &
      bind(C, name="InternalGetProfileMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: profile_map_ptr

    ! variables
    type(core_t),              pointer :: core
    type(profile_warehouse_t), pointer :: profile_warehouse

    call c_f_pointer(tuvx, core)
    profile_warehouse => core%get_profile_warehouse()
    
    profile_map_ptr = c_loc(profile_warehouse)

  end function internal_get_profile_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_radiator_map(tuvx, error_code) result(radiator_map_ptr) &
      bind(C, name="InternalGetRadiatorMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: radiator_map_ptr

    ! variables
    type(core_t),               pointer :: core
    type(radiator_warehouse_t), pointer :: radiator_warehouse

    call c_f_pointer(tuvx, core)
    radiator_warehouse => core%get_radiator_warehouse()

    radiator_map_ptr = c_loc(radiator_warehouse)

  end function internal_get_radiator_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_photolysis_rate_constants_ordering(tuvx, error_code) &
      result(photolysis_rate_constant_ordering) &
      bind(C, name="InternalGetPhotolysisRateConstantsOrdering")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_interface_util, only: create_string_t_c, mappings_t_c, &
                                   mapping_t_c, allocate_mappings_c

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(mappings_t_c) :: photolysis_rate_constant_ordering

    ! variables
    type(core_t), pointer :: core
    type(string_t), allocatable :: labels(:)
    type(mapping_t_c), pointer :: mappings(:)
    type(c_ptr) :: mappings_ptr
    integer :: i, n_labels

    error_code = 0
    call c_f_pointer(tuvx, core)

    labels = core%photolysis_reaction_labels()
    n_labels = size(labels)
    mappings_ptr = allocate_mappings_c(int(n_labels, kind=c_size_t))
    call c_f_pointer(mappings_ptr, mappings, [ n_labels ])
    do i = 1, n_labels
      mappings(i)%name_ = create_string_t_c(labels(i)%val_)
      mappings(i)%index_ = int(i-1, kind=c_size_t)
    end do

    photolysis_rate_constant_ordering%mappings_ = c_loc(mappings)
    photolysis_rate_constant_ordering%size_ = n_labels

  end function internal_get_photolysis_rate_constants_ordering

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_heating_rates_ordering(tuvx, error_code) &
      result(heating_rates_ordering) &
      bind(C, name="InternalGetHeatingRatesOrdering")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_interface_util, only: create_string_t_c, mappings_t_c, &
                                   mapping_t_c, allocate_mappings_c

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(mappings_t_c) :: heating_rates_ordering

    ! variables
    type(core_t), pointer :: core
    type(string_t), allocatable :: labels(:)
    type(mapping_t_c), pointer :: mappings(:)
    type(c_ptr) :: mappings_ptr
    integer :: i, n_labels

    error_code = 0
    call c_f_pointer(tuvx, core)

    labels = core%heating_rate_labels()
    n_labels = size(labels)
    mappings_ptr = allocate_mappings_c(int(n_labels, kind=c_size_t))
    call c_f_pointer(mappings_ptr, mappings, [ n_labels ])
    do i = 1, n_labels
      mappings(i)%name_ = create_string_t_c(labels(i)%val_)
      mappings(i)%index_ = int(i-1, kind=c_size_t)
    end do

    heating_rates_ordering%mappings_ = c_loc(mappings)
    heating_rates_ordering%size_ = n_labels

  end function internal_get_heating_rates_ordering

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_run_tuvx(tuvx, number_of_layers, solar_zenith_angle, &
      earth_sun_distance, photolysis_rate_constants, heating_rates, error_code) &
      bind(C, name="InternalRunTuvx")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use musica_constants, only: dk => musica_dk

    ! arguments
    type(c_ptr),         value,  intent(in)  :: tuvx
    integer(kind=c_int), value,  intent(in)  :: number_of_layers
    real(kind=dk),       value,  intent(in)  :: solar_zenith_angle         ! degrees
    real(kind=dk),       value,  intent(in)  :: earth_sun_distance         ! AU
    type(c_ptr),         value,  intent(in)  :: photolysis_rate_constants  ! s^-1 (layer, reaction)
    type(c_ptr),         value,  intent(in)  :: heating_rates              ! K s^-1 (layer, reaction)
    integer(kind=c_int),         intent(out) :: error_code

    ! variables
    type(core_t), pointer :: core
    real(kind=dk), pointer :: photo_rates(:,:), heat_rates(:,:)

    call c_f_pointer(tuvx, core)
    call c_f_pointer(photolysis_rate_constants, photo_rates, &
                     [number_of_layers + 1, core%number_of_photolysis_reactions()])
    call c_f_pointer(heating_rates, heat_rates, &
                     [number_of_layers + 1, core%number_of_heating_rates()])
    call core%run(solar_zenith_angle, earth_sun_distance, &
                  photolysis_rate_constants = photo_rates, &
                  heating_rates = heat_rates, &
                  diagnostic_label = "musica_tuvx_interface")
    error_code = 0

  end subroutine internal_run_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface