! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! This module defines the CARMA parameters type that corresponds to the C++ CARMAParameters struct
module carma_parameters_mod

   use iso_fortran_env, only: real64
   use iso_c_binding, only: c_int, c_double, c_ptr, c_bool, c_char

   implicit none

   private

   public :: carma_group_config_t, carma_element_config_t, carma_parameters_t, &
             carma_wavelength_bin_t, carma_output_data_t, carma_state_parameter_t

   type, bind(c) :: carma_wavelength_bin_t
      real(c_double) :: center       ! Center of the wavelength bin [m]
      real(c_double) :: width        ! Width of the wavelength bin [m]
      logical(c_bool) :: do_emission ! Flag to indicate if emission is considered for this bin
   end type carma_wavelength_bin_t

   type, bind(c) :: carma_group_config_t
      integer(c_int) :: id
      integer(c_int) :: name_length
      character(len=1, kind=c_char) :: name(256)
      integer(c_int) :: shortname_length
      character(len=1, kind=c_char) :: shortname(7)
      real(c_double) :: rmin
      real(c_double) :: rmrat
      integer(c_int) :: ishape
      real(c_double) :: eshape
      logical(c_bool) :: is_ice
      logical(c_bool) :: is_fractal
      logical(c_bool) :: do_mie
      logical(c_bool) :: do_wetdep
      logical(c_bool) :: do_drydep
      logical(c_bool) :: do_vtran
      real(c_double) :: solfac
      real(c_double) :: scavcoef
      real(c_double) :: rmon
      type(c_ptr) :: df
      integer(c_int) :: df_size
      real(c_double) :: falpha
   end type carma_group_config_t

   type, bind(c) :: carma_element_config_t
      integer(c_int) :: id
      integer(c_int) :: igroup
      integer(c_int) :: name_length
      character(len=1, kind=c_char) :: name(256)
      integer(c_int) :: shortname_length
      character(len=1, kind=c_char) :: shortname(7)
      real(c_double) :: rho
      integer(c_int) :: itype
      integer(c_int) :: icomposition
      integer(c_int) :: isolute
      type(c_ptr) :: rhobin
      integer(c_int) :: rhobin_size
      type(c_ptr) :: arat
      integer(c_int) :: arat_size
      real(c_double) :: kappa
      logical(c_bool) :: isShell
   end type carma_element_config_t

   type, bind(c) :: carma_parameters_t
      integer(c_int) :: max_bins = 100
      integer(c_int) :: max_groups = 10

      ! Model dimensions
      integer(c_int) :: nz = 1
      integer(c_int) :: nbin = 5
      integer(c_int) :: nsolute = 0
      integer(c_int) :: ngas = 0

      ! Time stepping parameters
      real(c_double) :: dtime = 1800.0_real64
      integer(c_int) :: nstep = 100

      ! Spatial parameters
      real(c_double) :: deltaz = 1000.0_real64
      real(c_double) :: zmin = 16500.0_real64

      ! Wavelength grid
      type(c_ptr) :: wavelength_bins  ! wavelength_bins(NWAVE)
      integer(c_int) :: wavelength_bin_size = 0
      integer(c_int) :: number_of_refractive_indices = 0  ! Number of refractive indices per wavelength

      ! Group and element configurations (will be handled through C interface)
      ! Note: groups and elements are managed through C pointers in interface
      type(c_ptr) :: groups
      integer(c_int) :: groups_size
      type(c_ptr) :: elements
      integer(c_int) :: elements_size
   end type carma_parameters_t

   type, bind(C) :: carma_output_data_t
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
   end type carma_output_data_t

   type, bind(C) :: carma_state_parameter_t
      real(c_double) :: time
      real(c_double) :: longitude
      real(c_double) :: latitude
      integer(c_int) :: coordinates

      type(c_ptr) :: vertical_center       ! vertical center of the grid [m]
      integer(c_int) :: vertical_center_size
      type(c_ptr) :: vertical_levels       ! vertical levels of the grid [m]
      integer(c_int) :: vertical_levels_size
      type(c_ptr) :: temperature           ! temperature at each vertical level [K]
      integer(c_int) :: temperature_size
      type(c_ptr) :: pressure              ! pressure at each vertical level [Pa]
      integer(c_int) :: pressure_size
      type(c_ptr) :: pressure_levels    ! pressure levels of the grid [Pa]
      integer(c_int) :: pressure_levels_size
   end type carma_state_parameter_t

end module carma_parameters_mod
