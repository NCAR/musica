! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx_profile
   use iso_c_binding, only: c_ptr, c_null_ptr

   implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

   private
   public :: profile_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   interface
      subroutine set_profile_edge_values_c(profile, edge_values, n_edge_values,  &
          error) bind(C, name="SetProfileEdgeValues")
         use musica_util, only: error_t_c
         use iso_c_binding, only: c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: profile
         real(c_double), dimension(*), intent(in) :: edge_values
         integer(c_size_t), value                 :: n_edge_values
         type(error_t_c), intent(inout)           :: error
      end subroutine set_profile_edge_values_c

      subroutine set_profile_midpoint_values_c(profile, midpoint_values,         &
          n_midpoint_values, error) bind(C, name="SetProfileMidpointValues")
         use musica_util, only: error_t_c
         use iso_c_binding, only: c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: profile
         real(c_double), dimension(*), intent(in) :: midpoint_values
         integer(c_size_t), value                 :: n_midpoint_values
         type(error_t_c), intent(inout)           :: error
      end subroutine set_profile_midpoint_values_c

      subroutine set_profile_layer_densities_c(profile, layer_densities,         &
          n_layer_densities, error) bind(C, name="SetProfileLayerDensities")
         use musica_util, only: error_t_c
         use iso_c_binding, only: c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: profile
         real(c_double), dimension(*), intent(in) :: layer_densities
         integer(c_size_t), value                 :: n_layer_densities
         type(error_t_c), intent(inout)           :: error
      end subroutine set_profile_layer_densities_c

      subroutine set_profile_exo_layer_density_c(profile, exo_layer_density,     &
          error) bind(C, name="SetProfileExoLayerDensity")
         use musica_util, only: error_t_c
         use iso_c_binding, only: c_ptr, c_double
         type(c_ptr), value, intent(in) :: profile
         real(c_double), value, intent(in) :: exo_layer_density
         type(error_t_c), intent(inout) :: error
      end subroutine set_profile_exo_layer_density_c

      subroutine calculate_profile_exo_layer_density(profile, scale_height,      &
          error) bind(C, name="CalculateProfileExoLayerDensity")
         use musica_util, only: error_t_c
         use iso_c_binding, only: c_ptr, c_double
         type(c_ptr), value, intent(in) :: profile
         real(c_double), value, intent(in) :: scale_height
         type(error_t_c), intent(inout) :: error
      end subroutine calculate_profile_exo_layer_density

   end interface

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   
   type :: profile_t
      type(c_ptr), private :: ptr_ = c_null_ptr
   contains
      ! Set profile edge values
      procedure :: set_edge_values
      ! Set the profile midpoint values
      procedure :: set_midpoint_values
      ! Set the profile layer densities
      procedure :: set_layer_densities
      ! Set the profile exo layer density
      procedure :: set_exo_layer_density
      ! Calculate the profile exo layer density
      procedure :: calculate_exo_layer_density
   end type profile_t

   interface profile_t
      procedure profile_t_ptr_constructor
   end interface profile_t
   
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Construct a profile instance
   function profile_t_ptr_constructor(profile_c_ptr) result(this)
      use iso_c_binding, only: c_ptr
      ! Arguments
      type(c_ptr), intent(in) :: profile_c_ptr

      ! Return value
      type(profile_t), pointer :: this

      allocate( this )
      this%ptr_ = profile_c_ptr

   end function profile_t_ptr_constructor

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_edge_values(this, edge_values, error)
      use iso_c_binding, only: c_double, c_size_t
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: edge_values
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_edge_values

      n_edge_values = size(edge_values)

      call set_profile_edge_values_c(this%ptr_, edge_values, n_edge_values,      &
                                     error_c)
      error = error_t(error_c)

   end subroutine set_edge_values

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_midpoint_values(this, midpoint_values, error)
      use iso_c_binding, only: c_double, c_size_t
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: midpoint_values
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_midpoint_values

      n_midpoint_values = size(midpoint_values)

      call set_profile_midpoint_values_c(this%ptr_, midpoint_values,             &
                                         n_midpoint_values, error_c)
      error = error_t(error_c)

   end subroutine set_midpoint_values

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_layer_densities(this, layer_densities, error)
      use iso_c_binding, only: c_double, c_size_t
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: layer_densities
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_layer_densities

      n_layer_densities = size(layer_densities)

      call set_profile_layer_densities_c(this%ptr_, layer_densities,             &
                                         n_layer_densities, error_c)
      error = error_t(error_c)

   end subroutine set_layer_densities

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_exo_layer_density(this, exo_layer_density, error)
      use iso_c_binding, only: c_double, c_size_t
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), intent(in) :: exo_layer_density
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c

      call set_profile_exo_layer_density_c(this%ptr_, exo_layer_density, error_c)
      error = error_t(error_c)

   end subroutine set_exo_layer_density

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine calculate_exo_layer_density(this, scale_height, error)
      use iso_c_binding, only: c_double, c_size_t
      use musica_util, only: error_t, error_t_c, assert

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), intent(in) :: scale_height
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c

      call calculate_profile_exo_layer_density(this%ptr_, scale_height, error_c)
      error = error_t(error_c)
      ASSERT(error%is_success())

   end subroutine calculate_exo_layer_density

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_profile
