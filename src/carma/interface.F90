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

end module carma_interface
