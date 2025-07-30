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
             carma_initialization_config_t

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

   type, bind(c) :: carma_parameters_t

      ! Model dimensions
      integer(c_int) :: nz = 1
      integer(c_int) :: ny = 1
      integer(c_int) :: nx = 1
      integer(c_int) :: nbin = 5

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

end module carma_parameters_mod
