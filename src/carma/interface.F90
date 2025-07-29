! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module carma_interface

   use iso_c_binding, only: c_int, c_double, c_ptr, c_char, c_null_char, &
      c_loc, c_f_pointer, c_associated, c_null_ptr, c_bool
   implicit none

   private

   integer, parameter, public :: ERROR_MEMORY_ALLOCATION = 97
   integer, parameter, public :: ERROR_DIMENSION_MISMATCH = 98

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
   end type c_carma_output_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   ! Interface to the C++ TransferCarmaOutputToCpp function
   interface
      subroutine TransferCarmaOutputToCpp(output_data, nz, ny, nx, nbin, nelem, ngroup) &
         bind(C, name="TransferCarmaOutputToCpp")
         use iso_c_binding, only: c_int
         import :: c_carma_output_data
         type(c_carma_output_data), intent(in) :: output_data
         integer(c_int), value, intent(in) :: nz, ny, nx, nbin, nelem, ngroup
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

   subroutine internal_run_carma(params, carma_cptr, c_output, rc) &
      bind(C, name="InternalRunCarma")
      use iso_c_binding, only: c_int, c_ptr, c_f_pointer
      use carma_parameters_mod, only: carma_parameters_t

      type(carma_parameters_t), intent(in)  :: params
      type(c_ptr), value,       intent(in)  :: carma_cptr
      type(c_ptr), value,       intent(in)  :: c_output
      integer(c_int),           intent(out) :: rc

      rc = 0

      ! Run CARMA simulation with optional output
      call run_carma_simulation(params, carma_cptr, rc, c_output)

   end subroutine internal_run_carma

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

      ! Check if carma_cptr is associated
      if (c_associated(carma_cptr)) then
         call c_f_pointer(carma_cptr, carma)

        ! Clean up the carma instance
        call CARMA_Destroy(carma, rc)
        if (rc /= 0) then
            print *, "Error destroying CARMA instance"
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
      integer :: NZ, NY, NX, NZP1, NELEM, NGROUP, NBIN, NSOLUTE, NGAS, NWAVE

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
         rc = 1
         print *, "Error allocating CARMA instance"
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
         print *, "Error creating CARMA instance"
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
               carma, &
               igroup, &
               group_name, &
               real(group%rmin, kind=real64) * 100.0_real64, & ! Convert m to cm
               real(group%rmrat, kind=real64), &
               int(group%ishape), &
               real(group%eshape, kind=real64), &
               logical(group%is_ice), &
               rc, &
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
               if (elem%rhobin_size > 0) then
                 call c_f_pointer(elem%rhobin, rhobin, [elem%rhobin_size])
                 rhobin(:) = rhobin(:) * 0.001_real64 ! Convert kg m-3 to g cm-3
                 if (elem%rhobin_size /= NBIN) then
                    print *, "Error: rhobin size does not match NBIN"
                    rc = ERROR_DIMENSION_MISMATCH
                    return
                 end if
               else
                 rhobin => null( )
               end if
               if (elem%arat_size > 0) then
                  call c_f_pointer(elem%arat, arat, [elem%arat_size])
                  if (elem%arat_size /= NBIN) then
                     print *, "Error: arat size does not match NBIN"
                     rc = ERROR_DIMENSION_MISMATCH
                     return
                  end if
               else
                  arat => null( )
               end if
               if (elem%refidx_dim_1_size > 0 .and. elem%refidx_dim_2_size > 0) then
                  if (elem%refidx_dim_2_size /= NWAVE) then
                     print *, "Error: refidx_dim_2_size does not match NWAVE"
                     rc = ERROR_DIMENSION_MISMATCH
                     return
                  end if
                  if (elem%refidx_dim_1_size /= params%number_of_refractive_indices) then
                     print *, "Error: refidx_dim_1_size does not match number_of_refractive_indices"
                     rc = ERROR_DIMENSION_MISMATCH
                     return
                  end if
                  call c_f_pointer(elem%refidx, refidx_t, [elem%refidx_dim_2_size, elem%refidx_dim_1_size])
                  allocate(refidx(elem%refidx_dim_2_size, elem%refidx_dim_1_size), stat=alloc_stat)
                  if (alloc_stat /= 0) then
                     print *, "Error allocating refractive index array"
                     rc = ERROR_MEMORY_ALLOCATION
                     return
                  end if
                  do iwave = 1, NWAVE
                     refidx(iwave,:) = cmplx(refidx_t(iwave,1:params%number_of_refractive_indices)%real_part, &
                                             refidx_t(iwave,1:params%number_of_refractive_indices)%imag_part, kind=real64)
                  end do
               end if
               call CARMAELEMENT_Create( &
                  carma, &
                  ielem, &
                  int(elem%igroup), &
                  element_name, &
                  real(elem%rho, kind=real64) * 0.001_real64, & ! Convert kg m-3 to g cm-3
                  int(elem%itype), &
                  int(elem%icomposition), &
                  rc, &
                  shortname=element_short_name, &
                  isolute=int(elem%isolute), &
                  rhobin=rhobin, &
                  arat=arat, &
                  kappa=real(elem%kappa, kind=real64), &
                  refidx=refidx, &
                  isShell=logical(elem%isShell))
               if (rc /= 0) then
                  print *, "Error creating CARMA element"
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
                  carma, &
                  isolute, &
                  solute_name, &
                  int(solute%ions), &
                  real(solute%wtmol, kind=real64), &
                  real(solute%rho, kind=real64), &
                  rc, &
                  shortname=solute_short_name)
               if (rc /= 0) then
                  print *, "Error creating CARMA solute"
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
                     print *, "Error: refidx_dim_2_size does not match NWAVE"
                     rc = ERROR_DIMENSION_MISMATCH
                     return
                  end if
                  if (gas%refidx_dim_1_size /= params%number_of_refractive_indices) then
                     print *, "Error: refidx_dim_1_size does not match number_of_refractive_indices"
                     rc = ERROR_DIMENSION_MISMATCH
                     return
                  end if
                  call c_f_pointer(gas%refidx, gas_refidx_t, [gas%refidx_dim_2_size, gas%refidx_dim_1_size])
                  allocate(gas_refidx(gas%refidx_dim_2_size, gas%refidx_dim_1_size), stat=alloc_stat)
                  if (alloc_stat /= 0) then
                     print *, "Error allocating refractive index array"
                     rc = ERROR_MEMORY_ALLOCATION
                     return
                  end if
                  do iwave = 1, NWAVE
                     gas_refidx(iwave,:) = cmplx(gas_refidx_t(iwave,1:params%number_of_refractive_indices)%real_part, &
                                                 gas_refidx_t(iwave,1:params%number_of_refractive_indices)%imag_part, kind=real64)
                  end do
               end if
               call CARMAGAS_Create( &
                  carma, &
                  igas, &
                  gas_name, &
                  real(gas%wtmol, kind=real64), &
                  int(gas%ivaprtn), &
                  int(gas%icomposition), &
                  rc, &
                  shortname=gas_short_name, &
                  dgc_threshold=real(gas%dgc_threshold, kind=real64), &
                  ds_threshold=real(gas%ds_threshold, kind=real64), &
                  refidx=gas_refidx)
               if (rc /= 0) then
                  print *, "Error creating CARMA gas"
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
               call CARMA_AddCoagulation( &
                  carma, &
                  int(coag%igroup1), &
                  int(coag%igroup2), &
                  int(coag%igroup3), &
                  int(coag%algorithm), &
                  rc, &
                  ck0=real(coag%ck0, kind=real64), &
                  grav_e_coll0=real(coag%grav_e_coll0, kind=real64), &
                  use_ccd=logical(coag%use_ccd))
               if (rc /= 0) then
                  print *, "Error adding CARMA coagulation"
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
                  carma, &
                  int(growth%ielem), &
                  int(growth%igas), &
                  rc)
               if (rc /= 0) then
                  print *, "Error adding CARMA growth"
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
                  carma, &
                  int(nucleation%ielemfrom), &
                  int(nucleation%ielemto), &
                  int(nucleation%algorithm), &
                  real(nucleation%rlh_nuc, kind=real64) * 1.0e4_real64, & ! convert m2 s-2 to cm2 s-2
                  rc, &
                  igas=int(nucleation%igas), &
                  ievp2elem=int(nucleation%ievp2elem))
               if (rc /= 0) then
                  print *, "Error adding CARMA nucleation"
                  return
               end if
            end associate
         end do
      end if

      ! Initialize CARMA with coagulation settings matching test_aluminum_simple
      call CARMA_Initialize(carma, rc, do_grow=.false., do_coag=.true., do_substep=.false., do_vtran=.FALSE.)
      if (rc /= 0) then
         print *, "Error initializing CARMA"
         return
      end if

      carma_cptr = c_loc(carma)

   end subroutine create_carma_instance

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine run_carma_simulation(params, carma_cptr, rc, c_output_ptr)
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

      type(carma_parameters_t), intent(in)  :: params
      type(c_ptr),              intent(in)  :: carma_cptr
      integer,                  intent(out) :: rc
      type(c_ptr), optional,    intent(in)  :: c_output_ptr

      ! Local variables for CARMA simulation
      type(carma_type), pointer :: carma
      type(carmastate_type)     :: cstate

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

      call c_f_pointer(carma_cptr, carma)

      ! Set dimensions from parameters
      NZ = int(params%nz)
      NY = int(params%ny)
      NX = int(params%nx)
      NZP1 = NZ + 1
      NELEM = int(params%elements_size)
      NGROUP = int(params%groups_size)
      NBIN = int(params%nbin)
      NSOLUTE = int(params%solutes_size)
      NGAS = int(params%gases_size)
      NWAVE = int(params%wavelength_bin_size)
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
         call CARMASTATE_Create(cstate, carma, time, dtime, NZ, &
            I_CART, lat(iy), lon(ix), &
            zc(:), zl(:), &
            p(:),  pl(:), &
            t(:), rc, &
            told=t(:))
         if (rc /= 0) then
            print *, "Error creating CARMA state"
            return
         end if

         ! Send the bin mmrs to CARMA
         do ielem = 1, NELEM
            do ibin = 1, NBIN
               call CARMASTATE_SetBin(cstate, ielem, ibin, mmr(:,ielem,ibin), rc)
               if (rc /= 0) then
                  print *, "Error setting CARMA state bin"
                  return
               end if
            end do
         end do

         ! Execute the time step
         call CARMASTATE_Step(cstate, rc)
         if (rc /= 0) then
            print *, "Error executing CARMA state step"
            return
         end if

         ! Get the updated bin mmr
         do ielem = 1, NELEM
            do ibin = 1, NBIN
               call CARMASTATE_GetBin(cstate, ielem, ibin, mmr(:,ielem,ibin), rc)
               if (rc /= 0) then
                  print *, "Error getting CARMA state bin"
                  return
               end if
            end do
         end do

      end do carma_time_integration

      ! Transfer output data to C++ if output pointer is provided
      ! This must happen before the CARMA state is destroyed
      if (present(c_output_ptr) .and. c_associated(c_output_ptr)) then
         call transfer_carma_output_data(c_output_ptr, &
            cstate, carma, params, &
            NZ, NY, NX, NELEM, NGROUP, NBIN, NGAS, nstep, &
            real(time, kind=8), int(nstep), &
            lat, lon, zc, zl, p, t, rhoa, deltaz)
      end if

      ! Clean up the carma state for this step
      call CARMASTATE_Destroy(cstate, rc)
      if (rc /= 0) then
         print *, "Error destroying CARMA state"
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

      ! Create and populate the output data struct
      type(c_carma_output_data) :: output_data_struct

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

      ! Call the C++ transfer function with the struct and dimensions
      call TransferCarmaOutputToCpp(output_data_struct, &
         int(nz, c_int), int(ny, c_int), int(nx, c_int), int(nbin, c_int), &
         int(nelem, c_int), int(ngroup, c_int))

      ! Clean up fundamental data arrays
      deallocate(pc, mmr, r_wet, rhop_wet, vf, nucleation, vd)
      deallocate(r_dry, rmass, rrat, arat)
      deallocate(ienconc, constituent_type, maxbin)

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
            print *, "Error getting CARMA element group"
            return 
         end if

         do ibin = 1, nbin
            ! Get fundamental bin data - all the data needed for Python calculations
            call CARMASTATE_GetBin(cstate, ielem, ibin, mmr_bin, rc, &
               numberDensity=numberDensity, r_wet=rwet_bin, rhop_wet=rhop_wet_bin, &
               vf=vf_bin, nucleationRate=nucleationRate, vd=vd_scalar)
            if (rc /= 0) then
               print *, "Error getting CARMA state bin"
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
            print *, "Error getting CARMA group properties"
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
