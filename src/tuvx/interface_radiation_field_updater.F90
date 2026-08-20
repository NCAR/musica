! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface_radiation_field_updater

   use iso_c_binding, only : c_ptr, c_int, c_size_t

   implicit none

   private

   integer, parameter :: ERROR_NONE = 0

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_get_radiation_field_updater(tuvx, found, error_code) &
      result(updater_ptr) bind(C, name="InternalGetRadiationFieldUpdater")
      use iso_c_binding, only : c_ptr, c_f_pointer, c_loc, c_null_ptr
      use tuvx_core,             only : core_t
      use tuvx_solver_from_host, only : radiation_field_updater_t

      ! arguments
      type(c_ptr)                      :: updater_ptr
      type(c_ptr), value,  intent(in)  :: tuvx
      integer(kind=c_int), intent(out) :: found
      integer(kind=c_int), intent(out) :: error_code

      ! variables
      type(core_t),                     pointer :: core
      type(radiation_field_updater_t),  pointer :: f_updater
      logical :: f_found

      error_code = ERROR_NONE
      call c_f_pointer(tuvx, core)

      allocate(f_updater)
      f_updater = core%get_radiation_field_updater(found = f_found)

      if (f_found) then
         found = 1
         updater_ptr = c_loc(f_updater)
      else
         found = 0
         deallocate(f_updater)
         updater_ptr = c_null_ptr
      end if

   end function internal_get_radiation_field_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_delete_radiation_field_updater(updater, error_code) &
      bind(C, name="InternalDeleteRadiationFieldUpdater")
      use iso_c_binding, only : c_ptr, c_f_pointer
      use tuvx_solver_from_host, only : radiation_field_updater_t

      ! arguments
      type(c_ptr), value,  intent(in)  :: updater
      integer(kind=c_int), intent(out) :: error_code

      ! variables
      type(radiation_field_updater_t), pointer :: f_updater

      error_code = ERROR_NONE
      call c_f_pointer(updater, f_updater)
      deallocate(f_updater)

   end subroutine internal_delete_radiation_field_updater

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_update_radiation_field(updater, direct_actinic_flux, &
      upward_actinic_flux, downward_actinic_flux, direct_irradiance,        &
      upward_irradiance, downward_irradiance, num_vertical_interfaces,      &
      num_wavelength_bins, error_code)                                      &
      bind(C, name="InternalUpdateRadiationField")
      use iso_c_binding, only : c_ptr, c_f_pointer, c_associated
      use musica_constants,     only : dk => musica_dk
      use tuvx_solver_from_host, only : radiation_field_updater_t

      ! arguments
      type(c_ptr), value,  intent(in)  :: updater
      type(c_ptr), value,  intent(in)  :: direct_actinic_flux
      type(c_ptr), value,  intent(in)  :: upward_actinic_flux
      type(c_ptr), value,  intent(in)  :: downward_actinic_flux
      type(c_ptr), value,  intent(in)  :: direct_irradiance   ! optional; null if omitted
      type(c_ptr), value,  intent(in)  :: upward_irradiance   ! optional; null if omitted
      type(c_ptr), value,  intent(in)  :: downward_irradiance ! optional; null if omitted
      integer(kind=c_size_t), value, intent(in) :: num_vertical_interfaces
      integer(kind=c_size_t), value, intent(in) :: num_wavelength_bins
      integer(kind=c_int), intent(out) :: error_code

      ! variables
      type(radiation_field_updater_t), pointer :: f_updater
      real(kind=dk), pointer :: fdr(:,:), fup(:,:), fdn(:,:)
      real(kind=dk), pointer :: edr(:,:), eup(:,:), edn(:,:)
      integer :: n_int, n_bin

      error_code = ERROR_NONE
      n_int = int(num_vertical_interfaces)
      n_bin = int(num_wavelength_bins)

      call c_f_pointer(updater, f_updater)
      call c_f_pointer(direct_actinic_flux,   fdr, [n_int, n_bin])
      call c_f_pointer(upward_actinic_flux,   fup, [n_int, n_bin])
      call c_f_pointer(downward_actinic_flux, fdn, [n_int, n_bin])

      ! The three irradiance arrays are optional on both sides: a disassociated
      ! pointer actual argument matched to update()'s optional, non-pointer,
      ! assumed-shape dummy argument is treated as an absent argument, so
      ! leaving edr/eup/edn unassociated when the caller omitted them is
      ! sufficient -- no separate branch per combination is needed.
      nullify(edr, eup, edn)
      if (c_associated(direct_irradiance))   call c_f_pointer(direct_irradiance,   edr, [n_int, n_bin])
      if (c_associated(upward_irradiance))   call c_f_pointer(upward_irradiance,   eup, [n_int, n_bin])
      if (c_associated(downward_irradiance)) call c_f_pointer(downward_irradiance, edn, [n_int, n_bin])

      call f_updater%update( &
         direct_actinic_flux   = fdr, &
         upward_actinic_flux   = fup, &
         downward_actinic_flux = fdn, &
         direct_irradiance     = edr, &
         upward_irradiance     = eup, &
         downward_irradiance   = edn)

   end subroutine internal_update_radiation_field

end module tuvx_interface_radiation_field_updater
