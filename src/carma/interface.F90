! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module carma_interface

   use iso_c_binding, only: c_int, c_double, c_ptr, c_char, c_null_char, &
      c_loc, c_f_pointer, c_associated, c_null_ptr, c_bool
   implicit none

   private

   ! C-compatible structure for CARMA output data
   ! MUST match the exact order and types of the C++ CARMAOutputData struct
   type, bind(C) :: c_carma_output_data
      type(c_ptr) :: c_output_ptr
      integer(c_int) :: nz
      integer(c_int) :: ny
      integer(c_int) :: nx
      integer(c_int) :: nelem
      integer(c_int) :: ngroup
      integer(c_int) :: nbin
      integer(c_int) :: ngas
      integer(c_int) :: nstep
      type(c_ptr) :: lat
      type(c_ptr) :: lon
      type(c_ptr) :: vertical_center
      type(c_ptr) :: vertical_levels
      type(c_ptr) :: pressure
      type(c_ptr) :: temperature
      type(c_ptr) :: air_density
      type(c_ptr) :: radiative_heating
      type(c_ptr) :: delta_temperature
      type(c_ptr) :: gas_mmr
      type(c_ptr) :: gas_saturation_liquid
      type(c_ptr) :: gas_saturation_ice
      type(c_ptr) :: gas_vapor_pressure_ice
      type(c_ptr) :: gas_vapor_pressure_liquid
      type(c_ptr) :: gas_weight_percent
      type(c_ptr) :: number_density
      type(c_ptr) :: surface_area
      type(c_ptr) :: mass_density
      type(c_ptr) :: effective_radius
      type(c_ptr) :: effective_radius_wet
      type(c_ptr) :: mean_radius
      type(c_ptr) :: nucleation_rate
      type(c_ptr) :: mass_mixing_ratio
      type(c_ptr) :: projected_area
      type(c_ptr) :: aspect_ratio
      type(c_ptr) :: vertical_mass_flux
      type(c_ptr) :: extinction
      type(c_ptr) :: optical_depth
      type(c_ptr) :: bin_wet_radius
      type(c_ptr) :: bin_number_density
      type(c_ptr) :: bin_density
      type(c_ptr) :: bin_mass_mixing_ratio
      type(c_ptr) :: bin_deposition_velocity
      type(c_ptr) :: group_radius
      type(c_ptr) :: group_mass
      type(c_ptr) :: group_volume
      type(c_ptr) :: group_radius_ratio
      type(c_ptr) :: group_aspect_ratio
      type(c_ptr) :: group_fractal_dimension
   end type c_carma_output_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   ! Interface to the C++ TransferCarmaOutputToCpp function
   interface
      subroutine TransferCarmaOutputToCpp(output_data) &
         bind(C, name="TransferCarmaOutputToCpp")
         use iso_c_binding, only: c_ptr
         import :: c_carma_output_data

         type(c_carma_output_data), intent(in) :: output_data

      end subroutine TransferCarmaOutputToCpp
   end interface

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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

   subroutine internal_run_carma(params, c_output, rc) &
      bind(C, name="InternalRunCarma")
      use iso_c_binding, only: c_int, c_ptr, c_f_pointer
      use carma_parameters_mod, only: carma_parameters_t

      type(carma_parameters_t), intent(in) :: params
      type(c_ptr), value, intent(in) :: c_output
      integer(c_int), intent(out) :: rc

      rc = 0

      ! Run CARMA simulation with optional output
      call run_carma_simulation(params, rc, c_output)

   end subroutine internal_run_carma

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function c_to_f_string(c_string, string_size) result(f_string)

     use iso_c_binding, only: c_ptr, c_loc, c_char

     type(c_ptr), intent(in) :: c_string
     integer(c_int), intent(in) :: string_size

     character(kind=c_char), pointer :: c_string_ptr(:)
     character(len=string_size), allocatable :: f_string
     integer :: i

     ! Convert C string to Fortran string
     allocate(character(len=string_size) :: f_string)
     call c_f_pointer(c_string, c_string_ptr, [string_size])
     do i = 1, string_size
        if (c_string_ptr(i) == c_null_char) exit
        f_string(i:i) = c_string_ptr(i)
     end do

   end function c_to_f_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine run_carma_simulation(params, rc, c_output_ptr)
      use carma_precision_mod
      use carma_constants_mod
      use carma_enums_mod
      use carma_types_mod
      use carmaelement_mod
      use carmagroup_mod
      use carmagas_mod
      use carmastate_mod
      use carma_mod
      use atmosphere_mod
      use carma_parameters_mod, only: carma_parameters_t, carma_group_config_t, &
                                      carma_element_config_t
      use iso_fortran_env, only: real64
      use iso_c_binding, only: c_ptr, c_null_ptr, c_associated

      implicit none

      type(carma_parameters_t), intent(in) :: params
      integer, intent(out) :: rc
      type(c_ptr), intent(in), optional :: c_output_ptr

      ! Local variables for CARMA simulation
      type(carma_type), target :: carma
      type(carma_type), pointer :: carma_ptr
      type(carmastate_type) :: cstate

      ! Model dimensions
      integer :: NZ, NY, NX, NZP1, NELEM, NGROUP, NBIN, NSOLUTE, NGAS, NWAVE
      integer, parameter :: LUNOPRT = 6

      ! Grid and atmospheric variables
      real(kind=c_double), allocatable :: lat(:), lon(:)
      real(kind=c_double), allocatable :: zc(:), zl(:), p(:), pl(:), t(:), rhoa(:)
      real(kind=c_double), allocatable :: t_orig(:)
      real(kind=c_double) :: time
      real(kind=c_double) :: dtime
      integer :: nstep
      real(kind=c_double) :: deltaz, zmin

      ! Gas and particle variables
      real(kind=c_double), allocatable :: mmr(:,:,:)
      real(kind=c_double), allocatable :: mmr_gas(:,:)  ! Keep for potential future use

      ! Loop indices
      integer :: i, iy, ix, istep, igas, ielem, ibin, igroup

      ! Physical constants and parameters from aluminum test
      real(kind=c_double), parameter :: rmin = 21.5e-6_c_double
      real(kind=c_double), parameter :: rmon = 21.5e-6_c_double
      real(kind=c_double), parameter :: rmrat = 2.0_c_double
      real(kind=c_double), parameter :: RHO_ALUMINUM = 3.95_c_double   ! dry density of aluminum particles (g/cm3)
      real(kind=c_double), parameter :: falpha = 1.0_c_double           ! satellite aerosol fractal packing coefficient
      integer, parameter :: I_ALUMINUM = 1
      integer, parameter :: I_GRP_ALUM = 1
      integer, parameter :: I_ELEM_ALUM = 1

      ! Group parameters
      type(carma_group_config_t), pointer :: group_config(:)
      character(len=:), allocatable :: group_name, group_short_name
      real(kind=real64), pointer :: df(:) ! fractal dimension per group (NBINS)

      ! Element parameters
      type(carma_element_config_t), pointer :: element_config(:)
      character(len=:), allocatable :: element_name, element_short_name
      real(real64), pointer :: rhobin(:) ! rho bin for element (NBINS)
      real(real64), pointer :: arat(:) ! area ratio for element (NBINS)

      rc = 0

      ! Set dimensions from parameters
      NZ = int(params%nz)
      NY = int(params%ny)
      NX = int(params%nx)
      NZP1 = NZ + 1
      NELEM = int(params%nelem)
      NGROUP = int(params%ngroup)
      NBIN = int(params%nbin)
      NSOLUTE = int(params%nsolute)
      NGAS = int(params%ngas)
      NWAVE = int(params%nwave)
      dtime = real(params%dtime, kind=real64)
      nstep = int(params%nstep)
      deltaz = real(params%deltaz, kind=real64)
      zmin = real(params%zmin, kind=real64)

      ! Set up simple grid
      iy = 1
      ix = 1

      ! Allocate arrays
      allocate(lat(NY), lon(NX))
      allocate(zc(NZ), zl(NZP1), p(NZ), pl(NZP1), t(NZ), rhoa(NZ))
      allocate(t_orig(NZ))  ! Allocate original temperature array
      allocate(mmr(NZ, NELEM, NBIN))
      if (NGAS > 0) allocate(mmr_gas(NZ, NGAS))

      ! Create the CARMA instance
      call CARMA_Create(carma, NBIN, NELEM, NGROUP, NSOLUTE, NGAS, NWAVE, rc, LUNOPRT=LUNOPRT)
      if (rc /= 0) then
         return
      end if

      carma_ptr => carma

      ! Create groups based on configuration
      if (c_associated(params%groups)) then
         call c_f_pointer(params%groups, group_config, [params%groups_size])
         do igroup = 1, params%groups_size
         associate(group => group_config(igroup))
            group_name = c_to_f_string(group%name, group%name_length)
            group_short_name = c_to_f_string(group%shortname, group%shortname_length)
            call c_f_pointer(group%df, df, [group%df_size])
            call CARMAGROUP_Create(carma, int(group%id), group_name, real(group%rmin, kind=real64), &
               real(group%rmrat, kind=real64), &
               int(group%ishape), real(group%eshape, kind=real64), logical(group%is_ice), rc,&
               is_fractal=logical(group%is_fractal), &
               irhswell=I_NO_SWELLING, do_mie=logical(group%do_mie), do_wetdep=logical(group%do_wetdep), &
               do_drydep=logical(group%do_drydep), do_vtran=logical(group%do_vtran), &
               solfac=real(group%solfac, kind=real64), scavcoef=real(group%scavcoef, kind=real64), &
               shortname=group_short_name, rmon=real(group%rmon, kind=real64), df=df, falpha=real(group%falpha, kind=real64), &
               is_sulfate=.false.)
            if (rc /= 0) return
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
            call c_f_pointer(elem%rhobin, rhobin, [elem%rhobin_size])
            call c_f_pointer(elem%arat, arat, [elem%arat_size])
            call CARMAELEMENT_Create(carma, int(elem%id), int(elem%igroup), element_name, real(elem%rho, kind=real64), &
               int(elem%itype), int(elem%icomposition), rc, shortname=element_short_name, isolute=int(elem%isolute), &
               rhobin=rhobin, arat=arat, kappa=real(elem%kappa, kind=real64), isShell=logical(elem%isShell))
            if (rc /= 0) return
         end associate
         end do
      end if

      ! Setup CARMA processes - coagulation for aluminum particles
      if (NGROUP >= 1) then
         call CARMA_AddCoagulation(carma, I_GRP_ALUM, I_GRP_ALUM, I_GRP_ALUM, I_COLLEC_FUCHS, rc)
         if (rc /= 0) then
            return
         end if
      end if

      ! Initialize CARMA with coagulation settings matching test_aluminum_simple
      call CARMA_Initialize(carma, rc, do_grow=.false., do_coag=.true., do_substep=.false., do_vtran=.FALSE.)
      if (rc /= 0) then
         return
      end if


      ! Set up atmospheric conditions matching test_aluminum_simple
      lat(iy) = 0.0_c_double
      lon(ix) = -105.0_c_double

      ! Vertical grid setup
      do i = 1, NZ
         zc(i) = zmin + (deltaz * (i - 0.5_c_double))
      end do

      call GetStandardAtmosphere(zc, p=p, t=t)

      do i = 1, NZP1
         zl(i) = zmin + ((i - 1) * deltaz)
      end do
      call GetStandardAtmosphere(zl, p=pl)

      ! Set up initial conditions
      rhoa(:) = (p(:) * 10.0_c_double) / (R_AIR * t(:)) * (1e-3_c_double * 1e6_c_double)

      ! Store original temperature for told parameter
      t_orig(:) = t(:)

      ! Initialize gas mixing ratios (no gases in aluminum test)
      if (NGAS > 0) then
         mmr_gas(:,:) = 0.0_c_double
      end if

      ! Initialize particle mixing ratios with aluminum concentration
      mmr(:,:,:) = 5e9_c_double / (deltaz * 2.57474699e14_c_double) / rhoa(1)

      ! Time integration loop - start from step 2 to match test_aluminum_simple
      carma_time_integration: do istep = 2, nstep + 1

         ! Calculate the model time
         time = (istep - 1) * dtime

         ! Create a CARMASTATE for this column
         call CARMASTATE_Create(cstate, carma_ptr, time, dtime, NZ, &
            I_CART, lat(iy), lon(ix), &
            zc(:), zl(:), &
            p(:),  pl(:), &
            t(:), rc, &
            told=t(:))
         if (rc /= 0) then
            return
         end if

         ! Send the bin mmrs to CARMA
         do ielem = 1, NELEM
            do ibin = 1, NBIN
               call CARMASTATE_SetBin(cstate, ielem, ibin, mmr(:,ielem,ibin), rc)
               if (rc /= 0) then
                  return
               end if
            end do
         end do

         ! Execute the time step
         call CARMASTATE_Step(cstate, rc)
         if (rc /= 0) then
            return
         end if

         ! Get the updated bin mmr
         do ielem = 1, NELEM
            do ibin = 1, NBIN
               call CARMASTATE_GetBin(cstate, ielem, ibin, mmr(:,ielem,ibin), rc)
               if (rc /= 0) then
                  return
               end if
            end do
         end do

      end do carma_time_integration

      ! Transfer output data to C++ if output pointer is provided
      ! This must happen before the CARMA state is destroyed
      if (present(c_output_ptr) .and. c_associated(c_output_ptr)) then
         call transfer_carma_output_data(c_output_ptr, &
            cstate, carma_ptr, params, &
            NZ, NY, NX, NELEM, NGROUP, NBIN, NGAS, nstep, &
            real(time, kind=8), int(nstep), &
            lat, lon, zc, zl, p, t, rhoa, deltaz)
      end if

      ! Clean up the carma state for this step
      call CARMASTATE_Destroy(cstate, rc)
      if (rc /= 0) then
         return
      end if

      ! Clean up the carma instance
      call CARMA_Destroy(carma, rc)
      if (rc /= 0) then
         return
      end if
      ! Deallocate arrays
      deallocate(lat, lon, zc, zl, p, pl, t, rhoa, t_orig, mmr)
      deallocate(df)
      if (NGAS > 0) deallocate(mmr_gas)

   end subroutine run_carma_simulation

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine transfer_carma_output_data(c_output_ptr, &
      cstate, carma_ptr, params, &
      nz, ny, nx, nelem, ngroup, nbin, ngas, nstep, &
      current_time, current_step, &
      lat, lon, vertical_center, vertical_levels, pressure, temperature, air_density, deltaz)
      use carma_precision_mod
      use carma_types_mod
      use carma_parameters_mod, only: carma_parameters_t
      use iso_c_binding, only: c_ptr, c_int, c_double, c_loc
      use iso_fortran_env, only: real64

      implicit none

      ! Input parameters
      type(c_ptr), intent(in) :: c_output_ptr
      type(carmastate_type), intent(in) :: cstate
      type(carma_type), intent(in), target :: carma_ptr
      type(carma_parameters_t), intent(in) :: params
      integer, intent(in) :: nz, ny, nx, nelem, ngroup, nbin, ngas, nstep
      real(kind=8), intent(in) :: current_time
      integer, intent(in) :: current_step
      real(kind=c_double), intent(in), target :: lat(ny), lon(nx)
      real(kind=c_double), intent(in), target :: vertical_center(nz), vertical_levels(nz+1)
      real(kind=c_double), intent(in), target :: pressure(nz), temperature(nz), air_density(nz)
      real(kind=c_double), intent(in) :: deltaz
      real(kind=real64), pointer :: extinction_coefficient(:,:,:)

      ! Arrays for CARMA diagnostics output
      real(kind=c_double), allocatable, target :: number_density(:,:), surface_area(:,:), mass_density(:,:)
      real(kind=c_double), allocatable, target :: effective_radius(:,:), effective_radius_wet(:,:), mean_radius(:,:)
      real(kind=c_double), allocatable, target :: nucleation_rate(:,:), mass_mixing_ratio(:,:)
      real(kind=c_double), allocatable, target :: projected_area(:,:), aspect_ratio(:,:), vertical_mass_flux(:,:)
      real(kind=c_double), allocatable, target :: extinction(:,:), optical_depth(:,:)

      ! Bin-resolved arrays
      real(kind=c_double), allocatable, target :: bin_wet_radius(:,:,:), bin_number_density(:,:,:)
      real(kind=c_double), allocatable, target :: bin_density(:,:,:), bin_mass_mixing_ratio(:,:,:)
      real(kind=c_double), allocatable, target :: bin_deposition_velocity(:,:,:)

      ! Additional arrays for missing fields
      real(kind=c_double), allocatable, target :: radiative_heating(:), delta_temperature(:)
      real(kind=c_double), allocatable, target :: gas_mmr(:,:), gas_saturation_liquid(:,:), gas_saturation_ice(:,:)
      real(kind=c_double), allocatable, target :: gas_vapor_pressure_ice(:,:), gas_vapor_pressure_liquid(:,:), gas_weight_percent(:,:)
      real(kind=c_double), allocatable, target :: group_radius(:,:), group_mass(:,:), group_volume(:,:)
      real(kind=c_double), allocatable, target :: group_radius_ratio(:,:), group_aspect_ratio(:,:), group_fractal_dimension(:,:)

      ! Create and populate the output data struct
      type(c_carma_output_data) :: output_data_struct

      ! Allocate arrays for carmadiags output
      allocate(number_density(nz, ngroup))
      allocate(surface_area(nz, ngroup))
      allocate(mass_density(nz, ngroup))
      allocate(effective_radius(nz, ngroup))
      allocate(effective_radius_wet(nz, ngroup))
      allocate(mean_radius(nz, ngroup))
      allocate(nucleation_rate(nz, ngroup))
      allocate(mass_mixing_ratio(nz, ngroup))
      allocate(projected_area(nz, ngroup))
      allocate(aspect_ratio(nz, ngroup))
      allocate(vertical_mass_flux(nz, ngroup))
      allocate(extinction(nz, ngroup))
      allocate(optical_depth(nz, ngroup))

      allocate(bin_wet_radius(nz, ngroup, nbin))
      allocate(bin_number_density(nz, ngroup, nbin))
      allocate(bin_density(nz, ngroup, nbin))
      allocate(bin_mass_mixing_ratio(nz, ngroup, nbin))
      allocate(bin_deposition_velocity(nz, ngroup, nbin))

      extinction_coefficient => null( )
      if (c_associated(params%extinction_coefficient)) then
         call c_f_pointer(params%extinction_coefficient, extinction_coefficient, &
         [params%nwave, params%nbin, params%ngroup])
      end if

      ! Extract diagnostics from CARMA state using carmadiags
      if (associated(extinction_coefficient)) then
         call carmadiags(cstate, carma_ptr, nz, nbin, nelem, ngroup, params%nwave, &
            deltaz, qext=extinction_coefficient, idx_wave=params%idx_wave, &
            nd=number_density, ad=surface_area, md=mass_density, &
            re=effective_radius, rew=effective_radius_wet, rm=mean_radius, &
            jn=nucleation_rate, mr=mass_mixing_ratio, &
            pa=projected_area, ar=aspect_ratio, vm=vertical_mass_flux, &
            ex=extinction, od=optical_depth, &
            wr_bin=bin_wet_radius, nd_bin=bin_number_density, &
            ro_bin=bin_density, mr_bin=bin_mass_mixing_ratio, &
            vd_bin=bin_deposition_velocity)
      else
         call carmadiags(cstate, carma_ptr, nz, nbin, nelem, ngroup, 0, &
            deltaz, &
            nd=number_density, ad=surface_area, md=mass_density, &
            re=effective_radius, rew=effective_radius_wet, rm=mean_radius, &
            jn=nucleation_rate, mr=mass_mixing_ratio, &
            pa=projected_area, ar=aspect_ratio, vm=vertical_mass_flux, &
            ex=extinction, od=optical_depth, &
            wr_bin=bin_wet_radius, nd_bin=bin_number_density, &
            ro_bin=bin_density, mr_bin=bin_mass_mixing_ratio, &
            vd_bin=bin_deposition_velocity)
      end if

      allocate(radiative_heating(nz))
      allocate(delta_temperature(nz))
      ! Allocate and initialize atmospheric state variables
      radiative_heating(:) = 0.0_c_double  ! Default value
      delta_temperature(:) = 0.0_c_double  ! Default value

      ! Allocate and initialize gas variables if gases exist
      if (ngas > 0) then
         allocate(gas_mmr(nz, ngas), gas_saturation_liquid(nz, ngas), gas_saturation_ice(nz, ngas))
         allocate(gas_vapor_pressure_ice(nz, ngas), gas_vapor_pressure_liquid(nz, ngas), gas_weight_percent(nz, ngas))
         gas_mmr(:,:) = 0.0_c_double
         gas_saturation_liquid(:,:) = 0.0_c_double
         gas_saturation_ice(:,:) = 0.0_c_double
         gas_vapor_pressure_ice(:,:) = 0.0_c_double
         gas_vapor_pressure_liquid(:,:) = 0.0_c_double
         gas_weight_percent(:,:) = 0.0_c_double
      else
         ! Allocate minimal arrays for zero gas case
         allocate(gas_mmr(nz, 1), gas_saturation_liquid(nz, 1), gas_saturation_ice(nz, 1))
         allocate(gas_vapor_pressure_ice(nz, 1), gas_vapor_pressure_liquid(nz, 1), gas_weight_percent(nz, 1))
         gas_mmr(:,:) = 0.0_c_double
         gas_saturation_liquid(:,:) = 0.0_c_double
         gas_saturation_ice(:,:) = 0.0_c_double
         gas_vapor_pressure_ice(:,:) = 0.0_c_double
         gas_vapor_pressure_liquid(:,:) = 0.0_c_double
         gas_weight_percent(:,:) = 0.0_c_double
      end if

      ! Allocate and initialize group properties with default values
      allocate(group_radius(nbin, ngroup), group_mass(nbin, ngroup), group_volume(nbin, ngroup))
      allocate(group_radius_ratio(nbin, ngroup), group_aspect_ratio(nbin, ngroup), group_fractal_dimension(nbin, ngroup))
      group_radius(:,:) = 0.0_c_double
      group_mass(:,:) = 0.0_c_double
      group_volume(:,:) = 0.0_c_double
      group_radius_ratio(:,:) = 1.0_c_double  ! Default to 1.0
      group_aspect_ratio(:,:) = 1.0_c_double  ! Default to 1.0
      group_fractal_dimension(:,:) = 3.0_c_double  ! Default to 3.0 (spherical)

      output_data_struct%c_output_ptr = c_output_ptr
      output_data_struct%nz = int(nz, c_int)
      output_data_struct%ny = int(ny, c_int)
      output_data_struct%nx = int(nx, c_int)
      output_data_struct%nelem = int(nelem, c_int)
      output_data_struct%ngroup = int(ngroup, c_int)
      output_data_struct%nbin = int(nbin, c_int)
      output_data_struct%ngas = int(ngas, c_int)
      output_data_struct%nstep = int(nstep, c_int)

      ! Set pointers to array data
      output_data_struct%lat = c_loc(lat)
      output_data_struct%lon = c_loc(lon)
      output_data_struct%vertical_center = c_loc(vertical_center)
      output_data_struct%vertical_levels = c_loc(vertical_levels)
      output_data_struct%pressure = c_loc(pressure)
      output_data_struct%temperature = c_loc(temperature)
      output_data_struct%air_density = c_loc(air_density)
      output_data_struct%radiative_heating = c_loc(radiative_heating)
      output_data_struct%delta_temperature = c_loc(delta_temperature)
      output_data_struct%gas_mmr = c_loc(gas_mmr)
      output_data_struct%gas_saturation_liquid = c_loc(gas_saturation_liquid)
      output_data_struct%gas_saturation_ice = c_loc(gas_saturation_ice)
      output_data_struct%gas_vapor_pressure_ice = c_loc(gas_vapor_pressure_ice)
      output_data_struct%gas_vapor_pressure_liquid = c_loc(gas_vapor_pressure_liquid)
      output_data_struct%gas_weight_percent = c_loc(gas_weight_percent)
      output_data_struct%number_density = c_loc(number_density)
      output_data_struct%surface_area = c_loc(surface_area)
      output_data_struct%mass_density = c_loc(mass_density)
      output_data_struct%effective_radius = c_loc(effective_radius)
      output_data_struct%effective_radius_wet = c_loc(effective_radius_wet)
      output_data_struct%mean_radius = c_loc(mean_radius)
      output_data_struct%nucleation_rate = c_loc(nucleation_rate)
      output_data_struct%mass_mixing_ratio = c_loc(mass_mixing_ratio)
      output_data_struct%projected_area = c_loc(projected_area)
      output_data_struct%aspect_ratio = c_loc(aspect_ratio)
      output_data_struct%vertical_mass_flux = c_loc(vertical_mass_flux)
      output_data_struct%extinction = c_loc(extinction)
      output_data_struct%optical_depth = c_loc(optical_depth)
      output_data_struct%bin_wet_radius = c_loc(bin_wet_radius)
      output_data_struct%bin_number_density = c_loc(bin_number_density)
      output_data_struct%bin_density = c_loc(bin_density)
      output_data_struct%bin_mass_mixing_ratio = c_loc(bin_mass_mixing_ratio)
      output_data_struct%bin_deposition_velocity = c_loc(bin_deposition_velocity)
      output_data_struct%group_radius = c_loc(group_radius)
      output_data_struct%group_mass = c_loc(group_mass)
      output_data_struct%group_volume = c_loc(group_volume)
      output_data_struct%group_radius_ratio = c_loc(group_radius_ratio)
      output_data_struct%group_aspect_ratio = c_loc(group_aspect_ratio)
      output_data_struct%group_fractal_dimension = c_loc(group_fractal_dimension)

      ! Call the C++ transfer function with the struct
      call TransferCarmaOutputToCpp(output_data_struct)

      ! Clean up
      deallocate(number_density, surface_area, mass_density)
      deallocate(effective_radius, effective_radius_wet, mean_radius)
      deallocate(nucleation_rate, mass_mixing_ratio, projected_area, aspect_ratio)
      deallocate(vertical_mass_flux, extinction, optical_depth)
      deallocate(bin_wet_radius, bin_number_density, bin_density)
      deallocate(bin_mass_mixing_ratio, bin_deposition_velocity)
      deallocate(radiative_heating, delta_temperature)
      deallocate(gas_mmr, gas_saturation_liquid, gas_saturation_ice)
      deallocate(gas_vapor_pressure_ice, gas_vapor_pressure_liquid, gas_weight_percent)
      deallocate(group_radius, group_mass, group_volume)
      deallocate(group_radius_ratio, group_aspect_ratio, group_fractal_dimension)

   end subroutine transfer_carma_output_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine carmadiags(cstate, carma, nlev, nbin, nelem, ngroup, nwave, &
      dz, qext, idx_wave, &
      nd, ad, md, re, rew, rm, jn, mr, pa, ar, vm, ex, od,&
      wr_bin, nd_bin, ro_bin, mr_bin, vd_bin)
      use carma_precision_mod
      use carma_enums_mod
      use carma_constants_mod
      use carma_types_mod
      use carmaelement_mod
      use carmagroup_mod
      use carmastate_mod

      ! Input
      type(carma_type), intent(in)       :: carma
      type(carmastate_type), intent(in)  :: cstate
      integer, intent(in)                :: nlev, nbin, nelem, ngroup, nwave
      integer, intent(in), optional      :: idx_wave
      real(kind=c_double), intent(in)           :: dz
      real(kind=c_double), intent(in), optional :: qext(nwave, nbin, ngroup)

      ! Output
      real(kind=c_double), intent(out), optional :: nd(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: ad(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: md(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: re(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: rew(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: rm(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: jn(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: mr(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: pa(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: ar(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: vm(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: ex(nlev, ngroup)
      real(kind=c_double), intent(out), optional :: od(nlev, ngroup)

      real(kind=c_double), intent(out), optional :: wr_bin(nlev, ngroup, nbin)
      real(kind=c_double), intent(out), optional :: nd_bin(nlev, ngroup, nbin)
      real(kind=c_double), intent(out), optional :: ro_bin(nlev, ngroup, nbin)
      real(kind=c_double), intent(out), optional :: mr_bin(nlev, ngroup, nbin)
      real(kind=c_double), intent(out), optional :: vd_bin(nlev, ngroup, nbin)

      ! Local Variables
      integer      :: igroup, ielem, ibin, ienconc, rc, cnsttype, maxbin
      real(kind=c_double) :: r(nbin), rmass(nbin), rrat(nbin), arat(nbin)
      logical      :: is_cloud, is_ice, grp_do_drydep
      real(kind=c_double) :: vqext(nbin, ngroup)

      real(kind=c_double)           :: mmr(nlev)             !! the bin mass mixing ratio [kg/kg]
      real(kind=c_double)           :: totalmmr(nlev)        !! mmr of the entire particle (kg/m3)
      real(kind=c_double)           :: numberDensity(nlev)   !! number density [#/cm3]
      real(kind=c_double)           :: nucleationRate(nlev)  !! nucleation rate [1/cm3/s]
      real(kind=c_double)           :: rwet(nlev)           !! wet particle radius [cm]
      real(kind=c_double)           :: rhop_wet(nlev)        !! wet particle density [g/cm3]
      real(kind=c_double)           :: dd                    !! particle sedimentation mass flux to surface [kg/m2/s]
      real(kind=c_double)           :: vd                    !! deposition velocity [cm/s]
      real(kind=c_double)           :: vf(nlev+1)            !! fall velocity [cm/s]
      real(kind=c_double)           :: dtpart(nlev)          !! delta particle temperature [K]
      real(kind=c_double)           :: re2(nlev, ngroup), re3(nlev, ngroup)
      real(kind=c_double)           :: rew2(nlev, ngroup), rew3(nlev, ngroup)


      real(kind=f) :: vnd(nlev, ngroup)
      real(kind=f) :: vad(nlev, ngroup)
      real(kind=f) :: vmd(nlev, ngroup)
      real(kind=f) :: vre(nlev, ngroup)
      real(kind=f) :: vrew(nlev, ngroup)
      real(kind=f) :: vrm(nlev, ngroup)
      real(kind=f) :: vjn(nlev, ngroup)
      real(kind=f) :: vmr(nlev, ngroup)
      real(kind=f) :: vpa(nlev, ngroup)
      real(kind=f) :: var(nlev, ngroup)
      real(kind=f) :: vvm(nlev, ngroup)
      real(kind=f) :: vex(nlev, ngroup)
      real(kind=f) :: vod(nlev, ngroup)

      real(kind=f) :: vwr_bin(nlev, ngroup, nbin)
      real(kind=f) :: vnd_bin(nlev, ngroup, nbin)
      real(kind=f) :: vro_bin(nlev, ngroup, nbin)
      real(kind=f) :: vmr_bin(nlev, ngroup, nbin)
      real(kind=f) :: vvd_bin(nlev, ngroup, nbin)


      ! Initialization
      re2(:, :) = 0._f
      re3(:, :) = 0._f
      rew2(:, :) = 0._f
      rew3(:, :) = 0._f

      vnd(:, :)   = 0._f
      vad(:, :)   = 0._f
      vmd(:, :)   = 0._f
      vre(:, :)   = 0._f
      vrew(:, :)  = 0._f
      vrm(:, :)   = 0._f
      vjn(:, :)   = 0._f
      vmr(:, :)   = 0._f
      vpa(:, :)   = 0._f
      var(:, :)   = 0._f
      vvm(:, :)   = 0._f
      vex(:, :)   = 0._f
      vod(:, :)   = 0._f
      vwr_bin(:, :, :) = 0._f
      vnd_bin(:, :, :) = 0._f
      vro_bin(:, :, :) = 0._f
      vvd_bin(:, :, :) = 0._f

      vqext(:, :) = 2.0_f
      if (present(qext)) vqext(:, :) = qext(idx_wave, :, :)

      ! Calculation
      do ielem = 1, nelem

         call CARMAELEMENT_Get(carma, ielem, rc, igroup=igroup)
         if (rc /=0) stop "    *** CARMAELEMENT_Get FAILED ***"

         call CARMAGROUP_Get(carma, igroup, rc, ienconc=ienconc, cnsttype=cnsttype, r=r, rmass=rmass, maxbin=maxbin, &
            is_cloud=is_cloud, is_ice=is_ice, do_drydep=grp_do_drydep, rrat=rrat, arat=arat)

         do ibin = 1, nbin
            call CARMASTATE_GetBin(cstate, ielem, ibin, mmr, rc, &
               numberDensity=numberDensity, nucleationRate=nucleationRate, r_wet=rwet, rhop_wet=rhop_wet, &
               sedimentationflux=dd, vd=vd, vf=vf, dtpart=dtpart, totalmmr=totalmmr)

            if (rc /=0) stop "    *** CARMASTATE_GetBin FAILED ***"

            if (ielem == ienconc) then

               vnd(:, igroup)   = vnd(:, igroup)  + numberDensity(:)
               re2(:, igroup)   = re2(:, igroup)  + numberDensity(:) * ((r(ibin)*rrat(ibin))**2)
               re3(:, igroup)   = re3(:, igroup)  + numberDensity(:) * ((r(ibin)*rrat(ibin))**3)
               rew2(:, igroup)  = rew2(:, igroup) + numberDensity(:) * ((rwet*rrat(ibin))**2)
               rew3(:, igroup)  = rew3(:, igroup) + numberDensity(:) * ((rwet*rrat(ibin))**3)
               vad(:, igroup)   = vad(:, igroup)  + numberDensity(:) * 4.0_f * PI * (rwet**2) * 1.0e8_f
               vmd(:, igroup)   = vmd(:, igroup)  + numberDensity(:) * rmass(ibin)
               vmr(:, igroup)   = vmr(:, igroup)  + totalmmr(:)
               vpa(:, igroup)   = vpa(:, igroup)  + numberDensity(:) * PI * ((rwet * rrat(ibin))**2) * arat(ibin)
               vvm(:, igroup)   = vvm(:, igroup)  + numberDensity(:) * rmass(ibin) * vf(2:)
               vex(:, igroup)   = vex(:, igroup)  + numberDensity(:) * vqext(ibin,igroup) * PI * (r(ibin)**2) * 1e5_f
               vod(:, igroup)   = vod(:, igroup)  + numberDensity(:) * vqext(ibin,igroup) * PI * (r(ibin)**2) * dz * 100._f

               vmr_bin(:, igroup, ibin) = totalmmr(:)
               vnd_bin(:, igroup, ibin) = numberDensity(:)
               vro_bin(:, igroup, ibin) = rhop_wet(:)
               vwr_bin(:, igroup, ibin) = rwet(:)* 1e4_f

               vjn(:, igroup)  = vjn(:, igroup)  + nucleationRate(:)

               if (cnsttype == I_CNSTTYPE_PROGNOSTIC) then
                  if (ibin <= maxbin) then
                     if (grp_do_drydep) then
                        vvd_bin(:, igroup, ibin) = - vd / 100._f
                     end if
                  end if
               end if

            end if
         end do   ! loop over bins

         if (ielem == ienconc) then
            where (re2(:, igroup) > 0.0_f)
               vre(:, igroup)  = (re3(:, igroup) / re2(:, igroup)) * 1e4_f
               vrew(:, igroup) = (rew3(:, igroup) / rew2(:, igroup)) * 1e4_f
               vrm(:, igroup)  = (3._f / 4._f) * (vmd(:, igroup)  / (0.917_f * vpa(:, igroup))) * 1e4_f
               var(:, igroup)  = vpa(:, igroup) / PI / rew2(:, igroup)
            end where

            where (vmd(:, igroup) > 0.0_f)
               vvm(:, igroup) = vvm(:, igroup) / vmd(:, igroup)
            end where
         end if

      end do      ! loop over elements

      if (present(nd))     nd(:, :)  = vnd(:, :)
      if (present(ad))     ad(:, :)  = vad(:, :)
      if (present(md))     md(:, :)  = vmd(:, :)
      if (present(re))    re(:, :)  = vre(:, :)
      if (present(rew))    rew(:, :) = vrew(:, :)
      if (present(rm))     rm(:, :)  = vrm(:, :)
      if (present(jn))     jn(:, :)  = vjn(:, :)
      if (present(mr))     mr(:, :)  = vmr(:, :)
      if (present(pa))     pa(:, :)  = vpa(:, :)
      if (present(ar))     ar(:, :)  = var(:, :)
      if (present(vm))     vm(:, :)  = vvm(:, :)
      if (present(ex))     ex(:, :)  = vex(:, :)
      if (present(od))     od(:, :)  = vod(:, :)
      if (present(wr_bin)) wr_bin(:, :, :) = vwr_bin(:, :, :)
      if (present(nd_bin)) nd_bin(:, :, :) = vnd_bin(:, :, :)
      if (present(ro_bin)) ro_bin(:, :, :) = vro_bin(:, :, :)
      if (present(mr_bin)) mr_bin(:, :, :) = vmr_bin(:, :, :)
      if (present(vd_bin)) vd_bin(:, :, :) = vvd_bin(:, :, :)

   end subroutine carmadiags

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module carma_interface
