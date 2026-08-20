! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! STUB. This file exists only so that TUVX::GetRadiationFieldUpdater links; it
! does not yet bridge to tuv-x's real radiation_field_updater_t. Every call
! reports "no host-updatable solver found", which is honest given nothing
! wires up the real check yet. The functions below are replaced with a real
! bridge to tuv-x's core_t%get_radiation_field_updater and
! radiation_field_updater_t%update in a follow-up change.
module tuvx_interface_radiation_field_updater

   use iso_c_binding, only: c_ptr, c_null_ptr, c_int, c_double, c_size_t

   implicit none

   private

   integer, parameter :: ERROR_NONE = 0
   integer, parameter :: ERROR_NOT_IMPLEMENTED = 1

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_get_radiation_field_updater(tuvx, found, error_code) &
      bind(C, name="InternalGetRadiationFieldUpdater") result(updater)
      ! arguments
      type(c_ptr) :: updater
      type(c_ptr), intent(in), value :: tuvx
      integer(kind=c_int), intent(out) :: found
      integer(kind=c_int), intent(out) :: error_code

      updater = c_null_ptr
      found = 0
      error_code = ERROR_NONE

   end function internal_get_radiation_field_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_delete_radiation_field_updater(updater, error_code) &
      bind(C, name="InternalDeleteRadiationFieldUpdater")
      ! arguments
      type(c_ptr), intent(in), value :: updater
      integer(kind=c_int), intent(out) :: error_code

      error_code = ERROR_NONE

   end subroutine internal_delete_radiation_field_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_update_radiation_field(updater, direct_actinic_flux, &
      upward_actinic_flux, downward_actinic_flux, direct_irradiance,        &
      upward_irradiance, downward_irradiance, has_direct_irradiance,        &
      has_upward_irradiance, has_downward_irradiance,                      &
      num_vertical_interfaces, num_wavelength_bins, error_code)             &
      bind(C, name="InternalUpdateRadiationField")
      ! arguments
      type(c_ptr), intent(in), value :: updater
      type(c_ptr), intent(in), value :: direct_actinic_flux
      type(c_ptr), intent(in), value :: upward_actinic_flux
      type(c_ptr), intent(in), value :: downward_actinic_flux
      type(c_ptr), intent(in), value :: direct_irradiance
      type(c_ptr), intent(in), value :: upward_irradiance
      type(c_ptr), intent(in), value :: downward_irradiance
      integer(kind=c_int), intent(in), value :: has_direct_irradiance
      integer(kind=c_int), intent(in), value :: has_upward_irradiance
      integer(kind=c_int), intent(in), value :: has_downward_irradiance
      integer(kind=c_size_t), intent(in), value :: num_vertical_interfaces
      integer(kind=c_size_t), intent(in), value :: num_wavelength_bins
      integer(kind=c_int), intent(out) :: error_code

      ! Unreachable while internal_get_radiation_field_updater always
      ! reports "not found": the C++ layer never hands out a
      ! RadiationFieldUpdater to call Update on in that case.
      error_code = ERROR_NOT_IMPLEMENTED

   end subroutine internal_update_radiation_field

end module tuvx_interface_radiation_field_updater
