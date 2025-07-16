! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! This module defines the CARMA parameters type that corresponds to the C++ CARMAParameters struct
module carma_parameters_mod

   use iso_fortran_env, only: real64
   implicit none

   private

   public :: carma_parameters_type

   type :: carma_parameters_type
      integer :: max_bins = 100
      integer :: max_groups = 10

      ! Model dimensions
      integer :: nz = 1
      integer :: ny = 1
      integer :: nx = 1
      integer :: nelem = 1
      integer :: ngroup = 1
      integer :: nbin = 5
      integer :: nsolute = 0
      integer :: ngas = 0
      integer :: nwave = 30

      ! Time stepping parameters
      real(real64) :: dtime = 1800.0_real64
      integer :: nstep = 100

      ! Spatial parameters
      real(real64) :: deltaz = 1000.0_real64
      real(real64) :: zmin = 16500.0_real64
   end type carma_parameters_type

end module carma_parameters_mod
