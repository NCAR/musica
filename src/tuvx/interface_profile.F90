! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_profile

  use iso_c_binding,       only : c_ptr, c_loc, c_int, c_size_t, c_char
  use tuvx_profile,        only : profile_t
  use musica_tuvx_util,    only : to_f_string, string_t_c
  use musica_string,       only : string_t
  
  implicit none

  private

  contains

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
    real(kind=c_double), intent(in) :: scale_height ! [m]
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(profile_t), pointer :: f_profile

    call c_f_pointer(profile, f_profile)

    f_profile%exo_layer_dens_(size(f_profile%exo_layer_dens_)) = &
    f_profile%layer_dens_(size(f_profile%layer_dens_)) * scale_height * 100.0 ! m to cm

  end subroutine internal_calculate_exo_layer_density

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_profile
