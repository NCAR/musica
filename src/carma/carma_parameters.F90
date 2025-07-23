! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! This module defines the CARMA parameters type that corresponds to the C++ CARMAParameters struct
module carma_parameters_mod

   use iso_fortran_env, only: real64
   use iso_c_binding, only: c_int, c_double, c_ptr, c_bool
   
   implicit none

   private

   public :: carma_group_config_t, carma_element_config_t, carma_parameters_t

   type, bind(c) :: carma_group_config_t
      integer(c_int) :: id
      integer(c_int) :: name_length
      type(c_ptr) :: name
      integer(c_int) :: shortname_length
      type(c_ptr) :: shortname
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
      type(c_ptr) :: name
      integer(c_int) :: shortname_length
      type(c_ptr) :: shortname
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
      integer(c_int) :: ny = 1
      integer(c_int) :: nx = 1
      integer(c_int) :: nbin = 5
      integer(c_int) :: nsolute = 0
      integer(c_int) :: ngas = 0
      integer(c_int) :: nwave = 30
      integer(c_int) :: idx_wave = 0

      ! Time stepping parameters
      real(c_double) :: dtime = 1800.0_real64
      integer(c_int) :: nstep = 100

      ! Spatial parameters
      real(c_double) :: deltaz = 1000.0_real64
      real(c_double) :: zmin = 16500.0_real64

      ! Optical parameters
      type(c_ptr) :: extinction_coefficient  ! qext(NWAVE, NBIN, NGROUP)
      integer(c_int) :: extinction_coefficient_size

      ! Group and element configurations (will be handled through C interface)
      ! Note: groups and elements are managed through C pointers in interface
      type(c_ptr) :: groups
      integer(c_int) :: groups_size
      type(c_ptr) :: elements
      integer(c_int) :: elements_size
   end type carma_parameters_t

end module carma_parameters_mod
