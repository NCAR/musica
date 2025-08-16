! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module carma_interface

   use iso_c_binding, only: c_int, c_double, c_ptr, c_char, c_null_char, &
      c_loc, c_f_pointer, c_associated, c_null_ptr, c_bool

   implicit none
   private

#include "musica/carma/error.hpp"

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   interface
      subroutine TransferCarmaOutputToCpp(output_data, nz, ny, nx, nbin, nelem, ngroup) &
         bind(C, name="TransferCarmaOutputToCpp")
         use iso_c_binding, only: c_int
         use carma_parameters_mod, only: carma_output_data_t
         type(carma_output_data_t), intent(in) :: output_data
         integer(c_int), value, intent(in) :: nz, ny, nx, nbin, nelem, ngroup
      end subroutine TransferCarmaOutputToCpp
   end interface

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_carma_version(version_ptr, version_length) &
      bind(C, name="InternalGetCarmaVersion")
      use iso_c_binding, only: c_ptr, c_int, c_f_pointer, c_null_char, c_loc, c_char
      use carma_version, only: get_carma_version

      ! arguments
      type(c_ptr),    intent(out) :: version_ptr
      integer(c_int), intent(out) :: version_length

      ! local variables
      character(len=:),       allocatable :: version_fortran
      character(kind=c_char), pointer     :: version_string_ptr(:)
      integer :: i

      version_fortran = get_carma_version()
      version_length = len_trim(version_fortran)

      ! Allocate and copy string
      allocate(version_string_ptr(version_length + 1))
      do i = 1, version_length
         version_string_ptr(i) = version_fortran(i:i)
      end do
      version_string_ptr(version_length + 1) = c_null_char

      version_ptr = c_loc(version_string_ptr)

   end subroutine internal_get_carma_version

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_free_carma_version(version_ptr, version_length) &
      bind(C, name="InternalFreeCarmaVersion")
      use iso_c_binding, only: c_char, c_ptr, c_int, c_associated, c_f_pointer

      type(c_ptr),    value, intent(in) :: version_ptr
      integer(c_int), value, intent(in) :: version_length
      character(kind=c_char), pointer :: version_string_ptr(:)

      ! Free the allocated version string pointer
      if (c_associated(version_ptr)) then
         call c_f_pointer(version_ptr, version_string_ptr, [version_length + 1])
         deallocate(version_string_ptr)
      end if

   end subroutine internal_free_carma_version

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_create_carma(params, rc) &
      bind(C, name="InternalCreateCarma") result(carma_cptr)
      use iso_c_binding, only: c_int, c_ptr, c_f_pointer
      use carma_parameters_mod, only: carma_parameters_t

      type(carma_parameters_t), intent(in)  :: params
      integer(c_int),           intent(out) :: rc
      type(c_ptr)                           :: carma_cptr

      rc = 0

      ! Create CARMA instance with the provided parameters
      call create_carma_instance(params, carma_cptr, rc)

   end function internal_create_carma

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_create_carma_state(carma_cptr, carma_params, carma_state_params, rc) &
      bind(C, name="InternalCreateCarmaState") result(carma_state_cptr)
      use iso_c_binding, only: c_ptr, c_int, c_double, c_loc, c_f_pointer
      use iso_fortran_env, only: real64
      use carma_types_mod, only: carma_type
      use carma_parameters_mod, only: carma_parameters_t, carma_state_parameter_t
      use carma_types_mod
      use carmastate_mod

      ! Arguments
      type(c_ptr),            value, intent(in)  :: carma_cptr
      type(carma_parameters_t),      intent(in)  :: carma_params
      type(carma_state_parameter_t), intent(in)  :: carma_state_params
      integer(c_int),                intent(out) :: rc

      ! Local variables
      type(carma_type), pointer      :: carma
      type(carmastate_type), pointer :: cstate
      real(kind=real64), pointer     :: vertical_center(:)
      real(kind=real64), pointer     :: vertical_levels(:)
      real(kind=real64), pointer     :: temperature(:)
      real(kind=real64), pointer     :: pressure(:)
      real(kind=real64), pointer     :: pressure_levels(:)
      real(kind=real64), pointer     :: specific_humidity_ptr(:)
      real(kind=real64), pointer     :: relative_humidity_ptr(:)
      real(kind=real64), pointer     :: original_temperature_ptr(:)
      real(kind=real64), pointer     :: radiative_intensity_ptr(:,:)
      real(kind=real64), allocatable :: specific_humidity(:)
      real(kind=real64), allocatable :: relative_humidity(:)
      real(kind=real64), allocatable :: original_temperature(:)
      real(kind=real64), allocatable :: radiative_intensity(:,:)
      type(c_ptr)                    :: carma_state_cptr
      integer :: alloc_stat

      rc = 0

      ! Create the CARMA instance
      allocate(cstate, stat=alloc_stat)
      if (alloc_stat /= 0) then
         rc = MUSICA_CARMA_ERROR_CODE_MEMORY_ALLOCATION
         return
      end if

      call c_f_pointer(carma_state_params%vertical_center, vertical_center, [carma_state_params%vertical_center_size])
      call c_f_pointer(carma_state_params%vertical_levels, vertical_levels, [carma_state_params%vertical_levels_size])
      call c_f_pointer(carma_state_params%temperature, temperature, [carma_state_params%temperature_size])
      call c_f_pointer(carma_state_params%pressure, pressure, [carma_state_params%pressure_size])
      call c_f_pointer(carma_state_params%pressure_levels, pressure_levels, [carma_state_params%pressure_levels_size])
      if (carma_state_params%specific_humidity_size > 0) then
         call c_f_pointer(carma_state_params%specific_humidity, specific_humidity_ptr, [carma_state_params%specific_humidity_size])
         allocate(specific_humidity(carma_state_params%specific_humidity_size))
         specific_humidity(:) = specific_humidity_ptr(:)
      end if
      if (carma_state_params%relative_humidity_size > 0) then
         call c_f_pointer(carma_state_params%relative_humidity, relative_humidity_ptr, [carma_state_params%relative_humidity_size])
         allocate(relative_humidity(carma_state_params%relative_humidity_size))
         relative_humidity(:) = relative_humidity_ptr(:)
      end if
      if (carma_state_params%original_temperature_size > 0) then
         call c_f_pointer(carma_state_params%original_temperature, original_temperature_ptr, [carma_state_params%original_temperature_size])
         allocate(original_temperature(carma_state_params%original_temperature_size))
         original_temperature(:) = original_temperature_ptr(:)
      end if
      if (carma_state_params%radiative_intensity_dim_1_size > 0 .and. &
         carma_state_params%radiative_intensity_dim_2_size > 0) then
         call c_f_pointer(carma_state_params%radiative_intensity, radiative_intensity_ptr, [carma_state_params%radiative_intensity_dim_2_size, carma_state_params%radiative_intensity_dim_1_size])
         allocate(radiative_intensity(carma_state_params%radiative_intensity_dim_2_size, carma_state_params%radiative_intensity_dim_1_size))
         radiative_intensity(:,:) = radiative_intensity_ptr(:,:)
      end if

      if (c_associated(carma_cptr)) then
         call c_f_pointer(carma_cptr, carma)
         call CARMASTATE_Create( &
            cstate=cstate, &
            carma_ptr=carma, &
            time=carma_state_params%time, &
            dtime=carma_state_params%time_step, &
            nz=carma_params%nz, &
            igridv=carma_state_params%coordinates, &
            xc=carma_state_params%latitude, &
            yc=carma_state_params%longitude, &
            zc=vertical_center(:), &
            zl=vertical_levels(:), &
            p=pressure(:), &
            pl=pressure_levels(:), &
            t=temperature(:), &
            rc=rc, &
            qh2o=specific_humidity(:), &
            relhum=relative_humidity(:), &
            told=original_temperature(:), &
            radint=radiative_intensity(:,:) &
            )
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_CREATION_FAILED
            return
         end if

         ! Set the weight percents to zero to avoid uninitialized values
         ! Actual values can be set once the CARMASTATE_SetGas() function is implemented
         cstate%f_wtpct(:) = 0.0_real64
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
         return
      end if

      carma_state_cptr = c_loc(cstate)
   end function internal_create_carma_state

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_destroy_carma_state(carma_state_cptr, rc) &
      bind(C, name="InternalDestroyCarmaState")
      use iso_c_binding, only: c_ptr, c_int
      use carma_types_mod, only: carmastate_type
      use carmastate_mod, only: CARMASTATE_Destroy

      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      integer(c_int),        intent(out) :: rc

      type(carmastate_type), pointer :: cstate

      rc = 0

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         ! Clean up the carma state instance
         call CARMASTATE_Destroy(cstate, rc)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_DESTROY_FAILED
            return
         end if
         deallocate(cstate)
      end if
   end subroutine internal_destroy_carma_state

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_set_bin(carma_state_cptr, bin_index, element_index, &
      values_ptr, values_size, surface_mass, rc) bind(C, name="InternalSetBin")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use carmastate_mod, only: CARMASTATE_SetBin
      use carma_types_mod, only: carmastate_type
      use iso_fortran_env, only: real64

      ! Arguments
      type(c_ptr),       value, intent(in)  :: carma_state_cptr
      integer(c_int),    value, intent(in)  :: bin_index
      integer(c_int),    value, intent(in)  :: element_index
      type(c_ptr),       value              :: values_ptr
      integer(c_int),    value, intent(in)  :: values_size
      real(kind=real64), value, intent(in)  :: surface_mass
      integer(c_int),           intent(out) :: rc

      ! Local variables
      real(kind=real64), pointer :: values(:)
      type(carmastate_type), pointer :: cstate

      if (element_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (bin_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(values_ptr, values, [values_size])

         call CARMASTATE_SetBin( &
            cstate=cstate, &
            ielem=element_index, &
            ibin=bin_index, &
            mmr=values, &
            rc=rc, &
            surface=surface_mass)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_SET_FAILED
            return
         end if
      end if

   end subroutine internal_set_bin

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_set_detrain(carma_state_cptr, bin_index, element_index, values_ptr, values_size, rc) &
      bind(C, name="InternalSetDetrain")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use carmastate_mod, only: CARMASTATE_SetDetrain
      use carma_types_mod, only: carmastate_type
      use iso_fortran_env, only: real64

      ! Arguments
      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      integer(c_int), value, intent(in)  :: bin_index
      integer(c_int), value, intent(in)  :: element_index
      type(c_ptr),    value              :: values_ptr
      integer(c_int), value, intent(in)  :: values_size
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(kind=real64), pointer :: values(:)
      type(carmastate_type), pointer :: cstate

      if (element_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (bin_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(values_ptr, values, [values_size])

         call CARMASTATE_SetDetrain( &
             cstate=cstate, &
             ielem=element_index, &
             ibin=bin_index, &
             mmr=values, &
             rc=rc)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_SET_FAILED
            return
         end if
      end if

   end subroutine internal_set_detrain

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_set_gas(carma_state_cptr, gas_index, values_ptr, values_size, old_mmr_ptr, old_mmr_size, &
      gas_saturation_wrt_ice_ptr, gas_saturation_wrt_ice_size, &
      gas_saturation_wrt_liquid_ptr, gas_saturation_wrt_liquid_size, rc) &
      bind(C, name="InternalSetGas")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use carmastate_mod, only: CARMASTATE_SetGas
      use carma_types_mod, only: carmastate_type
      use iso_fortran_env, only: real64

      ! Arguments
      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      integer(c_int), value, intent(in)  :: gas_index
      type(c_ptr),    value              :: values_ptr
      integer(c_int), value, intent(in)  :: values_size
      type(c_ptr),    value              :: old_mmr_ptr
      integer(c_int), value, intent(in)  :: old_mmr_size
      type(c_ptr),    value              :: gas_saturation_wrt_ice_ptr
      integer(c_int), value, intent(in)  :: gas_saturation_wrt_ice_size
      type(c_ptr),    value              :: gas_saturation_wrt_liquid_ptr
      integer(c_int), value, intent(in)  :: gas_saturation_wrt_liquid_size
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(kind=real64), pointer :: values(:)
      real(kind=real64), allocatable :: old_mmr(:)
      real(kind=real64), allocatable :: gas_saturation_wrt_ice(:)
      real(kind=real64), allocatable :: gas_saturation_wrt_liquid(:)
      type(carmastate_type), pointer :: cstate
      integer :: index

      if (gas_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (old_mmr_size > 0) then
         allocate(old_mmr(old_mmr_size))
         call c_f_pointer(old_mmr_ptr, values, [old_mmr_size])
         old_mmr = values
      end if

      if (gas_saturation_wrt_ice_size > 0) then
         allocate(gas_saturation_wrt_ice(gas_saturation_wrt_ice_size))
         call c_f_pointer(gas_saturation_wrt_ice_ptr, values, [gas_saturation_wrt_ice_size])
         gas_saturation_wrt_ice = values
      end if

      if (gas_saturation_wrt_liquid_size > 0) then
         allocate(gas_saturation_wrt_liquid(gas_saturation_wrt_liquid_size))
         call c_f_pointer(gas_saturation_wrt_liquid_ptr, values, [gas_saturation_wrt_liquid_size])
         gas_saturation_wrt_liquid = values
      end if

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(values_ptr, values, [values_size])

         call CARMASTATE_SetGas( &
             cstate=cstate, &
             igas=gas_index, &
             mmr=values, &
             rc=rc, &
             mmr_old=old_mmr, &
             satice_old=gas_saturation_wrt_ice, &
             satliq_old=gas_saturation_wrt_liquid)

         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_SET_FAILED
            return
         end if
      end if

   end subroutine internal_set_gas

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_state_statistics(carma_state_cptr, max_number_of_substeps, max_number_of_retries, &
      total_number_of_steps, total_number_of_substeps, total_number_of_retries, &
      xc, yc, z_substeps, nz, rc) &
      bind(C, name="InternalGetStepStatistics")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use iso_fortran_env, only: real64
      use carma_types_mod, only: carmastate_type
      use carmastate_mod, only: CARMASTATE_Get

      type(c_ptr), value, intent(in)     :: carma_state_cptr
      integer(c_int), intent(out)  :: max_number_of_substeps
      real(real64), intent(out)    :: max_number_of_retries
      real(real64), intent(out)    :: total_number_of_steps
      integer(c_int), intent(out)  :: total_number_of_substeps
      real(real64), intent(out)    :: total_number_of_retries
      real(real64), intent(out)    :: xc
      real(real64), intent(out)    :: yc
      type(c_ptr),    value              :: z_substeps
      integer(c_int), value, intent(in)  :: nz
      integer(c_int), intent(out)        :: rc

      ! Local variables
      type(carmastate_type), pointer :: cstate
      real(kind=real64), pointer      :: values(:)

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(z_substeps, values, [nz])

         ! Get state statistics
         if (allocated(cstate%f_zsubsteps)) then
            ! the z substeps are only allocated if substepping happens
            ! therefore we can only pass that argument to CARMASTATE_Get if it is allocated
            call CARMASTATE_Get(cstate, rc, &
               max_nsubstep=max_number_of_substeps, &
               max_nretry=max_number_of_retries, &
               nstep=total_number_of_steps, &
               nsubstep=total_number_of_substeps, &
               nretry=total_number_of_retries, &
               zsubsteps=values, &
               xc=xc, yc=yc)
         else
            ! -1 allows us to know to set this to None in the python wrapper, which will indicate
            ! that no z substeps were taken
            ! we have to remove the z_substeps argument in this call to not get a segfault
            values = -1
            call CARMASTATE_Get(cstate, rc, &
               max_nsubstep=max_number_of_substeps, &
               max_nretry=max_number_of_retries, &
               nstep=total_number_of_steps, &
               nsubstep=total_number_of_substeps, &
               nretry=total_number_of_retries, &
               xc=xc, yc=yc)
         end if

         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_GET_FAILED
            return
         end if
      end if

   end subroutine internal_get_state_statistics

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_bin(carma_state_cptr, bin_index, element_index, nz, mass_mixing_ratio_ptr, &
      number_mixing_ratio_ptr, number_density_ptr, nucleation_rate_ptr, wet_particle_radius_ptr, &
      wet_particle_density_ptr, dry_particle_density_ptr, particle_mass_on_surface, &
      sedimentation_flux, fall_velocity_ptr, deposition_velocity, delta_particle_temperature_ptr, &
      kappa_ptr, total_mass_mixing_ratio_ptr, rc) &
      bind(C, name="InternalGetBin")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use iso_fortran_env, only: real64
      use carma_types_mod, only: carmastate_type
      use carmastate_mod, only: CARMASTATE_GetBin

      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      integer(c_int), value, intent(in)  :: bin_index
      integer(c_int), value, intent(in)  :: element_index
      integer(c_int), value, intent(in)  :: nz
      type(c_ptr),    value              :: mass_mixing_ratio_ptr
      type(c_ptr),    value              :: number_mixing_ratio_ptr
      type(c_ptr),    value              :: number_density_ptr
      type(c_ptr),    value              :: nucleation_rate_ptr
      type(c_ptr),    value              :: wet_particle_radius_ptr
      type(c_ptr),    value              :: wet_particle_density_ptr
      type(c_ptr),    value              :: dry_particle_density_ptr
      real(real64),   intent(out)        :: particle_mass_on_surface
      real(real64),   intent(out)        :: sedimentation_flux
      type(c_ptr),    value              :: fall_velocity_ptr
      real(real64),   intent(out)        :: deposition_velocity
      type(c_ptr),    value              :: delta_particle_temperature_ptr
      type(c_ptr),    value              :: kappa_ptr
      type(c_ptr),    value              :: total_mass_mixing_ratio_ptr
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(real64), pointer :: mass_mixing_ratio(:)
      real(real64), pointer :: number_mixing_ratio(:)
      real(real64), pointer :: number_density(:)
      real(real64), pointer :: nucleation_rate(:)
      real(real64), pointer :: wet_particle_radius(:)
      real(real64), pointer :: wet_particle_density(:)
      real(real64), pointer :: dry_particle_density(:)
      real(real64), pointer :: fall_velocity(:)
      real(real64), pointer :: delta_particle_temperature(:)
      real(real64), pointer :: kappa(:)
      real(real64), pointer :: total_mass_mixing_ratio(:)
      type(carmastate_type), pointer :: cstate

      rc = 0
      if (element_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if
      if (bin_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)

         call c_f_pointer(mass_mixing_ratio_ptr, mass_mixing_ratio, [nz])
         call c_f_pointer(number_mixing_ratio_ptr, number_mixing_ratio, [nz])
         call c_f_pointer(number_density_ptr, number_density, [nz])
         call c_f_pointer(nucleation_rate_ptr, nucleation_rate, [nz])
         call c_f_pointer(wet_particle_radius_ptr, wet_particle_radius, [nz])
         call c_f_pointer(wet_particle_density_ptr, wet_particle_density, [nz])
         call c_f_pointer(dry_particle_density_ptr, dry_particle_density, [nz])
         call c_f_pointer(fall_velocity_ptr, fall_velocity, [nz+1])
         call c_f_pointer(delta_particle_temperature_ptr, delta_particle_temperature, [nz])
         call c_f_pointer(kappa_ptr, kappa, [nz])
         call c_f_pointer(total_mass_mixing_ratio_ptr, total_mass_mixing_ratio, [nz])

         call CARMASTATE_GetBin(cstate, ibin=bin_index, ielem=element_index, mmr=mass_mixing_ratio, rc=rc,&
            nmr=number_mixing_ratio, numberDensity=number_density, nucleationRate=nucleation_rate, r_wet=wet_particle_radius, &
            rhop_wet=wet_particle_density, rhop_dry=dry_particle_density, surface=particle_mass_on_surface, &
            sedimentationFlux=sedimentation_flux, vf=fall_velocity, vd=deposition_velocity, &
            dtpart=delta_particle_temperature, kappa=kappa, totalmmr=total_mass_mixing_ratio)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_GET_FAILED
            return
         end if
         ! Convert to SI base units
         number_density = number_density * 1.0e6  ! # cm-3 to # m-3
         nucleation_rate = nucleation_rate * 1.0e6  ! # cm-3 s-1 to # m-3 s-1
         wet_particle_radius = wet_particle_radius * 1.0e-2  ! cm to m
         wet_particle_density = wet_particle_density * 1.0e3  ! g cm-3 to kg m-3
         dry_particle_density = dry_particle_density * 1.0e3  ! g cm-3 to kg m-3
         fall_velocity = fall_velocity * 1.0e-2  ! cm s-1 to m s-1
         deposition_velocity = deposition_velocity * 1.0e-2  ! cm s-1 to m s-1
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if
   end subroutine internal_get_bin


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_detrain(carma_state_cptr, bin_index, element_index, nz, mass_mixing_ratio_ptr, &
      number_mixing_ratio_ptr, number_density_ptr, wet_particle_radius_ptr, &
      wet_particle_density_ptr, rc) &
      bind(C, name="InternalGetDetrain")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use iso_fortran_env, only: real64
      use carma_types_mod, only: carmastate_type
      use carmastate_mod, only: CARMASTATE_GetDetrain

      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      integer(c_int), value, intent(in)  :: bin_index
      integer(c_int), value, intent(in)  :: element_index
      integer(c_int), value, intent(in)  :: nz
      type(c_ptr),    value              :: mass_mixing_ratio_ptr
      type(c_ptr),    value              :: number_mixing_ratio_ptr
      type(c_ptr),    value              :: number_density_ptr
      type(c_ptr),    value              :: wet_particle_radius_ptr
      type(c_ptr),    value              :: wet_particle_density_ptr
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(real64), pointer :: mass_mixing_ratio(:)
      real(real64), pointer :: number_mixing_ratio(:)
      real(real64), pointer :: number_density(:)
      real(real64), pointer :: wet_particle_radius(:)
      real(real64), pointer :: wet_particle_density(:)
      type(carmastate_type), pointer :: cstate

      rc = 0

      if (element_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if
      if (bin_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(mass_mixing_ratio_ptr, mass_mixing_ratio, [nz])
         call c_f_pointer(number_mixing_ratio_ptr, number_mixing_ratio, [nz])
         call c_f_pointer(number_density_ptr, number_density, [nz])
         call c_f_pointer(wet_particle_radius_ptr, wet_particle_radius, [nz])
         call c_f_pointer(wet_particle_density_ptr, wet_particle_density, [nz])

         call CARMASTATE_GetDetrain(cstate, ibin=bin_index, ielem=element_index, &
            mmr=mass_mixing_ratio, rc=rc, nmr=number_mixing_ratio, &
            numberDensity=number_density, r_wet=wet_particle_radius, &
            rhop_wet=wet_particle_density)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_GET_FAILED
            return
         end if
         ! Convert to SI base units
         number_density = number_density * 1.0e6  ! # cm-3 to # m-3
         wet_particle_radius = wet_particle_radius * 1.0e-2  ! cm to m
         wet_particle_density = wet_particle_density * 1.0e3  ! g cm-3 to kg m-3
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if
   end subroutine internal_get_detrain

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_gas(carma_state_cptr, gas_index, nz, &
      mass_mixing_ratio_ptr, gas_saturation_wrt_ice_ptr, &
      gas_saturation_wrt_liquid_ptr, gas_vapor_pressure_wrt_ice_ptr, &
      gas_vapor_pressure_wrt_liquid_ptr, weight_pct_aerosol_composition_ptr, rc) &
      bind(C, name="InternalGetGas")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use iso_fortran_env, only: real64
      use carma_types_mod, only: carmastate_type
      use carmastate_mod, only: CARMASTATE_GetGas

      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      integer(c_int), value, intent(in)  :: gas_index
      integer(c_int), value, intent(in)  :: nz
      type(c_ptr),    value              :: mass_mixing_ratio_ptr
      type(c_ptr),    value              :: gas_saturation_wrt_ice_ptr
      type(c_ptr),    value              :: gas_saturation_wrt_liquid_ptr
      type(c_ptr),    value              :: gas_vapor_pressure_wrt_ice_ptr
      type(c_ptr),    value              :: gas_vapor_pressure_wrt_liquid_ptr
      type(c_ptr),    value              :: weight_pct_aerosol_composition_ptr
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(real64), pointer :: mass_mixing_ratio(:)
      real(real64), pointer :: gas_vapor_pressure_wrt_ice(:)
      real(real64), pointer :: gas_vapor_pressure_wrt_liquid(:)
      real(real64), pointer :: gas_saturation_wrt_ice(:)
      real(real64), pointer :: gas_saturation_wrt_liquid(:)
      real(real64), pointer :: weight_pct_aerosol_composition(:)
      type(carmastate_type), pointer :: cstate

      rc = 0

      if (gas_index < 1) then
         rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
         return
      end if

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(mass_mixing_ratio_ptr, mass_mixing_ratio, [nz])
         call c_f_pointer(gas_saturation_wrt_ice_ptr, gas_saturation_wrt_ice, [nz])
         call c_f_pointer(gas_saturation_wrt_liquid_ptr, gas_saturation_wrt_liquid, [nz])
         call c_f_pointer(gas_vapor_pressure_wrt_ice_ptr, gas_vapor_pressure_wrt_ice, [nz])
         call c_f_pointer(gas_vapor_pressure_wrt_liquid_ptr, gas_vapor_pressure_wrt_liquid, [nz])
         call c_f_pointer(weight_pct_aerosol_composition_ptr, weight_pct_aerosol_composition, [nz])

         call CARMASTATE_GetGas(cstate, igas=gas_index, mmr=mass_mixing_ratio, rc=rc, &
            satice=gas_saturation_wrt_ice, &
            satliq=gas_saturation_wrt_liquid, &
            eqice=gas_vapor_pressure_wrt_ice, &
            eqliq=gas_vapor_pressure_wrt_liquid, &
            wtpct=weight_pct_aerosol_composition)

         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_GET_FAILED
            return
         end if
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if
   end subroutine internal_get_gas

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_state_environmental_values(carma_state_cptr, nz, temperature_ptr, pressure_ptr, air_density_ptr, latent_heat_ptr, rc) &
      bind(C, name="InternalGetEnvironmentalValues")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use iso_fortran_env, only: real64
      use carma_types_mod, only: carmastate_type
      use carmastate_mod, only: CARMASTATE_GetState

      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      integer(c_int), value, intent(in)  :: nz
      type(c_ptr),    value              :: temperature_ptr
      type(c_ptr),    value              :: pressure_ptr
      type(c_ptr),    value              :: air_density_ptr
      type(c_ptr),    value              :: latent_heat_ptr
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(real64), pointer :: temperature(:)
      real(real64), pointer :: pressure(:)
      real(real64), pointer :: air_density(:)
      real(real64), pointer :: latent_heat(:)

      type(carmastate_type), pointer :: cstate

      rc = 0

      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(temperature_ptr, temperature, [nz])
         call c_f_pointer(pressure_ptr, pressure, [nz])
         call c_f_pointer(air_density_ptr, air_density, [nz])
         call c_f_pointer(latent_heat_ptr, latent_heat, [nz])

         if (allocated(cstate%f_rlheat)) then
            ! the latent heat is only allocated if it is used in the simulation
            ! therefore we can only pass that argument to CARMASTATE_GetState if it is allocated
            call CARMASTATE_GetState(cstate, t=temperature, p=pressure, &
               rhoa_wet=air_density, rlheat=latent_heat, rc=rc)
         else
            ! -1 allows us to know to set this to None in the python wrapper, which will indicate
            ! that no latent heat was calculated
            latent_heat = -1.0
            call CARMASTATE_GetState(cstate, t=temperature, p=pressure, &
               rhoa_wet=air_density, rc=rc)
         end if

         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_GET_FAILED
            return
         end if
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if
   end subroutine internal_get_state_environmental_values

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_set_temperature(carma_state_cptr, temperature_ptr, &
      temperature_size, rc) bind(C, name="InternalSetTemperature")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use carmastate_mod, only: CARMASTATE_SetState
      use carma_types_mod, only: carmastate_type
      use iso_fortran_env, only: real64

      ! Arguments
      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      type(c_ptr),    value              :: temperature_ptr
      integer(c_int), value, intent(in)  :: temperature_size
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(kind=real64), pointer :: temperature(:)
      type(carmastate_type), pointer :: cstate

      ! Check if carma_state_cptr is associated
      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(temperature_ptr, temperature, [temperature_size])

         call CARMASTATE_SetState(cstate, rc, t=temperature)

         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_SET_FAILED
            return
         end if
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if

   end subroutine internal_set_temperature

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_set_air_density(carma_state_cptr, air_density_ptr, &
      air_density_size, rc) bind(C, name="InternalSetAirDensity")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use carmastate_mod, only: CARMASTATE_SetState
      use carma_types_mod, only: carmastate_type
      use iso_fortran_env, only: real64

      ! Arguments
      type(c_ptr),    value, intent(in)  :: carma_state_cptr
      type(c_ptr),    value              :: air_density_ptr
      integer(c_int), value, intent(in)  :: air_density_size
      integer(c_int), intent(out)        :: rc

      ! Local variables
      real(kind=real64), pointer :: air_density(:)
      type(carmastate_type), pointer :: cstate

      ! Check if carma_state_cptr is associated
      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         call c_f_pointer(air_density_ptr, air_density, [air_density_size])

         call CARMASTATE_SetState(cstate, rc, rhoa_wet=air_density)

         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_SET_FAILED
            return
         end if
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if

   end subroutine internal_set_air_density

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_step(carma_state_cptr, step_config, rc) &
      bind(C, name="InternalStepCarmaState")
      use iso_c_binding, only: c_ptr, c_int
      use iso_fortran_env, only: real64
      use carmastate_mod, only: CARMASTATE_Step, CARMASTATE_GetBin
      use carma_types_mod, only: carmastate_type
      use carma_parameters_mod, only: carma_state_step_config_t

      ! Arguments
      type(c_ptr),                     value, intent(in)  :: carma_state_cptr
      type(carma_state_step_config_t), value, intent(in)  :: step_config
      integer(c_int),                         intent(out) :: rc

      ! Local variables
      type(carmastate_type), pointer :: cstate
      real(real64), pointer :: cloud_fraction(:), critical_rh(:)
      logical :: deallocate_cloud_fraction, deallocate_critical_rh

      rc = 0
      deallocate_cloud_fraction = .false.
      deallocate_critical_rh = .false.

      ! Check if carma_state_cptr is associated
      if (c_associated(carma_state_cptr)) then
         call c_f_pointer(carma_state_cptr, cstate)
         if (step_config%cloud_fraction_size > 0) then
            call c_f_pointer(step_config%cloud_fraction, cloud_fraction, &
               [step_config%cloud_fraction_size])
         else
            allocate(cloud_fraction(cstate%f_NZ))
            cloud_fraction(:) = 1.0_real64
            deallocate_cloud_fraction = .true.
         end if
         if (step_config%critical_relative_humidity_size > 0) then
            call c_f_pointer(step_config%critical_relative_humidity, critical_rh, &
               [step_config%critical_relative_humidity_size])
         else
            allocate(critical_rh(cstate%f_NZ))
            critical_rh(:) = 1.0_real64
            deallocate_critical_rh = .true.
         end if
         call CARMASTATE_Step( &
            cstate=cstate, &
            rc=rc, &
            cldfrc=cloud_fraction, &
            rhcrit=critical_rh, &
            lndfv=step_config%land%surface_friction_velocity, &
            ocnfv=step_config%ocean%surface_friction_velocity, &
            icefv=step_config%ice%surface_friction_velocity, &
            lndram=step_config%land%aerodynamic_resistance, &
            ocnram=step_config%ocean%aerodynamic_resistance, &
            iceram=step_config%ice%aerodynamic_resistance, &
            lndfrac=step_config%land%area_fraction, &
            ocnfrac=step_config%ocean%area_fraction, &
            icefrac=step_config%ice%area_fraction)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_STEP_FAILED
            return
         end if
         if (deallocate_cloud_fraction) then
            deallocate(cloud_fraction)
         end if
         if (deallocate_critical_rh) then
            deallocate(critical_rh)
         end if
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if
   end subroutine internal_step

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_carma_parameters(carma_cptr, group_index, nbin, nwav, nelem, bin_radius_ptr, bin_radius_lower_bound_ptr, &
      bin_radius_upper_bound_ptr, bin_width_ptr, bin_mass_ptr, &
      bin_width_mass_ptr, bin_volume_ptr, projected_area_ratio_ptr, &
      radius_ratio_ptr, porosity_ratio_ptr, extinction_coefficient_ptr, &
      single_scattering_albedo_ptr, asymmetry_factor_ptr, &
      particle_number_element_for_group, number_of_core_mass_elements_for_group_ptr, &
      element_index_of_core_mass_elements_ptr, last_prognostic_bin, &
      numbers_of_monomers_per_bin_ptr, rc) &
      bind(C, name="InternalGetGroupProperties")
      use iso_c_binding, only: c_ptr, c_int, c_double
      use carma_types_mod, only: carma_type
      use carmagroup_mod, only: CARMAGROUP_Get

      ! Arguments
      type(c_ptr),    value, intent(in)  :: carma_cptr
      integer(c_int), value, intent(in)  :: group_index
      integer(c_int), value, intent(in)  :: nbin
      integer(c_int), value, intent(in)  :: nwav
      integer(c_int), value, intent(in)  :: nelem
      type(c_ptr),    value              :: bin_radius_ptr
      type(c_ptr),    value              :: bin_radius_lower_bound_ptr
      type(c_ptr),    value              :: bin_radius_upper_bound_ptr
      type(c_ptr),    value              :: bin_width_ptr
      type(c_ptr),    value              :: bin_mass_ptr
      type(c_ptr),    value              :: bin_width_mass_ptr
      type(c_ptr),    value              :: bin_volume_ptr
      type(c_ptr),    value              :: projected_area_ratio_ptr
      type(c_ptr),    value              :: radius_ratio_ptr
      type(c_ptr),    value              :: porosity_ratio_ptr
      type(c_ptr),    value              :: extinction_coefficient_ptr
      type(c_ptr),    value              :: single_scattering_albedo_ptr
      type(c_ptr),    value              :: asymmetry_factor_ptr
      integer(c_int), intent(out)        :: particle_number_element_for_group
      integer(c_int), intent(out)        :: number_of_core_mass_elements_for_group_ptr
      type(c_ptr),    value              :: element_index_of_core_mass_elements_ptr
      integer(c_int), intent(out)        :: last_prognostic_bin
      type(c_ptr),    value              :: numbers_of_monomers_per_bin_ptr
      integer(c_int), intent(out)        :: rc

      ! Local variables
      type(carma_type), pointer :: carma
      real(kind=c_double), pointer :: bin_radius(:)
      real(kind=c_double), pointer :: bin_radius_lower_bound(:)
      real(kind=c_double), pointer :: bin_radius_upper_bound(:)
      real(kind=c_double), pointer :: bin_width(:)
      real(kind=c_double), pointer :: bin_mass(:)
      real(kind=c_double), pointer :: bin_width_mass(:)
      real(kind=c_double), pointer :: bin_volume(:)
      real(kind=c_double), pointer :: projected_area_ratio(:)
      real(kind=c_double), pointer :: radius_ratio(:)
      real(kind=c_double), pointer :: porosity_ratio(:)
      real(kind=c_double), pointer :: extinction_coefficient(:, :)
      real(kind=c_double), pointer :: single_scattering_albedo(:, :)
      real(kind=c_double), pointer :: asymmetry_factor(:, :)
      integer(c_int), pointer :: element_index_of_core_mass_elements(:)
      real(kind=c_double), pointer :: numbers_of_monomers_per_bin(:)

      if (c_associated(carma_cptr)) then
         call c_f_pointer(carma_cptr, carma)
         call c_f_pointer(bin_radius_ptr, bin_radius, [nbin])
         call c_f_pointer(bin_radius_lower_bound_ptr, bin_radius_lower_bound, [nbin])
         call c_f_pointer(bin_radius_upper_bound_ptr, bin_radius_upper_bound, [nbin])
         call c_f_pointer(bin_width_ptr, bin_width, [nbin])
         call c_f_pointer(bin_mass_ptr, bin_mass, [nbin])
         call c_f_pointer(bin_width_mass_ptr, bin_width_mass, [nbin])
         call c_f_pointer(bin_volume_ptr, bin_volume, [nbin])
         call c_f_pointer(projected_area_ratio_ptr, projected_area_ratio, [nbin])
         call c_f_pointer(radius_ratio_ptr, radius_ratio, [nbin])
         call c_f_pointer(porosity_ratio_ptr, porosity_ratio, [nbin])
         call c_f_pointer(extinction_coefficient_ptr, extinction_coefficient, [nwav, nbin])
         call c_f_pointer(single_scattering_albedo_ptr, single_scattering_albedo, [nwav, nbin])
         call c_f_pointer(asymmetry_factor_ptr, asymmetry_factor, [nwav, nbin])
         call c_f_pointer(element_index_of_core_mass_elements_ptr, element_index_of_core_mass_elements, [nelem])
         call c_f_pointer(numbers_of_monomers_per_bin_ptr, numbers_of_monomers_per_bin, [nbin])

         ! Get group parameters
         call CARMAGROUP_Get(carma=carma, igroup=group_index, rc=rc,&
            r=bin_radius, rlow=bin_radius_lower_bound, rup=bin_radius_upper_bound, &
            dr=bin_width, rmass=bin_mass, dm=bin_width_mass, vol=bin_volume, &
            arat=projected_area_ratio, rrat=radius_ratio, rprat=porosity_ratio, &
            qext=extinction_coefficient, ssa=single_scattering_albedo, asym=asymmetry_factor, &
            ienconc=particle_number_element_for_group, &
            icorelem=element_index_of_core_mass_elements, &
            ncore=number_of_core_mass_elements_for_group_ptr, &
            maxbin=last_prognostic_bin, nmon=numbers_of_monomers_per_bin)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_GET_FAILED
            return
         end if
         ! Convert to base SI units
         bin_radius = bin_radius * 0.01 ! Convert from cm to m
         bin_radius_lower_bound = bin_radius_lower_bound * 0.01 ! Convert from cm to m
         bin_radius_upper_bound = bin_radius_upper_bound * 0.01 ! Convert from cm to m
         bin_width = bin_width * 0.01 ! Convert from cm to m
         bin_mass = bin_mass * 1.0e-3 ! Convert from g to kg
         bin_width_mass = bin_width_mass * 1.0e-3 ! Convert from g to kg
         bin_volume = bin_volume * 1.0e-6 ! Convert from cm^3 to m^3
      else
         rc = MUSICA_CARMA_ERROR_CODE_UNASSOCIATED_POINTER
      end if

   end subroutine internal_get_carma_parameters

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_element_properties(carma_cptr, element_index, &
      element_props, rc) bind(C, name="InternalGetElementProperties")

      use iso_c_binding, only: c_ptr, c_int, c_double
      use iso_fortran_env, only: real64
      use carma_parameters_mod, only: carma_element_properties_t, carma_complex_t
      use carma_types_mod, only: carma_type
      use carmaelement_mod, only: CARMAELEMENT_Get

      type(c_ptr), value, intent(in)  :: carma_cptr
      integer(c_int), value, intent(in) :: element_index
      type(carma_element_properties_t), intent(inout) :: element_props
      integer(c_int), intent(out) :: rc

      type(carma_type), pointer :: carma
      real(kind=real64), pointer :: rho(:)
      type(carma_complex_t), pointer :: refidx_c(:,:)
      complex(kind=real64), allocatable :: refidx(:,:)
      logical :: isShell

      rc = 0

      if (c_associated(carma_cptr)) then
         call c_f_pointer(carma_cptr, carma)
         call c_f_pointer(element_props%rho, rho, [element_props%rho_size])
         call c_f_pointer(element_props%refidx, refidx_c, &
            [element_props%refidx_dim_2_size, element_props%refidx_dim_1_size])
         allocate(refidx(element_props%refidx_dim_2_size, element_props%refidx_dim_1_size))
         refidx(:,:) = cmplx(0.0, 0.0, kind=real64)

         ! Get element properties
         if (carma%f_NWAVE < 1 .or. carma%f_NREFIDX < 1) then
            call CARMAELEMENT_Get( &
               carma=carma, &
               ielement=element_index, &
               rc=rc, &
               igroup=element_props%igroup, &
               rho=rho, &
               itype=element_props%itype, &
               icomposition=element_props%icomposition, &
               isolute=element_props%isolute, &
               kappa=element_props%kappa, &
               isShell=isShell)
         else
            call CARMAELEMENT_Get( &
               carma=carma, &
               ielement=element_index, &
               rc=rc, &
               igroup=element_props%igroup, &
               rho=rho, &
               itype=element_props%itype, &
               icomposition=element_props%icomposition, &
               isolute=element_props%isolute, &
               kappa=element_props%kappa, &
               refidx=refidx, &
               isShell=isShell)
         end if
         element_props%isShell = isShell
         refidx_c(:,:)%real_part = real(real(refidx(:,:)), kind=c_double)
         refidx_c(:,:)%imag_part = real(aimag(refidx(:,:)), kind=c_double)
         deallocate(refidx)
         ! Convert to base SI units
         rho(:) = rho(:) * 1.0e-3  ! g/cm³ to kg/m³
      else
         rc = 1
         print *, "CARMA pointer is not associated"
      end if

   end subroutine internal_get_element_properties

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_destroy_carma(carma_cptr, rc) &
      bind(C, name="InternalDestroyCarma")
      use carma_types_mod, only: carma_type
      use carma_mod, only: CARMA_Destroy
      use iso_c_binding, only: c_ptr, c_int, c_f_pointer, c_associated

      type(c_ptr),    value, intent(in)  :: carma_cptr
      integer(c_int),        intent(out) :: rc

      type(carma_type), pointer :: carma

      rc = 0

      if (c_associated(carma_cptr)) then
         call c_f_pointer(carma_cptr, carma)

         ! Clean up the carma instance
         call CARMA_Destroy(carma, rc)
         if (rc /= 0) then
            rc = MUSICA_CARMA_ERROR_CODE_DESTROY_FAILED
            return
         end if
         deallocate(carma)
      end if

   end subroutine internal_destroy_carma

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function c_to_f_string(c_string, string_size) result(f_string)

      use iso_c_binding, only: c_ptr, c_loc, c_char

      character(len=1, kind=c_char), intent(in) :: c_string(:)
      integer(c_int), intent(in) :: string_size

      character(len=string_size), allocatable :: f_string
      integer :: i

      ! Convert C string to Fortran string
      allocate(character(len=string_size) :: f_string)
      f_string(:) = ' '  ! Initialize with spaces
      do i = 1, string_size
         if (c_string(i) == c_null_char) exit
         f_string(i:i) = c_string(i)
      end do

   end function c_to_f_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine create_carma_instance(params, carma_cptr, rc)
      use carma_precision_mod
      use carma_constants_mod
      use carma_enums_mod
      use carma_types_mod
      use carmaelement_mod
      use carmagroup_mod
      use carmagas_mod
      use carmasolute_mod
      use carma_mod
      use atmosphere_mod
      use carma_parameters_mod, only: carma_parameters_t, carma_group_config_t, &
         carma_element_config_t, carma_wavelength_bin_t, &
         carma_element_config_t, carma_wavelength_bin_t, &
         carma_complex_t, carma_solute_config_t, carma_gas_config_t, &
         carma_coagulation_config_t, carma_growth_config_t, &
         carma_nucleation_config_t
      use iso_fortran_env, only: real64
      use iso_c_binding, only: c_ptr, c_loc

      implicit none

      type(carma_parameters_t), intent(in)  :: params
      type(c_ptr),              intent(out) :: carma_cptr
      integer,                  intent(out) :: rc

      ! Local variables for CARMA simulation
      type(carma_type), pointer :: carma

      ! Model dimensions
      integer :: NELEM, NGROUP, NBIN, NSOLUTE, NGAS, NWAVE

      ! Loop indices
      integer :: iwave, ielem, igroup, isolute, igas, icoag, igrowth, inucleation

      ! Wavelength grid parameters
      type(carma_wavelength_bin_t), pointer :: wavelength_bins(:)
      real(real64), allocatable :: wave_centers(:), wave_widths(:)
      logical, allocatable  :: wave_do_emission(:)

      ! Group parameters
      type(carma_group_config_t), pointer :: group_config(:)
      character(len=:), allocatable :: group_name, group_short_name
      real(kind=real64), pointer :: df(:) ! fractal dimension per group (NBINS)

      ! Element parameters
      type(carma_element_config_t), pointer :: element_config(:)
      character(len=:), allocatable :: element_name, element_short_name
      real(real64), pointer :: rhobin(:) ! rho bin for element (NBINS)
      real(real64), pointer :: arat(:) ! area ratio for element (NBINS)
      type(carma_complex_t), pointer :: refidx_t(:,:) ! refractive index [NWAVE, number_of_refractive_indices]
      complex(kind=real64), allocatable :: refidx(:,:) ! refractive index [NWAVE, number_of_refractive_indices]

      ! Solute parameters
      type(carma_solute_config_t), pointer :: solute_config(:)
      character(len=:), allocatable :: solute_name, solute_short_name

      ! Gas parameters
      type(carma_gas_config_t), pointer :: gas_config(:)
      character(len=:), allocatable :: gas_name, gas_short_name
      type(carma_complex_t), pointer :: gas_refidx_t(:,:) ! refractive index [NWAVE, number_of_refractive_indices]
      complex(kind=real64), allocatable :: gas_refidx(:,:) ! refractive index [NWAVE, number_of_refractive_indices]

      ! Process parameters
      ! Coagulation
      type(carma_coagulation_config_t), pointer :: coagulation_config(:)

      ! Growth parameters
      type(carma_growth_config_t), pointer :: growth_config(:)

      ! Nucleation parameters
      type(carma_nucleation_config_t), pointer :: nucleation_config(:)
      character(len=:), allocatable :: sulfate_nucleation_method

      integer :: alloc_stat

      rc = 0

      ! Set dimensions from parameters
      NELEM = int(params%elements_size)
      NGROUP = int(params%groups_size)
      NBIN = int(params%nbin)
      NSOLUTE = int(params%solutes_size)
      NGAS = int(params%gases_size)
      NWAVE = int(params%wavelength_bin_size)

      ! Create the CARMA instance
      allocate(carma, stat=alloc_stat)
      if (alloc_stat /= 0) then
         rc = MUSICA_CARMA_ERROR_CODE_MEMORY_ALLOCATION
         return
      end if

      if (NWAVE>0) then
         call c_f_pointer(params%wavelength_bins, wavelength_bins, [NWAVE])
         allocate(wave_centers(NWAVE))
         allocate(wave_widths(NWAVE))
         allocate(wave_do_emission(NWAVE))
         do iwave = 1, NWAVE
            wave_centers(iwave) = wavelength_bins(iwave)%center * 100.0_real64 ! Convert m to cm
            wave_widths(iwave) = wavelength_bins(iwave)%width * 100.0_real64 ! Convert m to cm
            wave_do_emission(iwave) = wavelength_bins(iwave)%do_emission
         end do
         call CARMA_Create(carma, NBIN, NELEM, NGROUP, NSOLUTE, NGAS, NWAVE, rc, &
            wave = wave_centers, &
            dwave = wave_widths, &
            do_wave_emit = wave_do_emission, &
            NREFIDX = params%number_of_refractive_indices, &
            LUNOPRT=6) ! Direct output to stdout
      else
         call CARMA_Create(carma, NBIN, NELEM, NGROUP, NSOLUTE, NGAS, NWAVE, rc)
      end if
      if (rc /= 0) then
         rc = MUSICA_CARMA_ERROR_CODE_CREATION_FAILED
         return
      end if

      ! Create groups based on configuration
      if (c_associated(params%groups)) then
         call c_f_pointer(params%groups, group_config, [params%groups_size])
         do igroup = 1, params%groups_size
            associate(group => group_config(igroup))
               group_name = c_to_f_string(group%name, group%name_length)
               group_short_name = c_to_f_string(group%shortname, group%shortname_length)
               call c_f_pointer(group%df, df, [group%df_size])
               call CARMAGROUP_Create( &
                  carma=carma, &
                  igroup=igroup, &
                  name=group_name, &
                  rmin=real(group%rmin, kind=real64) * 100.0_real64, & ! Convert m to cm
                  rmrat=real(group%rmrat, kind=real64), &
                  ishape=int(group%ishape), &
                  eshape=real(group%eshape, kind=real64), &
                  is_ice=logical(group%is_ice), &
                  rc=rc, &
                  is_fractal=logical(group%is_fractal), &
                  irhswell=int(group%swelling_algorithm), &
                  irhswcomp=int(group%swelling_composition), &
                  do_mie=(group%mie_calculation_algorithm /= 0), &
                  do_wetdep=logical(group%do_wetdep), &
                  do_drydep=logical(group%do_drydep), &
                  do_vtran=logical(group%do_vtran), &
                  solfac=real(group%solfac, kind=real64), &
                  scavcoef=real(group%scavcoef, kind=real64), &
                  shortname=group_short_name, &
                  ifallrtn=int(group%fall_velocity_routine), &
                  is_cloud= logical(group%is_cloud), &
                  rmassmin=real(group%rmassmin, kind=real64) * 1000.0_real64, & ! Convert kg to g
                  imiertn=int(group%mie_calculation_algorithm), &
                  iopticstype=int(group%optics_algorithm), &
                  is_sulfate=logical(group%is_sulfate), &
                  dpc_threshold=real(group%dpc_threshold, kind=real64), &
                  rmon=real(group%rmon, kind=real64) * 100.0_real64, & ! Convert m to cm
                  df=df, &
                  falpha=real(group%falpha, kind=real64), &
                  neutral_volfrc=real(group%neutral_volfrc, kind=real64))
               if (rc /= 0) then
                  rc = MUSICA_CARMA_ERROR_CODE_CREATION_FAILED
                  return
               end if
            end associate
         end do
      end if

      ! Create elements based on configuration
      if (c_associated(params%elements)) then
         call c_f_pointer(params%elements, element_config, [params%elements_size])
         do ielem = 1, params%elements_size
            associate(elem => element_config(ielem))
               element_name = c_to_f_string(elem%name, elem%name_length)
               element_short_name = c_to_f_string(elem%shortname, elem%shortname_length)
               if (elem%rhobin_size > 0) then
                  call c_f_pointer(elem%rhobin, rhobin, [elem%rhobin_size])
                  rhobin(:) = rhobin(:) * 0.001_real64 ! Convert kg m-3 to g cm-3
                  if (elem%rhobin_size /= NBIN) then
                     rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
                     return
                  end if
               else
                  rhobin => null( )
               end if
               if (elem%arat_size > 0) then
                  call c_f_pointer(elem%arat, arat, [elem%arat_size])
                  if (elem%arat_size /= NBIN) then
                     rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
                     return
                  end if
               else
                  arat => null( )
               end if
               if (elem%refidx_dim_1_size > 0 .and. elem%refidx_dim_2_size > 0) then
                  if (elem%refidx_dim_2_size /= NWAVE) then
                     rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
                     return
                  end if
                  if (elem%refidx_dim_1_size /= params%number_of_refractive_indices) then
                     rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
                     return
                  end if
                  call c_f_pointer(elem%refidx, refidx_t, [elem%refidx_dim_2_size, elem%refidx_dim_1_size])
                  allocate(refidx(elem%refidx_dim_2_size, elem%refidx_dim_1_size), stat=alloc_stat)
                  if (alloc_stat /= 0) then
                     rc = MUSICA_CARMA_ERROR_CODE_MEMORY_ALLOCATION
                     return
                  end if
                  do iwave = 1, NWAVE
                     refidx(iwave,:) = cmplx(refidx_t(iwave,1:params%number_of_refractive_indices)%real_part, &
                        refidx_t(iwave,1:params%number_of_refractive_indices)%imag_part, kind=real64)
                  end do
               end if
               call CARMAELEMENT_Create( &
                  carma=carma, &
                  ielement=ielem, &
                  igroup=int(elem%igroup), &
                  name=element_name, &
                  rho=real(elem%rho, kind=real64) * 0.001_real64, & ! Convert kg m-3 to g cm-3
                  itype=int(elem%itype), &
                  icomposition=int(elem%icomposition), &
                  rc=rc, &
                  shortname=element_short_name, &
                  isolute=int(elem%isolute), &
                  rhobin=rhobin, &
                  arat=arat, &
                  kappa=real(elem%kappa, kind=real64), &
                  refidx=refidx, &
                  isShell=logical(elem%isShell))
               if (rc /= 0) then
                  rc = MUSICA_CARMA_ERROR_CODE_CREATION_FAILED
                  return
               end if
            end associate
         end do
      end if

      ! Create solutes based on configuration
      if (c_associated(params%solutes)) then
         call c_f_pointer(params%solutes, solute_config, [params%solutes_size])
         do isolute = 1, params%solutes_size
            associate(solute => solute_config(isolute))
               solute_name = c_to_f_string(solute%name, solute%name_length)
               solute_short_name = c_to_f_string(solute%shortname, solute%shortname_length)
               call CARMASOLUTE_Create( &
                  carma=carma, &
                  isolute=isolute, &
                  name=solute_name, &
                  ions=int(solute%ions), &
                  wtmol=real(solute%wtmol, kind=real64), &
                  rho=real(solute%rho, kind=real64), &
                  rc=rc, &
                  shortname=solute_short_name)
               if (rc /= 0) then
                  rc = MUSICA_CARMA_ERROR_CODE_CREATION_FAILED
                  return
               end if
            end associate
         end do
      end if

      ! Create gases based on configuration
      if (c_associated(params%gases)) then
         call c_f_pointer(params%gases, gas_config, [params%gases_size])
         do igas = 1, params%gases_size
            associate(gas => gas_config(igas))
               gas_name = c_to_f_string(gas%name, gas%name_length)
               gas_short_name = c_to_f_string(gas%shortname, gas%shortname_length)
               if (gas%refidx_dim_1_size > 0 .and. gas%refidx_dim_2_size > 0) then
                  if (gas%refidx_dim_2_size /= NWAVE) then
                     rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
                     return
                  end if
                  if (gas%refidx_dim_1_size /= params%number_of_refractive_indices) then
                     rc = MUSICA_CARMA_ERROR_CODE_DIMENSION_MISMATCH
                     return
                  end if
                  call c_f_pointer(gas%refidx, gas_refidx_t, [gas%refidx_dim_2_size, gas%refidx_dim_1_size])
                  allocate(gas_refidx(gas%refidx_dim_2_size, gas%refidx_dim_1_size), stat=alloc_stat)
                  if (alloc_stat /= 0) then
                     rc = MUSICA_CARMA_ERROR_CODE_MEMORY_ALLOCATION
                     return
                  end if
                  do iwave = 1, NWAVE
                     gas_refidx(iwave,:) = cmplx(gas_refidx_t(iwave,1:params%number_of_refractive_indices)%real_part, &
                        gas_refidx_t(iwave,1:params%number_of_refractive_indices)%imag_part, kind=real64)
                  end do
               end if
               call CARMAGAS_Create( &
                  carma=carma, &
                  igas=igas, &
                  name=gas_name, &
                  wtmol=real(gas%wtmol, kind=real64) * 1000.0_real64, & ! convert to g/mol
                  ivaprtn=int(gas%ivaprtn), &
                  icomposition=int(gas%icomposition), &
                  rc=rc, &
                  shortname=gas_short_name, &
                  dgc_threshold=real(gas%dgc_threshold, kind=real64), &
                  ds_threshold=real(gas%ds_threshold, kind=real64), &
                  refidx=gas_refidx)
               if (rc /= 0) then
                  rc = MUSICA_CARMA_ERROR_CODE_CREATION_FAILED
                  return
               end if
            end associate
         end do
      end if

      ! Create coagulation processes based on configuration
      if (c_associated(params%coagulations)) then
         call c_f_pointer(params%coagulations, coagulation_config, [params%coagulations_size])
         do icoag = 1, params%coagulations_size
            associate(coag => coagulation_config(icoag))
               if (coag%ck0 .ge. 0.0_real64) then
                  call CARMA_AddCoagulation( &
                     carma=carma, &
                     igroup1=int(coag%igroup1), &
                     igroup2=int(coag%igroup2), &
                     igroup3=int(coag%igroup3), &
                     icollec=int(coag%algorithm), &
                     rc=rc, &
                     ck0=real(coag%ck0, kind=real64), &
                     grav_e_coll0=real(coag%grav_e_coll0, kind=real64), &
                     use_ccd=logical(coag%use_ccd))
               else
                  call CARMA_AddCoagulation( &
                     carma=carma, &
                     igroup1=int(coag%igroup1), &
                     igroup2=int(coag%igroup2), &
                     igroup3=int(coag%igroup3), &
                     icollec=int(coag%algorithm), &
                     rc=rc, &
                     grav_e_coll0=real(coag%grav_e_coll0, kind=real64), &
                     use_ccd=logical(coag%use_ccd))
               endif
               if (rc /= 0) then
                  rc = MUSICA_CARMA_ERROR_CODE_ADD_CARMA_OBJECT_FAILED
                  return
               end if
            end associate
         end do
      else
         carma%f_icoagop = 0 ! Disable coagulation if no processes defined
         carma%f_icollec = 0 ! Disable collection if no processes defined
      end if

      ! Create growth processes based on configuration
      if (c_associated(params%growths)) then
         call c_f_pointer(params%growths, growth_config, [params%growths_size])
         do igrowth = 1, params%growths_size
            associate(growth => growth_config(igrowth))
               call CARMA_AddGrowth( &
                  carma=carma, &
                  ielem=int(growth%ielem), &
                  igas=int(growth%igas), &
                  rc=rc)
               if (rc /= 0) then
                  rc = MUSICA_CARMA_ERROR_CODE_ADD_CARMA_OBJECT_FAILED
                  return
               end if
            end associate
         end do
      end if

      ! Create nucleation processes based on configuration
      if (c_associated(params%nucleations)) then
         call c_f_pointer(params%nucleations, nucleation_config, [params%nucleations_size])
         do inucleation = 1, params%nucleations_size
            associate(nucleation => nucleation_config(inucleation))
               call CARMA_AddNucleation( &
                  carma=carma, &
                  ielemfrom=int(nucleation%ielemfrom), &
                  ielemto=int(nucleation%ielemto), &
                  inucproc=int(nucleation%algorithm), &
                  rlh_nuc=real(nucleation%rlh_nuc, kind=real64) * 1.0e4_real64, & ! convert m2 s-2 to cm2 s-2
                  rc=rc, &
                  igas=int(nucleation%igas), &
                  ievp2elem=int(nucleation%ievp2elem))
               if (rc /= 0) then
                  rc = MUSICA_CARMA_ERROR_CODE_ADD_CARMA_OBJECT_FAILED
                  return
               end if
            end associate
         end do
      end if

      ! Initialize CARMA with coagulation settings matching test_aluminum_simple
      select case(int(params%initialization%sulfnucl_method))
         case(1)
            sulfate_nucleation_method = "ZhaoTurco"
         case(2)
            sulfate_nucleation_method = "Vehkamaki"
         case default
            sulfate_nucleation_method = "NONE"
      end select
      call CARMA_Initialize( &
         carma, &
         rc, &
         do_cnst_rlh=logical(params%initialization%do_cnst_rlh), &
         do_coag=params%coagulations_size > 0, &
         do_detrain=logical(params%initialization%do_detrain), &
         do_fixedinit=logical(params%initialization%do_fixedinit), &
         do_grow=params%growths_size > 0 .or. params%nucleations_size > 0, &
         do_incloud=logical(params%initialization%do_incloud), &
         do_explised=logical(params%initialization%do_explised), &
         do_substep=logical(params%initialization%do_substep), &
         do_thermo=logical(params%initialization%do_thermo), &
         do_vdiff=logical(params%initialization%do_vdiff), &
         do_vtran=logical(params%initialization%do_vtran), &
         do_drydep=logical(params%initialization%do_drydep), &
         vf_const=real(params%initialization%vf_const, kind=real64) * 100.0_real64, & ! Convert m to cm
         minsubsteps=int(params%initialization%minsubsteps), &
         maxsubsteps=int(params%initialization%maxsubsteps), &
         maxretries=int(params%initialization%maxretries), &
         conmax=real(params%initialization%conmax, kind=real64), &
         do_pheat=logical(params%initialization%do_pheat), &
         do_pheatatm=logical(params%initialization%do_pheatatm), &
         dt_threshold=real(params%initialization%dt_threshold, kind=real64), &
         cstick=real(params%initialization%cstick, kind=real64), &
         gsticki=real(params%initialization%gsticki, kind=real64), &
         gstickl=real(params%initialization%gstickl, kind=real64), &
         tstick=real(params%initialization%tstick, kind=real64), &
         do_clearsky=logical(params%initialization%do_clearsky), &
         do_partialinit=logical(params%initialization%do_partialinit), &
         do_coremasscheck=logical(params%initialization%do_coremasscheck), &
         sulfnucl_method=sulfate_nucleation_method &
         )
      if (rc /= 0) then
         rc = MUSICA_CARMA_ERROR_CODE_INITIALIZATION_FAILED
         return
      end if

      ! turn off printing
      carma%f_do_print = .false.
      carma%f_lunoprt = 6

      carma_cptr = c_loc(carma)

   end subroutine create_carma_instance

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module carma_interface
