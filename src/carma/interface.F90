! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module carma_interface

   use iso_c_binding, only: c_int, c_double, c_ptr, c_char, c_null_char, &
      c_loc, c_f_pointer, c_associated, c_null_ptr, c_bool
   implicit none

   private

   ! C-compatible structure for CARMA fundamental data
   ! Contains only the minimal essential data needed for Python calculations
   type, bind(C) :: c_carma_output_data
      type(c_ptr) :: c_output_ptr

      ! Grid and atmospheric data
      type(c_ptr) :: lat
      type(c_ptr) :: lon
      type(c_ptr) :: vertical_center
      type(c_ptr) :: vertical_levels
      type(c_ptr) :: pressure
      type(c_ptr) :: temperature
      type(c_ptr) :: air_density

      ! Fundamental particle state data [nz, nbin, nelem]
      type(c_ptr) :: particle_concentration     ! number density [#/cm³]
      type(c_ptr) :: mass_mixing_ratio          ! mass mixing ratio [kg/kg]

      ! Bin-level particle properties [nz, nbin, ngroup]
      type(c_ptr) :: wet_radius                 ! wet particle radius [cm]
      type(c_ptr) :: wet_density                ! wet particle density [g/cm³]
      type(c_ptr) :: fall_velocity              ! fall velocity [cm/s] (nz+1, nbin, ngroup)
      type(c_ptr) :: nucleation_rate            ! nucleation rate [1/cm³/s]
      type(c_ptr) :: deposition_velocity        ! deposition velocity [cm/s]

      ! Group configuration data [nbin, ngroup]
      type(c_ptr) :: dry_radius                 ! dry particle radius [cm]
      type(c_ptr) :: mass_per_bin               ! particle mass [g]
      type(c_ptr) :: radius_ratio               ! radius ratio
      type(c_ptr) :: area_ratio                 ! area ratio

      ! Group mapping data
      type(c_ptr) :: group_particle_number_concentration      ! concentration element per group [ngroup]

      ! Group properties
      type(c_ptr) :: constituent_type           ! constituent type per group [ngroup]
      type(c_ptr) :: max_prognostic_bin         ! max prognostic bin per group [ngroup]

      ! Optional optical data [nwave, nbin, ngroup] or [1, nbin, ngroup]
      type(c_ptr) :: extinction_efficiency      ! extinction efficiency
   end type c_carma_output_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   ! Interface to the C++ TransferCarmaOutputToCpp function
   interface
      subroutine TransferCarmaOutputToCpp(output_data, nz, ny, nx, nbin, nelem, ngroup, nwave) &
         bind(C, name="TransferCarmaOutputToCpp")
         use iso_c_binding, only: c_int
         import :: c_carma_output_data
         type(c_carma_output_data), intent(in) :: output_data
         integer(c_int), value, intent(in) :: nz, ny, nx, nbin, nelem, ngroup, nwave
      end subroutine TransferCarmaOutputToCpp
   end interface!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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
      integer, parameter :: I_GRP_ALUM = 1

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
      NELEM = int(params%elements_size)
      NGROUP = int(params%groups_size)
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
      else
         carma%f_icollec = I_COLLEC_CONST
         carma%f_icoagop  = I_COAGOP_CONST
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

      ! Fundamental arrays for Python calculations - core state variables
      real(kind=c_double), allocatable, target :: pc(:,:,:)           ! particle concentration [#/cm³]
      real(kind=c_double), allocatable, target :: mmr(:,:,:)          ! mass mixing ratio [kg/kg]

      ! Particle properties [nz, nbin, ngroup]
      real(kind=c_double), allocatable, target :: r_wet(:,:,:)        ! wet radius [cm]
      real(kind=c_double), allocatable, target :: rhop_wet(:,:,:)     ! wet density [g/cm³]
      real(kind=c_double), allocatable, target :: vf(:,:,:)           ! fall velocity [cm/s] (nz+1, nbin, ngroup)
      real(kind=c_double), allocatable, target :: nucleation(:,:,:)   ! nucleation rate [1/cm³/s]
      real(kind=c_double), allocatable, target :: vd(:,:,:)           ! deposition velocity [cm/s]

      ! Group configuration arrays [nbin, ngroup]
      real(kind=c_double), allocatable, target :: r_dry(:,:)          ! dry radius [cm]
      real(kind=c_double), allocatable, target :: rmass(:,:)          ! mass per bin [g]
      real(kind=c_double), allocatable, target :: rrat(:,:)           ! radius ratio
      real(kind=c_double), allocatable, target :: arat(:,:)           ! area ratio

      ! Group mapping and properties
      integer(kind=c_int), allocatable, target :: ienconc(:)          ! concentration element per group
      integer(kind=c_int), allocatable, target :: constituent_type(:) ! constituent type per group
      integer(kind=c_int), allocatable, target :: maxbin(:)           ! max prognostic bin per group

      ! Optional: extinction efficiency if optical calculations needed
      real(kind=c_double), allocatable, target :: qext(:,:,:)         ! extinction efficiency (nwave, nbin, ngroup)

      ! Create and populate the output data struct
      type(c_carma_output_data) :: output_data_struct
      real(kind=real64), pointer :: extinction_coefficient(:,:,:)

      ! Allocate fundamental arrays for Python calculation
      allocate(pc(nz, nbin, nelem))
      allocate(mmr(nz, nbin, nelem))
      allocate(r_wet(nz, nbin, ngroup))
      allocate(rhop_wet(nz, nbin, ngroup))
      allocate(vf(nz+1, nbin, ngroup))
      allocate(nucleation(nz, nbin, ngroup))
      allocate(vd(nz, nbin, ngroup))

      ! Group configuration arrays
      allocate(r_dry(nbin, ngroup))
      allocate(rmass(nbin, ngroup))
      allocate(rrat(nbin, ngroup))
      allocate(arat(nbin, ngroup))

      ! Group mapping and properties arrays
      allocate(ienconc(ngroup))
      allocate(constituent_type(ngroup))
      allocate(maxbin(ngroup))

      ! Initialize arrays
      pc(:,:,:) = 0.0_c_double
      mmr(:,:,:) = 0.0_c_double
      r_wet(:,:,:) = 0.0_c_double
      rhop_wet(:,:,:) = 0.0_c_double
      vf(:,:,:) = 0.0_c_double
      nucleation(:,:,:) = 0.0_c_double
      vd(:,:,:) = 0.0_c_double
      r_dry(:,:) = 0.0_c_double
      rmass(:,:) = 0.0_c_double
      rrat(:,:) = 1.0_c_double
      arat(:,:) = 1.0_c_double
      ienconc(:) = 0
      constituent_type(:) = 0
      maxbin(:) = 0

      ! Extract fundamental data from CARMA state
      call extract_carma_fundamental_data(cstate, carma_ptr, nz, nbin, nelem, ngroup, &
         pc, mmr, r_wet, rhop_wet, vf, nucleation, vd, &
         r_dry, rmass, rrat, arat, &
         ienconc, constituent_type, maxbin)

      ! Handle extinction coefficient if provided
      if (c_associated(params%extinction_coefficient)) then
         allocate(qext(params%nwave, nbin, ngroup))
         call c_f_pointer(params%extinction_coefficient, extinction_coefficient, &
            [params%nwave, params%nbin, ngroup])
         qext(:,:,:) = extinction_coefficient(:,:,:)
      else
         allocate(qext(1, nbin, ngroup))  ! Allocate minimal array even if no optical data
         qext(:,:,:) = 2.0_c_double       ! Default extinction efficiency
      end if

      output_data_struct%c_output_ptr = c_output_ptr

      ! Set pointers to fundamental data arrays
      output_data_struct%lat = c_loc(lat)
      output_data_struct%lon = c_loc(lon)
      output_data_struct%vertical_center = c_loc(vertical_center)
      output_data_struct%vertical_levels = c_loc(vertical_levels)
      output_data_struct%pressure = c_loc(pressure)
      output_data_struct%temperature = c_loc(temperature)
      output_data_struct%air_density = c_loc(air_density)

      ! Core fundamental data arrays - particle state [nz, nbin, nelem]
      output_data_struct%particle_concentration = c_loc(pc)            ! particle concentration [#/cm³]
      output_data_struct%mass_mixing_ratio = c_loc(mmr)                ! mass mixing ratio [kg/kg]

      ! Bin-level particle properties [nz, nbin, ngroup]
      output_data_struct%wet_radius = c_loc(r_wet)                     ! wet radius [cm]
      output_data_struct%wet_density = c_loc(rhop_wet)                 ! wet density [g/cm³]
      output_data_struct%fall_velocity = c_loc(vf)                     ! fall velocity [cm/s]
      output_data_struct%nucleation_rate = c_loc(nucleation)           ! nucleation rate [1/cm³/s]
      output_data_struct%deposition_velocity = c_loc(vd)               ! deposition velocity [cm/s]

      ! Group configuration arrays [nbin, ngroup]
      output_data_struct%dry_radius = c_loc(r_dry)                     ! dry radius [cm]
      output_data_struct%mass_per_bin = c_loc(rmass)                  ! particle mass [g]
      output_data_struct%radius_ratio = c_loc(rrat)                    ! radius ratio
      output_data_struct%area_ratio = c_loc(arat)                      ! area ratio

      ! Group mapping and properties
      output_data_struct%group_particle_number_concentration = c_loc(ienconc)        ! concentration element per group [ngroup]
      output_data_struct%constituent_type = c_loc(constituent_type)  ! constituent type per group [ngroup]
      output_data_struct%max_prognostic_bin = c_loc(maxbin)            ! max prognostic bin per group [ngroup]

      ! Optical data if available
      output_data_struct%extinction_efficiency = c_loc(qext)           ! extinction efficiency [nwave,nbin,ngroup]

      ! Call the C++ transfer function with the struct and dimensions
      call TransferCarmaOutputToCpp(output_data_struct, &
         int(nz, c_int), int(ny, c_int), int(nx, c_int), int(nbin, c_int), &
         int(nelem, c_int), int(ngroup, c_int), int(size(qext, 1), c_int))

      ! Clean up fundamental data arrays
      deallocate(pc, mmr, r_wet, rhop_wet, vf, nucleation, vd)
      deallocate(r_dry, rmass, rrat, arat)
      deallocate(ienconc, constituent_type, maxbin)
      deallocate(qext)

   end subroutine transfer_carma_output_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine extract_carma_fundamental_data(cstate, carma_ptr, nz, nbin, nelem, ngroup, &
      pc, mmr, r_wet, rhop_wet, vf, nucleation, vd, &
      r_dry, rmass, rrat, arat, &
      ienconc, constituent_type, maxbin)
      use carma_precision_mod
      use carma_types_mod
      use carmaelement_mod
      use carmagroup_mod
      use carmastate_mod
      use carma_enums_mod
      use iso_c_binding, only: c_double, c_int, c_bool

      implicit none

      ! Input parameters
      type(carmastate_type), intent(in) :: cstate
      type(carma_type), intent(in) :: carma_ptr
      integer, intent(in) :: nz, nbin, nelem, ngroup

      ! Output arrays - fundamental data for Python
      real(kind=c_double), intent(out) :: pc(nz, nbin, nelem)           ! particle concentration [#/cm³]
      real(kind=c_double), intent(out) :: mmr(nz, nbin, nelem)          ! mass mixing ratio [kg/kg]
      real(kind=c_double), intent(out) :: r_wet(nz, nbin, ngroup)       ! wet radius [cm]
      real(kind=c_double), intent(out) :: rhop_wet(nz, nbin, ngroup)    ! wet density [g/cm³]
      real(kind=c_double), intent(out) :: vf(nz+1, nbin, ngroup)        ! fall velocity [cm/s]
      real(kind=c_double), intent(out) :: nucleation(nz, nbin, ngroup)  ! nucleation rate [1/cm³/s]
      real(kind=c_double), intent(out) :: vd(nz, nbin, ngroup)          ! deposition velocity [cm/s]
      real(kind=c_double), intent(out) :: r_dry(nbin, ngroup)           ! dry radius [cm]
      real(kind=c_double), intent(out) :: rmass(nbin, ngroup)           ! mass per bin [g]
      real(kind=c_double), intent(out) :: rrat(nbin, ngroup)            ! radius ratio
      real(kind=c_double), intent(out) :: arat(nbin, ngroup)            ! area ratio
      integer(kind=c_int), intent(out) :: ienconc(ngroup)               ! concentration element per group
      integer(kind=c_int), intent(out) :: constituent_type(ngroup)      ! constituent type per group
      integer(kind=c_int), intent(out) :: maxbin(ngroup)                ! max prognostic bin per group

      ! Local variables
      integer :: ielem, ibin, igroup, rc
      real(kind=c_double) :: mmr_bin(nz)                                ! temporary for GetBin call
      real(kind=c_double) :: numberDensity(nz), rwet_bin(nz), rhop_wet_bin(nz)
      real(kind=c_double) :: vf_bin(nz+1), nucleationRate(nz), vd_scalar
      real(kind=c_double) :: r_group(nbin), rmass_group(nbin), rrat_group(nbin), arat_group(nbin)
      integer :: ienconc_tmp, maxbin_tmp, cnsttype_tmp

      ! Initialize arrays
      pc(:,:,:) = 0.0_c_double
      mmr(:,:,:) = 0.0_c_double
      r_wet(:,:,:) = 0.0_c_double
      rhop_wet(:,:,:) = 0.0_c_double
      vf(:,:,:) = 0.0_c_double
      nucleation(:,:,:) = 0.0_c_double
      vd(:,:,:) = 0.0_c_double

      ! Extract data for all elements and bins
      do ielem = 1, nelem
         ! Get the group for this element
         call CARMAELEMENT_Get(carma_ptr, ielem, rc, igroup=igroup)
         if (rc /= 0) then
            return 
         end if

         do ibin = 1, nbin
            ! Get fundamental bin data - all the data needed for Python calculations
            call CARMASTATE_GetBin(cstate, ielem, ibin, mmr_bin, rc, &
               numberDensity=numberDensity, r_wet=rwet_bin, rhop_wet=rhop_wet_bin, &
               vf=vf_bin, nucleationRate=nucleationRate, vd=vd_scalar)
            if (rc /= 0) then
               return
            end if

            ! Store the data - only for concentration elements to match carmadiags logic
            pc(:, ibin, ielem) = numberDensity(:)                      ! particle concentration [#/cm³]
            mmr(:, ibin, ielem) = mmr_bin(:)                           ! mass mixing ratio [kg/kg]
            r_wet(:, ibin, igroup) = rwet_bin(:)                       ! wet radius [cm]
            rhop_wet(:, ibin, igroup) = rhop_wet_bin(:)                ! wet density [g/cm³]
            vf(:, ibin, igroup) = vf_bin(:)                            ! fall velocity [cm/s]
            nucleation(:, ibin, igroup) = nucleationRate(:)           ! nucleation rate [1/cm³/s]
            vd(:, ibin, igroup) = vd_scalar                            ! deposition velocity [cm/s]
         end do
      end do

      ! Extract group configuration data
      do igroup = 1, ngroup
         ! Get group properties - all data needed for Python calculations
         call CARMAGROUP_Get(carma_ptr, igroup, rc, ienconc=ienconc_tmp, cnsttype=cnsttype_tmp, &
            r=r_group, rmass=rmass_group, rrat=rrat_group, arat=arat_group, &
            maxbin=maxbin_tmp)
         if (rc /= 0) then
            return
         end if

         ! Store group configuration data
         r_dry(:, igroup) = r_group(:)                                 ! dry radius [cm]
         rmass(:, igroup) = rmass_group(:)                             ! mass per bin [g]
         rrat(:, igroup) = rrat_group(:)                               ! radius ratio
         arat(:, igroup) = arat_group(:)                               ! area ratio
         ienconc(igroup) = ienconc_tmp                                 ! concentration element per group
         constituent_type(igroup) = cnsttype_tmp                      ! constituent type per group
         maxbin(igroup) = maxbin_tmp                                   ! max prognostic bin per group
      end do

   end subroutine extract_carma_fundamental_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module carma_interface
