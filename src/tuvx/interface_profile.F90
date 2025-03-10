! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_profile

  use tuvx_profile,        only : profile_t
  
  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_profile(profile_name, profile_name_length, units, &
    units_length, grid_updater_c, error_code) &
    bind(C, name="InternalCreateProfile") result(profile)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_char, c_loc, c_size_t, c_int
    use musica_string, only: string_t
    use tuvx_grid_from_host, only: grid_updater_t
    use tuvx_profile_from_host, only: profile_from_host_t

    ! arguments
    type(c_ptr) :: profile
    character(kind=c_char, len=1), dimension(*), intent(in) :: profile_name
    integer(kind=c_size_t), intent(in), value :: profile_name_length
    character(kind=c_char, len=1), dimension(*), intent(in) :: units
    integer(kind=c_size_t), intent(in), value :: units_length
    type(c_ptr), intent(in), value :: grid_updater_c
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_updater_t), pointer :: f_grid_updater
    type(profile_from_host_t), pointer :: f_profile
    type(string_t) :: f_name, f_units
    integer :: i

    allocate(character(len=profile_name_length) :: f_name%val_)
    do i = 1, profile_name_length
      f_name%val_(i:i) = profile_name(i)
    end do

    allocate(character(len=units_length) :: f_units%val_)
    do i = 1, units_length
      f_units%val_(i:i) = units(i)
    end do

    call c_f_pointer(grid_updater_c, f_grid_updater)
    f_profile => profile_from_host_t(f_name, f_units, &
                                     f_grid_updater%grid_%size())
    profile = c_loc(f_profile)

  end function internal_create_profile

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile_updater(profile, error_code) &
    bind(C, name="InternalGetProfileUpdater") result(updater)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_loc, c_int
    use tuvx_profile_from_host, only: profile_from_host_t, profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in) :: profile
    integer(kind=c_int), intent(out) :: error_code

    ! output
    type(c_ptr) :: updater

    ! variables
    type(profile_from_host_t), pointer :: f_profile
    type(profile_updater_t), pointer :: f_updater

    call c_f_pointer(profile, f_profile)
    allocate(f_updater, source = profile_updater_t(f_profile))
    updater = c_loc(f_updater)

  end function internal_get_profile_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_profile(profile, error_code) &
      bind(C, name="InternalDeleteProfile")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

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

  subroutine internal_delete_profile_updater(updater, error_code) &
      bind(C, name="InternalDeleteProfileUpdater")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in) :: updater
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater

    call c_f_pointer(updater, f_updater)
    if (associated(f_updater)) then
      deallocate(f_updater)
    end if

  end subroutine internal_delete_profile_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_edge_values(profile_updater, edge_values, &
      num_edge_values, error_code) bind(C, name="InternalSetEdgeValues")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: profile_updater
    type(c_ptr), value, intent(in)            :: edge_values
    integer(kind=c_size_t), intent(in), value :: num_edge_values
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_edge_values(:)

    call c_f_pointer(profile_updater, f_updater)
    call c_f_pointer(edge_values, f_edge_values, [num_edge_values])

    if (size(f_updater%profile_%edge_val_) /= num_edge_values) then
      error_code = 1
      return
    end if
    f_updater%profile_%edge_val_(:) = f_edge_values(:)
    f_updater%profile_%delta_val_(:) = f_edge_values(2:num_edge_values) - &
        f_edge_values(1:num_edge_values-1)

  end subroutine internal_set_edge_values

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_edge_values(profile_updater, edge_values, &
      num_edge_values, error_code) bind(C, name="InternalGetEdgeValues")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: profile_updater
    type(c_ptr), value, intent(in)            :: edge_values
    integer(kind=c_size_t), intent(in), value :: num_edge_values
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_edge_values(:)

    call c_f_pointer(profile_updater, f_updater)
    call c_f_pointer(edge_values, f_edge_values, [num_edge_values])

    if (size(f_updater%profile_%edge_val_) /= num_edge_values) then
      error_code = 1
      return
    end if
    f_edge_values(:) = f_updater%profile_%edge_val_(:)

  end subroutine internal_get_edge_values

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_midpoint_values(profile_updater, midpoint_values, &
      num_midpoint_values, error_code) bind(C, name="InternalSetMidpointValues")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: profile_updater
    type(c_ptr), value, intent(in)            :: midpoint_values
    integer(kind=c_size_t), intent(in), value :: num_midpoint_values
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_midpoint_values(:)

    call c_f_pointer(profile_updater, f_updater)
    call c_f_pointer(midpoint_values, f_midpoint_values, [num_midpoint_values])

    if (size(f_updater%profile_%mid_val_) /= num_midpoint_values) then
      error_code = 1
      return
    end if
    f_updater%profile_%mid_val_(:) = f_midpoint_values(:)

  end subroutine internal_set_midpoint_values

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_midpoint_values(profile_updater, midpoint_values, &
      num_midpoint_values, error_code) bind(C, name="InternalGetMidpointValues")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: profile_updater
    type(c_ptr), value, intent(in)            :: midpoint_values
    integer(kind=c_size_t), intent(in), value :: num_midpoint_values
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_midpoint_values(:)

    call c_f_pointer(profile_updater, f_updater)
    call c_f_pointer(midpoint_values, f_midpoint_values, [num_midpoint_values])

    if (size(f_updater%profile_%mid_val_) /= num_midpoint_values) then
      error_code = 1
      return
    end if
    f_midpoint_values(:) = f_updater%profile_%mid_val_(:)

  end subroutine internal_get_midpoint_values

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_layer_densities(profile_updater, layer_densities, &
      num_layer_densities, error_code) bind(C, name="InternalSetLayerDensities")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: profile_updater
    type(c_ptr), value, intent(in)            :: layer_densities
    integer(kind=c_size_t), intent(in), value :: num_layer_densities
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_layer_densities(:)

    call c_f_pointer(profile_updater, f_updater)
    call c_f_pointer(layer_densities, f_layer_densities, [num_layer_densities])

    if (size(f_updater%profile_%layer_dens_) /= num_layer_densities) then
      error_code = 1
      return
    end if

    f_updater%profile_%layer_dens_(:) = f_layer_densities(:)
    f_updater%profile_%exo_layer_dens_(1:num_layer_densities) = &
        f_layer_densities(:)
    f_updater%profile_%layer_dens_(num_layer_densities) = &
        f_updater%profile_%layer_dens_(num_layer_densities) + &
        f_updater%profile_%exo_layer_dens_(num_layer_densities+1)

  end subroutine internal_set_layer_densities

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_layer_densities(profile_updater, layer_densities, &
      num_layer_densities, error_code) bind(C, name="InternalGetLayerDensities")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: profile_updater
    type(c_ptr), value, intent(in)            :: layer_densities
    integer(kind=c_size_t), intent(in), value :: num_layer_densities
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_layer_densities(:)

    call c_f_pointer(profile_updater, f_updater)
    call c_f_pointer(layer_densities, f_layer_densities, [num_layer_densities])

    if (size(f_updater%profile_%layer_dens_) /= num_layer_densities) then
      error_code = 1
      return
    end if
    f_layer_densities(:) = f_updater%profile_%layer_dens_(:)

  end subroutine internal_get_layer_densities

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_exo_layer_density(profile_updater, &
      exo_layer_density, error_code) bind(C, name="InternalSetExoLayerDensity")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr),         value, intent(in)  :: profile_updater
    real(kind=c_double), value, intent(in)  :: exo_layer_density
    integer(kind=c_int),        intent(out) :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater

    call c_f_pointer(profile_updater, f_updater)

    associate(ld => f_updater%profile_%layer_dens_, &
              eld => f_updater%profile_%exo_layer_dens_)
      eld(size(eld)) = real(exo_layer_density, kind=dk)
      ld(size(ld))   = eld(size(ld)) + real(exo_layer_density, kind=dk)
    end associate

  end subroutine internal_set_exo_layer_density

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_calculate_exo_layer_density(profile_updater, &
      scale_height, error_code) bind(C, name="InternalCalculateExoLayerDensity")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr),         value, intent(in)  :: profile_updater
    real(kind=c_double), value, intent(in)  :: scale_height ! [m]
    integer(kind=c_int),        intent(out) :: error_code

    ! variables
    type(profile_updater_t), pointer :: f_updater
    real(kind=dk) :: exo_layer_density

    call c_f_pointer(profile_updater, f_updater)

    associate(ld => f_updater%profile_%layer_dens_, &
              eld => f_updater%profile_%exo_layer_dens_)
      exo_layer_density = &
          eld(size(ld)) * real(scale_height, kind=dk) * 100.0_dk ! m to cm
      eld(size(eld)) = exo_layer_density
      ld(size(ld)) = eld(size(ld)) + exo_layer_density
    end associate

  end subroutine internal_calculate_exo_layer_density

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_exo_layer_density(profile_updater, error_code) &
      bind(C, name="InternalGetExoLayerDensity") result(exo_layer_density)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_double
    use musica_constants, only: dk => musica_dk
    use tuvx_profile_from_host, only: profile_updater_t

    ! arguments
    type(c_ptr), value,  intent(in)  :: profile_updater
    integer(kind=c_int), intent(out) :: error_code

    ! output
    real(kind=c_double) :: exo_layer_density

    ! variables
    type(profile_updater_t), pointer :: f_updater

    call c_f_pointer(profile_updater, f_updater)
    associate(eld => f_updater%profile_%exo_layer_dens_)
      exo_layer_density = real(eld(size(eld)), kind=c_double)
    end associate
    
  end function internal_get_exo_layer_density

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_profile
