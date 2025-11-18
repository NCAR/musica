! Copyright (C) 2023-2025 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_radiator
  use tuvx_radiator,           only : radiator_t

  implicit none

  private

  integer, parameter :: ERROR_NONE = 0
  integer, parameter :: ERROR_RADIATOR_DIM_MISMATCH = 3201
  integer, parameter :: ERROR_UNALLOCATED_RADIATOR = 3202
  integer, parameter :: ERROR_UNALLOCATED_RADIATOR_UPDATER = 3203

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_radiator(radiator_name, radiator_name_length, &
      height_grid_updater_c, wavelength_grid_updater_c, error_code)      &
      result(radiator) bind(C, name="InternalCreateRadiator")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_char, c_loc, c_size_t, c_int
    use musica_string, only: string_t
    use tuvx_radiator_from_host, only: radiator_from_host_t
    use tuvx_grid_from_host, only: grid_updater_t

    ! arguments
    type(c_ptr)                                  :: radiator
    character(kind=c_char, len=1), dimension(*), intent(in) :: radiator_name
    integer(kind=c_size_t), value, intent(in)    :: radiator_name_length
    type(c_ptr),            value, intent(in)    :: height_grid_updater_c
    type(c_ptr),            value, intent(in)    :: wavelength_grid_updater_c
    integer(kind=c_int),           intent(out)   :: error_code
    
    ! variables
    type(radiator_from_host_t), pointer :: f_radiator
    type(string_t)                      :: f_name
    type(grid_updater_t),       pointer :: f_height_grid_updater
    type(grid_updater_t),       pointer :: f_wavelength_grid_updater
    integer                             :: i

    error_code = ERROR_NONE
    allocate(character(len=radiator_name_length) :: f_name%val_)
    do i = 1, radiator_name_length
      f_name%val_(i:i) = radiator_name(i)
    end do

    call c_f_pointer(height_grid_updater_c, f_height_grid_updater)
    call c_f_pointer(wavelength_grid_updater_c, f_wavelength_grid_updater)
    f_radiator => radiator_from_host_t(f_name, f_height_grid_updater%grid_, &
                                      f_wavelength_grid_updater%grid_)
    radiator = c_loc(f_radiator)
  
  end function internal_create_radiator

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_radiator(radiator, error_code) &
      bind(C, name="InternalDeleteRadiator")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

    ! arguments
    type(c_ptr), value,  intent(in)  :: radiator
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(radiator_t), pointer :: f_radiator

    error_code = ERROR_NONE
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
    type(c_ptr), value,  intent(in)  :: radiator
    integer(kind=c_int), intent(out) :: error_code

    ! output
    type(c_ptr) :: updater

    ! variables
    type(radiator_from_host_t), pointer :: f_radiator
    type(radiator_updater_t),   pointer :: f_updater

    error_code = ERROR_NONE
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
    type(c_ptr), value,  intent(in)  :: updater
    integer(kind=c_int), intent(out) :: error_code

    ! variables
    type(radiator_updater_t), pointer :: f_updater

    error_code = ERROR_NONE
    call c_f_pointer(updater, f_updater)
    if (associated(f_updater)) then
      deallocate(f_updater)
    end if

  end subroutine internal_delete_radiator_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_radiator_name(updater, name, error) &
      bind(C, name="InternalGetRadiatorName")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_char, c_size_t, c_int, &
                             c_loc
    use tuvx_interface_util, only: string_t_c, create_string_t_c
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr), value,  intent(in)  :: updater
    type(string_t_c), intent(out) :: name
    integer(kind=c_int), intent(out) :: error

    ! variables
    type(radiator_updater_t), pointer :: f_updater

    error = ERROR_NONE
    call c_f_pointer(updater, f_updater)
    name = create_string_t_c(f_updater%radiator_%handle_%val_)

  end subroutine internal_get_radiator_name

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_optical_depths(radiator_updater, optical_depths, &
      num_vertical_layers, num_wavelength_bins, error_code) &
      bind(C, name="InternalSetOpticalDepths")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t
  
    ! arguments
    type(c_ptr),            value, intent(in)  :: radiator_updater
    type(c_ptr),            value, intent(in)  :: optical_depths
    integer(kind=c_size_t), value, intent(in)  :: num_vertical_layers
    integer(kind=c_size_t), value, intent(in)  :: num_wavelength_bins
    integer(kind=c_int),           intent(out) :: error_code
  
    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk),            pointer :: f_optical_depths(:,:)

    error_code = ERROR_NONE
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(optical_depths, f_optical_depths, &
        [num_vertical_layers, num_wavelength_bins])

    if (.not. allocated(f_updater%radiator_%state_%layer_OD_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR
      return
    end if
    if ((size(f_updater%radiator_%state_%layer_OD_, 1) /= num_vertical_layers) &
      .or. (size(f_updater%radiator_%state_%layer_OD_, 2) /= num_wavelength_bins)) &
    then
      error_code = ERROR_RADIATOR_DIM_MISMATCH
      return
    end if
    f_updater%radiator_%state_%layer_OD_(:,:) = f_optical_depths(:,:)

  end subroutine internal_set_optical_depths

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_optical_depths(radiator_updater, optical_depths, &
      num_vertical_layers, num_wavelength_bins, error_code) &
      bind(C, name="InternalGetOpticalDepths")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr),            value, intent(in)  :: radiator_updater
    type(c_ptr),            value, intent(in)  :: optical_depths
    integer(kind=c_size_t), value, intent(in)  :: num_vertical_layers
    integer(kind=c_size_t), value, intent(in)  :: num_wavelength_bins
    integer(kind=c_int),           intent(out) :: error_code

    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk),            pointer :: f_optical_depths(:,:)

    error_code = ERROR_NONE
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(optical_depths, f_optical_depths, &
          [num_vertical_layers, num_wavelength_bins])

    if (.not. allocated(f_updater%radiator_%state_%layer_OD_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR
      return
    end if
    if ((size(f_updater%radiator_%state_%layer_OD_, 1) /= num_vertical_layers) &
      .or. (size(f_updater%radiator_%state_%layer_OD_, 2) /= num_wavelength_bins)) &
    then
      error_code = ERROR_RADIATOR_DIM_MISMATCH
      return
    end if
    f_optical_depths(:,:) = f_updater%radiator_%state_%layer_OD_(:,:)

  end subroutine internal_get_optical_depths

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_single_scattering_albedos(radiator_updater, &
      single_scattering_albedos, num_vertical_layers, num_wavelength_bins, &
      error_code) bind(C, name="InternalSetSingleScatteringAlbedos")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr),            value, intent(in)  :: radiator_updater
    type(c_ptr),            value, intent(in)  :: single_scattering_albedos
    integer(kind=c_size_t), value, intent(in)  :: num_vertical_layers
    integer(kind=c_size_t), value, intent(in)  :: num_wavelength_bins
    integer(kind=c_int),           intent(out) :: error_code
  
    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk),            pointer :: f_single_scattering_albedos(:,:)

    error_code = ERROR_NONE
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(single_scattering_albedos, f_single_scattering_albedos, &
          [num_vertical_layers, num_wavelength_bins])

    if (.not. allocated(f_updater%radiator_%state_%layer_SSA_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR
      return
    end if
    if ((size(f_updater%radiator_%state_%layer_SSA_, 1) /= num_vertical_layers) &
      .or. (size(f_updater%radiator_%state_%layer_SSA_, 2) /= num_wavelength_bins)) &
    then
      error_code = ERROR_RADIATOR_DIM_MISMATCH
      return
    end if
    f_updater%radiator_%state_%layer_SSA_(:,:) = f_single_scattering_albedos(:,:)

  end subroutine internal_set_single_scattering_albedos

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_single_scattering_albedos(radiator_updater, &
      single_scattering_albedos, num_vertical_layers, num_wavelength_bins, &
      error_code) bind(C, name="InternalGetSingleScatteringAlbedos")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr),            value, intent(in)  :: radiator_updater
    type(c_ptr),            value, intent(in)  :: single_scattering_albedos
    integer(kind=c_size_t), value, intent(in)  :: num_vertical_layers
    integer(kind=c_size_t), value, intent(in)  :: num_wavelength_bins
    integer(kind=c_int),           intent(out) :: error_code

    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk),            pointer :: f_single_scattering_albedos(:,:)

    error_code = ERROR_NONE
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(single_scattering_albedos, f_single_scattering_albedos, &
          [num_vertical_layers, num_wavelength_bins])

    if (.not. allocated(f_updater%radiator_%state_%layer_SSA_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR
      return
    end if
    if ((size(f_updater%radiator_%state_%layer_SSA_, 1) /= num_vertical_layers) &
      .or. (size(f_updater%radiator_%state_%layer_SSA_, 2) /= num_wavelength_bins)) &
    then
      error_code = ERROR_RADIATOR_DIM_MISMATCH
      return
    end if
    f_single_scattering_albedos(:,:) = f_updater%radiator_%state_%layer_SSA_(:,:)

  end subroutine internal_get_single_scattering_albedos

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_set_asymmetry_factors(radiator_updater, &
      asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, &
      error_code) bind(C, name="InternalSetAsymmetryFactors")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use musica_constants, only: dk => musica_dk
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr),            value, intent(in)  :: radiator_updater
    type(c_ptr),            value, intent(in)  :: asymmetry_factors
    integer(kind=c_size_t), value, intent(in)  :: num_vertical_layers
    integer(kind=c_size_t), value, intent(in)  :: num_wavelength_bins
    integer(kind=c_size_t), value, intent(in)  :: num_streams
    integer(kind=c_int),           intent(out) :: error_code

    ! variables
    type(radiator_updater_t), pointer :: f_updater
    real(kind=dk),            pointer :: f_asymmetry_factors(:,:,:)

    error_code = ERROR_NONE
    call c_f_pointer(radiator_updater, f_updater)
    call c_f_pointer(asymmetry_factors, f_asymmetry_factors, &
          [num_vertical_layers, num_wavelength_bins, num_streams])

    if (.not. allocated(f_updater%radiator_%state_%layer_G_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR
      return
    end if
    if ((size(f_updater%radiator_%state_%layer_G_, 1) /= num_vertical_layers) &
      .or. (size(f_updater%radiator_%state_%layer_G_, 2) /= num_wavelength_bins) &
      .or. (size(f_updater%radiator_%state_%layer_G_, 3) /= num_streams)) then
      error_code = ERROR_RADIATOR_DIM_MISMATCH
      return
    end if
    f_updater%radiator_%state_%layer_G_(:,:,:) = f_asymmetry_factors(:,:,:)

  end subroutine internal_set_asymmetry_factors

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_asymmetry_factors(radiator_updater, &
    asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, &
    error_code) bind(C, name="InternalGetAsymmetryFactors")
  use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
  use musica_constants, only: dk => musica_dk
  use tuvx_radiator_from_host, only: radiator_updater_t

  ! arguments
  type(c_ptr),            value, intent(in)  :: radiator_updater
  type(c_ptr),            value, intent(in)  :: asymmetry_factors
  integer(kind=c_size_t), value, intent(in)  :: num_vertical_layers
  integer(kind=c_size_t), value, intent(in)  :: num_wavelength_bins
  integer(kind=c_size_t), value, intent(in)  :: num_streams
  integer(kind=c_int),           intent(out) :: error_code

  ! variables
  type(radiator_updater_t), pointer :: f_updater
  real(kind=dk),            pointer :: f_asymmetry_factors(:,:,:)

  error_code = ERROR_NONE
  call c_f_pointer(radiator_updater, f_updater)
  call c_f_pointer(asymmetry_factors, f_asymmetry_factors, &
        [num_vertical_layers, num_wavelength_bins, num_streams])

  if (.not. allocated(f_updater%radiator_%state_%layer_G_)) then
    error_code = ERROR_UNALLOCATED_RADIATOR
    return
  end if
  if ((size(f_updater%radiator_%state_%layer_G_, 1) /= num_vertical_layers) &
    .or. (size(f_updater%radiator_%state_%layer_G_, 2) /= num_wavelength_bins) &
    .or. (size(f_updater%radiator_%state_%layer_G_, 3) /= num_streams)) then
    error_code = ERROR_RADIATOR_DIM_MISMATCH
    return
  end if
  f_asymmetry_factors(:,:,:) = f_updater%radiator_%state_%layer_G_(:,:,:)

end subroutine internal_get_asymmetry_factors

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_number_of_height_sections(radiator_updater, error_code) &
      bind(C, name="InternalGetRadiatorNumberOfHeightSections") &
      result(num_sections)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr),            value, intent(in)  :: radiator_updater
    integer(kind=c_int),           intent(out) :: error_code

    ! output
    integer(kind=c_size_t) :: num_sections

    ! variables
    type(radiator_updater_t), pointer :: f_updater

    error_code = ERROR_NONE
    call c_f_pointer(radiator_updater, f_updater)
    if (.not. allocated(f_updater%radiator_%state_%layer_OD_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR
      num_sections = -1
      return
    end if
    num_sections = size(f_updater%radiator_%state_%layer_OD_, 1)

  end function internal_get_number_of_height_sections

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_number_of_wavelength_sections(radiator_updater, &
      error_code) &
      bind(C, name="InternalGetRadiatorNumberOfWavelengthSections") &
      result(num_sections)
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int, c_size_t
    use tuvx_radiator_from_host, only: radiator_updater_t

    ! arguments
    type(c_ptr),            value, intent(in)  :: radiator_updater
    integer(kind=c_int),           intent(out) :: error_code

    ! output
    integer(kind=c_size_t) :: num_sections

    ! variables
    type(radiator_updater_t), pointer :: f_updater

    error_code = ERROR_NONE
    call c_f_pointer(radiator_updater, f_updater)
    if (.not. allocated(f_updater%radiator_%state_%layer_OD_)) then
      error_code = ERROR_UNALLOCATED_RADIATOR
      num_sections = -1
      return
    end if
    num_sections = size(f_updater%radiator_%state_%layer_OD_, 2)

  end function internal_get_number_of_wavelength_sections

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface_radiator