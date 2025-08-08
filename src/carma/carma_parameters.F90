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
             carma_wavelength_bin_t, carma_complex_t, carma_solute_config_t, carma_gas_config_t, &
             carma_coagulation_config_t, carma_growth_config_t, carma_nucleation_config_t, &
             carma_initialization_config_t, carma_output_data_t, carma_state_parameter_t, &
             carma_state_step_config_t, carma_surface_properties_t, carma_element_properties_t

   type, bind(c) :: carma_wavelength_bin_t
      real(c_double) :: center       ! Center of the wavelength bin [m]
      real(c_double) :: width        ! Width of the wavelength bin [m]
      logical(c_bool) :: do_emission ! Flag to indicate if emission is considered for this bin
   end type carma_wavelength_bin_t

   ! Complex number type for CARMA
   type, bind(c) :: carma_complex_t
      real(c_double) :: real_part    ! Real part of the complex number
      real(c_double) :: imag_part    ! Imaginary part of the complex number
   end type carma_complex_t

   type, bind(c) :: carma_group_config_t
      integer(c_int) :: name_length
      character(len=1, kind=c_char) :: name(256)
      integer(c_int) :: shortname_length
      character(len=1, kind=c_char) :: shortname(7)
      real(c_double) :: rmin
      real(c_double) :: rmrat
      real(c_double) :: rmassmin
      integer(c_int) :: ishape
      real(c_double) :: eshape
      integer(c_int) :: swelling_algorithm
      integer(c_int) :: swelling_composition
      integer(c_int) :: fall_velocity_routine
      integer(c_int) :: mie_calculation_algorithm
      integer(c_int) :: optics_algorithm
      logical(c_bool) :: is_ice
      logical(c_bool) :: is_fractal
      logical(c_bool) :: is_cloud
      logical(c_bool) :: is_sulfate
      logical(c_bool) :: do_wetdep
      logical(c_bool) :: do_drydep
      logical(c_bool) :: do_vtran
      real(c_double) :: solfac
      real(c_double) :: scavcoef
      real(c_double) :: dpc_threshold
      real(c_double) :: rmon
      type(c_ptr) :: df
      integer(c_int) :: df_size
      real(c_double) :: falpha
      real(c_double) :: neutral_volfrc
   end type carma_group_config_t

   type, bind(c) :: carma_element_config_t
      integer(c_int) :: igroup
      integer(c_int) :: isolute
      integer(c_int) :: name_length
      character(len=1, kind=c_char) :: name(256)
      integer(c_int) :: shortname_length
      character(len=1, kind=c_char) :: shortname(7)
      integer(c_int) :: itype
      integer(c_int) :: icomposition
      logical(c_bool) :: isShell
      real(c_double) :: rho
      type(c_ptr) :: rhobin
      integer(c_int) :: rhobin_size
      type(c_ptr) :: arat
      integer(c_int) :: arat_size
      real(c_double) :: kappa
      type(c_ptr) :: refidx  ! Complex refractive index
      integer(c_int) :: refidx_dim_1_size
      integer(c_int) :: refidx_dim_2_size
   end type carma_element_config_t

   type, bind(c) :: carma_solute_config_t
      integer(c_int) :: name_length
      character(len=1, kind=c_char) :: name(256)
      integer(c_int) :: shortname_length
      character(len=1, kind=c_char) :: shortname(7)
      integer(c_int) :: ions
      real(c_double) :: wtmol
      real(c_double) :: rho
   end type carma_solute_config_t

   type, bind(c) :: carma_gas_config_t
      integer(c_int) :: name_length
      character(len=1, kind=c_char) :: name(256)
      integer(c_int) :: shortname_length
      character(len=1, kind=c_char) :: shortname(7)
      real(c_double) :: wtmol          ! Molar mass of the gas [kg/mol]
      integer(c_int) :: ivaprtn        ! Vaporization routine
      integer(c_int) :: icomposition   ! Composition of the gas
      real(c_double) :: dgc_threshold  ! Convergence criteria for gas concentration [0 : off; > 0 : fraction]
      real(c_double) :: ds_threshold   ! Convergence criteria for gas saturation [0 : off; > 0 : fraction; < 0 : amount past 0 crossing]
      type(c_ptr) :: refidx            ! Wavelength-resolved refractive indices (n_ref_idx, n_wave)
      integer(c_int) :: refidx_dim_1_size  ! Size of first dimension
      integer(c_int) :: refidx_dim_2_size  ! Size of second dimension
   end type carma_gas_config_t

   type, bind(c) :: carma_coagulation_config_t
      integer(c_int) :: igroup1       ! First group index (first group to coagulate)
      integer(c_int) :: igroup2       ! Second group index (second group to coagulate)
      integer(c_int) :: igroup3       ! Third group index (coagulated particles)
      integer(c_int) :: algorithm     ! Coagulation algorithm
      real(c_double) :: ck0           ! Collection efficiency coefficient
      real(c_double) :: grav_e_coll0  ! Gravitational collection efficiency coefficient
      logical(c_bool) :: use_ccd      ! Use constant collection efficiency data
   end type carma_coagulation_config_t

   type, bind(c) :: carma_growth_config_t
      integer(c_int) :: ielem  ! Element index to grow
      integer(c_int) :: igas   ! Gas index to grow from
   end type carma_growth_config_t

   type, bind(c) :: carma_nucleation_config_t
      integer(c_int) :: ielemfrom   ! Element index to nucleate from
      integer(c_int) :: ielemto     ! Element index to nucleate to
      integer(c_int) :: algorithm   ! Nucleation algorithm
      real(c_double) :: rlh_nuc     ! Latent heat of nucleation [m2 s-2]
      integer(c_int) :: igas        ! Gas index to nucleate from
      integer(c_int) :: ievp2elem   ! Element index to evaporate to (if applicable)
   end type carma_nucleation_config_t

   type, bind(c) :: carma_initialization_config_t
      logical(c_bool) :: do_cnst_rlh
      logical(c_bool) :: do_detrain
      logical(c_bool) :: do_fixedinit
      logical(c_bool) :: do_incloud
      logical(c_bool) :: do_explised
      logical(c_bool) :: do_substep
      logical(c_bool) :: do_thermo
      logical(c_bool) :: do_vdiff
      logical(c_bool) :: do_vtran
      logical(c_bool) :: do_drydep
      logical(c_bool) :: do_pheat
      logical(c_bool) :: do_pheatatm
      logical(c_bool) :: do_clearsky
      logical(c_bool) :: do_partialinit
      logical(c_bool) :: do_coremasscheck
      integer(c_int) :: sulfnucl_method
      real(c_double) :: vf_const
      integer(c_int) :: minsubsteps
      integer(c_int) :: maxsubsteps
      integer(c_int) :: maxretries
      real(c_double) :: conmax
      real(c_double) :: dt_threshold
      real(c_double) :: cstick
      real(c_double) :: gsticki
      real(c_double) :: gstickl
      real(c_double) :: tstick
   end type carma_initialization_config_t

   type, bind(c) :: carma_element_properties_t
      integer(c_int) :: igroup
      integer(c_int) :: isolute
      integer(c_int) :: itype
      integer(c_int) :: icomposition
      logical(c_bool) :: isShell
      real(c_double) :: kappa
      type(c_ptr) :: rho
      integer(c_int) :: rho_size
      type(c_ptr) :: refidx
      integer(c_int) :: refidx_dim_1_size
      integer(c_int) :: refidx_dim_2_size
   end type carma_element_properties_t

   type, bind(c) :: carma_parameters_t

      ! Model dimensions
      integer(c_int) :: nbin = 5
      integer(c_int) :: nz = 1

      ! Time stepping parameters
      real(c_double) :: dtime = 1800.0_real64

      ! Wavelength grid
      type(c_ptr) :: wavelength_bins  ! wavelength_bins(NWAVE)
      integer(c_int) :: wavelength_bin_size = 0
      integer(c_int) :: number_of_refractive_indices = 0  ! Number of refractive indices per wavelength

      ! Component configurations (will be handled through C interface)
      ! Note: components are managed through C pointers in interface
      type(c_ptr) :: groups
      integer(c_int) :: groups_size
      type(c_ptr) :: elements
      integer(c_int) :: elements_size
      type(c_ptr) :: solutes
      integer(c_int) :: solutes_size
      type(c_ptr) :: gases
      integer(c_int) :: gases_size

      ! Process configurations
      type(c_ptr) :: coagulations  ! Pointer to coagulations array
      integer(c_int) :: coagulations_size  ! Number of coagulations
      type(c_ptr) :: growths        ! Pointer to growths array
      integer(c_int) :: growths_size  ! Number of growths
      type(c_ptr) :: nucleations     ! Pointer to nucleations array
      integer(c_int) :: nucleations_size  ! Number of nucleations

      ! Initialization configuration
      type(carma_initialization_config_t) :: initialization
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
      real(c_double) :: time                ! Current time [s]
      real(c_double) :: time_step
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
      type(c_ptr) :: specific_humidity    ! specific humidity at each vertical level [kg/kg]
      integer(c_int) :: specific_humidity_size
      type(c_ptr) :: relative_humidity    ! relative humidity at each vertical level [fraction]
      integer(c_int) :: relative_humidity_size
      type(c_ptr) :: original_temperature  ! original temperature profile at each vertical level [K]
      integer(c_int) :: original_temperature_size
      type(c_ptr) :: radiative_intensity   ! radiative intensity at each vertical level [W/m²/sr/m]
      integer(c_int) :: radiative_intensity_dim_1_size
      integer(c_int) :: radiative_intensity_dim_2_size
   end type carma_state_parameter_t

   type, bind(c) :: carma_surface_properties_t
      real(c_double) :: surface_friction_velocity ! Surface friction velocity [m/s]
      real(c_double) :: aerodynamic_resistance    ! Aerodynamic resistance [s/m]
      real(c_double) :: area_fraction             ! Area fraction of the surface type [fraction]
   end type carma_surface_properties_t

   type, bind(c) :: carma_state_step_config_t
      type(c_ptr) :: cloud_fraction              ! Cloud fraction at each vertical level [fraction]
      integer(c_int) :: cloud_fraction_size
      type(c_ptr) :: critical_relative_humidity  ! Critical relative humidity at each vertical level [fraction]
      integer(c_int) :: critical_relative_humidity_size
      type(carma_surface_properties_t) :: land   ! Surface properties for land
      type(carma_surface_properties_t) :: ocean  ! Surface properties for ocean
      type(carma_surface_properties_t) :: ice    ! Surface properties for ice
   end type carma_state_step_config_t

end module carma_parameters_mod
