! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_radiator
  use tuvx_radiator,           only : radiator_t

  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_radiator(radiator_name, radiator_name_length, height_grid, &
    wavelength_grid, error_code) result(radiator) &
    bind(C, name="InternalCreateRadiator")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_char, c_loc, c_size_t, c_int
    use musica_string, only: string_t

    use tuvx_radiator_from_host, only: radiator_from_host_t
    use tuvx_grid,                     only : grid_t

    ! arguments
    type(c_ptr)                                             :: radiator
    character(kind=c_char, len=1), dimension(*), intent(in) :: radiator_name
    integer(kind=c_size_t), intent(in), value               :: radiator_name_length
    type(c_ptr), intent(in), value                          :: height_grid
    type(c_ptr), intent(in), value                          :: wavelength_grid
    integer(kind=c_int), intent(out)                        :: error_code
    
    ! variables
    type(radiator_from_host_t), pointer :: f_radiator
    type(string_t)                      :: f_name
    type(grid_t)                        :: f_height_grid
    type(grid_t)                        :: f_wavelength_grid

    allocate(character(len=radiator_name_length) :: f_name%val_)
    do i = 1, radiator_name_length
      f_name%val_(i:i) = radiator_name(i)
    end do

    call c_f_pointer(height_grid, f_height_grid)
    call c_f_pointer(wavelength_grid, f_wavelength_grid)
    f_radiator => radiator_from_host_t(f_name, f_height_grid, f_wavelength_grid)
    radiator = c_loc(f_radiator)
  
  end function internal_create_radiator

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_radiator(radiator, error_code) &
    bind(C, name="InternalDeleteRadiator")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

    ! arguments
    type(c_ptr), value, intent(in)   :: radiator
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(radiator_t), pointer :: f_radiator

    call c_f_pointer(radiator, f_radiator)
    if (associated(f_radiator)) then
      deallocate(f_radiator)
    end if

  end subroutine internal_delete_radiator

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_radiator_updater(radiator, error_code) &
    bind(C, name="InternalGetRadiatorUpdater") result(updater)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_loc, c_int
    use tuvx_radiator_from_host, only: radiator_from_host_t, radiator_updater_t

    ! arguments
    type(c_ptr), value, intent(in)   :: radiator
    integer(kind=c_int), intent(out) :: error_code

    ! output
    type(c_ptr) :: updater

    ! variables
    type(radiator_from_host_t), pointer :: f_radiator
    type(radiator_updater_t), pointer   :: f_updater

    call c_f_pointer(radiator, f_radiator)
    allocate(f_updater, source = radiator_updater_t(f_radiator))
    updater = c_loc(f_updater)

  end function internal_get_radiator_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_radiator_updater(updater, error_code) &
    bind(C, name="InternalDeleteRadiatorUpdater")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr), value, intent(in)   :: updater
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(grid_radiator_t), pointer :: f_updater

    call c_f_pointer(updater, f_updater)
    if (associated(f_updater)) then
      deallocate(f_updater)
    end if

  end subroutine internal_delete_radiator_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_optical_depths(radiator_updater, optical_depths, num_vertical_layers, & 
      num_wavelength_bins, error_code) &
      bind(C, name="InternalSetOpticalDepths")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t
  
    ! arguments
    type(c_ptr), value, intent(in)            :: radiator_updater
    type(c_ptr), value, intent(in)            :: optical_depths
    integer(kind=c_size_t), intent(in), value :: num_vertical_layers
    integer(kind=c_size_t), intent(in), value :: num_wavelength_bins
    integer(kind=c_int), intent(out)          :: error_code
  
    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_optical_depths(:,:)
  
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(optical_depths, f_optical_depths, [num_vertical_layers, num_wavelength_bins])

    if ((size(f_updater%radiator_%state_%layer_OD_, 1) /= num_vertical_layers) .or. &
        (size(f_updater%radiator_%state_%layer_OD_, 2) /= num_wavelength_bins)) then
      error_code = 1
      return
    end if
    f_updater%radiator_%state_%layer_OD_(:,:) = f_optical_depths(:,:)

  end subroutine internal_set_optical_depths

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_optical_depths(radiator_updater, optical_depths, num_vertical_layers, & 
      num_wavelength_bins, error_code) &
      bind(C, name="InternalGetOpticalDepths")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: radiator_updater
    type(c_ptr), value, intent(in)            :: optical_depths
    integer(kind=c_size_t), intent(in), value :: num_vertical_layers
    integer(kind=c_size_t), intent(in), value :: num_wavelength_bins
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_optical_depths(:,:)
  
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(optical_depths, f_optical_depths, [num_vertical_layers, num_wavelength_bins])

    if ((size(f_updater%radiator_%state_%layer_OD_, 1) /= num_vertical_layers) .or. &
        (size(f_updater%radiator_%state_%layer_OD_, 2) /= num_wavelength_bins)) then
      error_code = 1
      return
    end if
    f_optical_depths(:,:) = f_updater%radiator_%state_%layer_OD_(:,:)

  end subroutine internal_get_optical_depths

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_single_scattering_albedos(radiator_updater, single_scattering_albedos, & 
      num_vertical_layers, num_wavelength_bins, error_code) &
      bind(C, name="InternalSetSingleScatteringAlbedos")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: radiator_updater
    type(c_ptr), value, intent(in)            :: single_scattering_albedos
    integer(kind=c_size_t), intent(in), value :: num_vertical_layers
    integer(kind=c_size_t), intent(in), value :: num_wavelength_bins
    integer(kind=c_int), intent(out)          :: error_code
  
    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_single_scattering_albedos(:,:)
  
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(single_scattering_albedos, f_single_scattering_albedos, [num_vertical_layers, num_wavelength_bins])

    if ((size(f_updater%radiator_%state_%layer_SSA_, 1) /= num_vertical_layers) .or. &
        (size(f_updater%radiator_%state_%layer_SSA_, 2) /= num_wavelength_bins)) then
      error_code = 1
      return
    end if
    f_updater%radiator_%state_%layer_SSA_(:,:) = f_single_scattering_albedos(:,:)

  end subroutine internal_set_single_scattering_albedos

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_single_scattering_albedos(radiator_updater, single_scattering_albedos, & 
      num_vertical_layers, num_wavelength_bins, error_code) &
      bind(C, name="InternalGetSingleScatteringAlbedos")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: radiator_updater
    type(c_ptr), value, intent(in)            :: single_scattering_albedos
    integer(kind=c_size_t), intent(in), value :: num_vertical_layers
    integer(kind=c_size_t), intent(in), value :: num_wavelength_bins
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_single_scattering_albedos(:,:)

    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(single_scattering_albedos, f_single_scattering_albedos, [num_vertical_layers, num_wavelength_bins])

    if ((size(f_updater%radiator_%state_%layer_SSA_, 1) /= num_vertical_layers) .or. &
        (size(f_updater%radiator_%state_%layer_SSA_, 2) /= num_wavelength_bins)) then
      error_code = 1
      return
    end if
    f_single_scattering_albedos(:,:) = f_updater%radiator_%state_%layer_SSA_(:,:)

  end subroutine internal_get_single_scattering_albedos

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_asymmetry_factor(radiator_updater, asymmetry_factor, num_vertical_layers, & 
      num_wavelength_bins, num_streams, error_code) &
      bind(C, name="InternalSetAsymmetryFactor")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr), value, intent(in)            :: radiator_updater
    type(c_ptr), value, intent(in)            :: asymmetry_factor
    integer(kind=c_size_t), intent(in), value :: num_vertical_layers
    integer(kind=c_size_t), intent(in), value :: num_wavelength_bins
    integer(kind=c_size_t), intent(in), value :: num_streams
    integer(kind=c_int), intent(out)          :: error_code

    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk), pointer :: f_asymmetry_factor(:,:,:)

    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(asymmetry_factor, f_asymmetry_factor, [num_vertical_layers, num_wavelength_bins, num_streams])

    if ((size(f_updater%radiator_%state_%layer_SSA_, 1) /= num_vertical_layers) .or. &
        (size(f_updater%radiator_%state_%layer_SSA_, 2) /= num_wavelength_bins) .or. &
        (size(f_updater%radiator_%state_%layer_G_, 3) /= num_streams)) then
      error_code = 1
      return
    end if
    f_updater%radiator_%state_%layer_G_(:,:,:) = f_asymmetry_factor(:,:,:)

  end subroutine internal_set_asymmetry_factor

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_radiator