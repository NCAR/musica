! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module carma_interface

   use iso_c_binding, only: c_int, c_double, c_ptr, c_char, c_null_char, &
      c_loc, c_f_pointer, c_associated
   implicit none

   private

   ! C-compatible structure for CARMA parameters
   ! MUST match the exact order and types of the C++ CARMAParameters struct
   type, bind(C) :: c_carma_parameters
      integer(c_int) :: max_bins = 100
      integer(c_int) :: max_groups = 10

      ! Model dimensions
      integer(c_int) :: nz = 1
      integer(c_int) :: ny = 1
      integer(c_int) :: nx = 1
      integer(c_int) :: nelem = 1
      integer(c_int) :: ngroup = 1
      integer(c_int) :: nbin = 5
      integer(c_int) :: nsolute = 0
      integer(c_int) :: ngas = 0
      integer(c_int) :: nwave = 30

      ! Time stepping parameters
      real(c_double) :: dtime = 1800.0d0
      integer(c_int) :: nstep = 100

      ! Spatial parameters
      real(c_double) :: deltaz = 1000.0d0
      real(c_double) :: zmin = 16500.0d0

   end type c_carma_parameters

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_carma_version(version_ptr, version_length) &
      bind(C, name="InternalGetCarmaVersion")
      use iso_c_binding, only: c_ptr, c_int, c_f_pointer, c_null_char, c_loc, c_char
      use musica_string, only: string_t
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

   subroutine internal_run_carma_with_parameters(c_params, rc) &
      bind(C, name="InternalRunCarmaWithParameters")
      use iso_c_binding, only: c_int
      use carma_parameters_mod, only: carma_parameters_type

      type(c_carma_parameters), intent(in) :: c_params
      integer(c_int), intent(out) :: rc

      ! Convert C parameters to Fortran parameters
      type(carma_parameters_type) :: f_params

      rc = 0

      ! Copy parameters from C to Fortran structure
      call convert_c_to_fortran_params(c_params, f_params)

      print *, "Running CARMA with the following fortran parameters:"
      print *, "max_bins:", f_params%max_bins
      print *, "max_groups:", f_params%max_groups
      print *, "nz:", f_params%nz
      print *, "ny:", f_params%ny
      print *, "nx:", f_params%nx
      print *, "nelem:", f_params%nelem
      print *, "ngroup:", f_params%ngroup
      print *, "nbin:", f_params%nbin
      print *, "nsolute:", f_params%nsolute
      print *, "ngas:", f_params%ngas
      print *, "nwave:", f_params%nwave
      print *, "dtime:", f_params%dtime
      print *, "nstep:", f_params%nstep
      print *, "deltaz:", f_params%deltaz
      print *, "zmin:", f_params%zmin
      print *, ""

      ! Actually run CARMA simulation
      call run_carma_simulation(f_params, rc)

   end subroutine internal_run_carma_with_parameters

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine convert_c_to_fortran_params(c_params, f_params)
      use carma_parameters_mod, only: carma_parameters_type
      use iso_c_binding, only: c_associated

      type(c_carma_parameters), intent(in) :: c_params
      type(carma_parameters_type), intent(out) :: f_params

      ! Maximum values
      f_params%max_bins = c_params%max_bins
      f_params%max_groups = c_params%max_groups

      ! Model dimensions
      f_params%nz = c_params%nz
      f_params%ny = c_params%ny
      f_params%nx = c_params%nx
      f_params%nelem = c_params%nelem
      f_params%ngroup = c_params%ngroup
      f_params%nbin = c_params%nbin
      f_params%nsolute = c_params%nsolute
      f_params%ngas = c_params%ngas
      f_params%nwave = c_params%nwave

      ! Time stepping parameters
      f_params%dtime = real(c_params%dtime, kind=c_double)
      f_params%nstep = c_params%nstep

      ! Spatial parameters
      f_params%deltaz = real(c_params%deltaz, kind=c_double)
      f_params%zmin = real(c_params%zmin, kind=c_double)

   end subroutine convert_c_to_fortran_params

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine run_carma_simulation(f_params, rc)
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
      use carma_parameters_mod, only: carma_parameters_type

      implicit none

      type(carma_parameters_type), intent(in) :: f_params
      integer, intent(out) :: rc

      ! Local variables for CARMA simulation
      type(carma_type), target :: carma
      type(carma_type), pointer :: carma_ptr
      type(carmastate_type) :: cstate

      ! Model dimensions - use parameters from f_params
      integer :: NZ, NY, NX, NZP1, NELEM, NGROUP, NBIN, NSOLUTE, NGAS, NWAVE
      integer, parameter :: LUNOPRT = 6

      ! Grid and atmospheric variables
      real(kind=f), allocatable :: lat(:), lon(:)
      real(kind=f), allocatable :: zc(:), zl(:), p(:), pl(:), t(:), rhoa(:)
      real(kind=f), allocatable :: t_orig(:)  ! Store original temperature for told parameter
      real(kind=f) :: time
      real(kind=f) :: dtime
      integer :: nstep
      real(kind=f) :: deltaz, zmin

      ! Gas and particle variables
      real(kind=f), allocatable :: mmr_gas(:,:), mmr(:,:,:)

      ! Loop indices
      integer :: i, iy, ix, istep, igas, ielem, ibin

      ! Physical constants and parameters
      real(kind=f), parameter :: rmin = 2.e-8_f
      real(kind=f), parameter :: rmrat = 2._f
      real(kind=f), parameter :: RHO_SULFATE = 1.923_f
      integer, parameter :: I_H2SO4 = 1

      rc = 0

      ! Set dimensions from parameters
      NZ = f_params%nz
      NY = f_params%ny
      NX = f_params%nx
      NZP1 = NZ + 1
      NELEM = f_params%nelem
      NGROUP = f_params%ngroup
      NBIN = f_params%nbin
      NSOLUTE = f_params%nsolute
      NGAS = f_params%ngas
      NWAVE = f_params%nwave
      dtime = real(f_params%dtime, kind=f)
      nstep = f_params%nstep
      deltaz = real(f_params%deltaz, kind=f)
      zmin = real(f_params%zmin, kind=f)

      ! Set up simple grid
      iy = 1
      ix = 1

      ! Allocate arrays
      allocate(lat(NY), lon(NX))
      allocate(zc(NZ), zl(NZP1), p(NZ), pl(NZP1), t(NZ), rhoa(NZ))
      allocate(t_orig(NZ))  ! Allocate original temperature array
      allocate(mmr_gas(NZ, NGAS), mmr(NZ, NELEM, NBIN))

      print *, "Creating CARMA instance..."

      ! Create the CARMA instance
      call CARMA_Create(carma, NBIN, NELEM, NGROUP, NSOLUTE, NGAS, NWAVE, rc, &
         LUNOPRT=LUNOPRT)
      if (rc /= 0) then
         print *, "*** CARMA_Create FAILED ***, rc=", rc
         return
      end if

      carma_ptr => carma

      ! Set up a simple test configuration based on aluminum test
      if (NGROUP >= 1 .and. NELEM >= 1) then
         ! Create a simple particle group
         call CARMAGROUP_Create(carma, 1, "test_group", rmin, rmrat, I_SPHERE, 1._f, .false., rc)
         if (rc /= 0) then
            print *, "*** CARMAGROUP_Create FAILED ***, rc=", rc
            return
         end if

         ! Create a simple element
         call CARMAELEMENT_Create(carma, 1, 1, "test_element", RHO_SULFATE, I_INVOLATILE, I_H2SO4, rc)
         if (rc /= 0) then
            print *, "*** CARMAELEMENT_Create FAILED ***, rc=", rc
            return
         end if
      end if

      ! Create gases if specified
      if (NGAS >= 1) then
         call CARMAGAS_Create(carma, 1, "Water Vapor", WTMOL_H2O, I_VAPRTN_H2O_MURPHY2005, &
            I_GCOMP_H2O, rc)
         if (rc /= 0) then
            print *, "*** CARMAGAS_Create FAILED ***, rc=", rc
            return
         end if
      end if

      if (NGAS >= 2) then
         call CARMAGAS_Create(carma, 2, "Sulphuric Acid", 98.078479_f, I_VAPRTN_H2SO4_AYERS1980, &
            I_GCOMP_H2SO4, rc)
         if (rc /= 0) then
            print *, "*** CARMAGAS_Create FAILED ***, rc=", rc
            return
         end if

         ! Set up growth process if we have H2SO4
         call CARMA_AddGrowth(carma, 1, 2, rc)
         if (rc /= 0) then
            print *, "*** CARMA_AddGrowth FAILED ***, rc=", rc
            return
         end if
      end if

      ! Initialize CARMA
      call CARMA_Initialize(carma, rc, do_grow=.true., do_coag=.false., do_substep=.true., &
         do_thermo=.false., maxretries=16, maxsubsteps=32, dt_threshold=1._f)
      if (rc /= 0) then
         print *, "*** CARMA_Initialize FAILED ***, rc=", rc
         return
      end if

      print *, "Setting up atmosphere..."

      ! Set up simple atmospheric conditions
      lat(iy) = -40.0_f
      lon(ix) = -105.0_f

      ! Vertical grid setup
      do i = 1, NZ
         zc(i) = zmin + (deltaz * (i - 0.5_f))
      end do

      call GetStandardAtmosphere(zc, p=p, t=t)

      do i = 1, NZP1
         zl(i) = zmin + ((i - 1) * deltaz)
      end do
      call GetStandardAtmosphere(zl, p=pl)

      ! Set up initial conditions
      rhoa(:) = (p(:) * 10._f) / (R_AIR * t(:)) * (1e-3_f * 1e6_f)

      ! Store original temperature for told parameter
      t_orig(:) = t(:)

      ! Initialize gas mixing ratios
      mmr_gas(:,:) = 0._f
      if (NGAS >= 1) mmr_gas(:,1) = 100.e-6_f  ! H2O g/g
      if (NGAS >= 2) mmr_gas(:,2) = 0.1e-9_f * (98._f / 29._f)  ! H2SO4

      ! Initialize particle mixing ratios
      mmr(:,:,:) = 0._f

      print *, "Running time integration..."

      ! Time integration loop
      do istep = 1, nstep

         ! Calculate the model time
         time = (istep - 1) * dtime

         ! Create a CARMASTATE for this column
         call CARMASTATE_Create(cstate, carma_ptr, time, dtime, NZ, &
            I_CART, lat(iy), lon(ix), &
            zc(:), zl(:), &
            p(:),  pl(:), &
            t(:), rc, &
            told=t_orig(:), &
            qh2o=mmr_gas(:,1))
         if (rc /= 0) then
            print *, "*** CARMASTATE_Create FAILED ***, rc=", rc
            return
         end if

         ! Send the bin mmrs to CARMA
         do ielem = 1, NELEM
            do ibin = 1, NBIN
               call CARMASTATE_SetBin(cstate, ielem, ibin, mmr(:,ielem,ibin), rc)
               if (rc /= 0) then
                  print *, "*** CARMASTATE_SetBin FAILED ***, rc=", rc
                  return
               end if
            end do
         end do

         ! Send the gas mmrs to CARMA
         do igas = 1, NGAS
            call CARMASTATE_SetGas(cstate, igas, mmr_gas(:,igas), rc)
            if (rc /= 0) then
               print *, "*** CARMASTATE_SetGas FAILED ***, rc=", rc
               return
            end if
         end do

         ! Execute the time step
         call CARMASTATE_Step(cstate, rc)
         if (rc /= 0) then
            print *, "*** CARMASTATE_Step FAILED ***, rc=", rc
            return
         end if

         ! Get the updated bin mmr
         do ielem = 1, NELEM
            do ibin = 1, NBIN
               call CARMASTATE_GetBin(cstate, ielem, ibin, mmr(:,ielem,ibin), rc)
               if (rc /= 0) then
                  print *, "*** CARMASTATE_GetBin FAILED ***, rc=", rc
                  return
               end if
            end do
         end do

         ! Get the updated gas mmr
         do igas = 1, NGAS
            call CARMASTATE_GetGas(cstate, igas, mmr_gas(:,igas), rc)
            if (rc /= 0) then
               print *, "*** CARMASTATE_GetGas FAILED ***, rc=", rc
               return
            end if
         end do

         ! Clean up the carma state for this step
         call CARMASTATE_Destroy(cstate, rc)
         if (rc /= 0) then
            print *, "*** CARMASTATE_Destroy FAILED ***, rc=", rc
            return
         end if

         ! Print progress every 10 steps
         if (mod(istep, max(1, nstep/10)) == 0) then
            print *, "Completed step", istep, "of", nstep
         end if

      end do   ! time loop

      print *, "CARMA simulation completed with error code:", rc

      ! Clean up the carma instance
      call CARMA_Destroy(carma, rc)
      if (rc /= 0) then
         print *, "*** CARMA_Destroy FAILED ***, rc=", rc
      end if

      ! Deallocate arrays
      deallocate(lat, lon, zc, zl, p, pl, t, rhoa, t_orig, mmr_gas, mmr)

   end subroutine run_carma_simulation

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module carma_interface
