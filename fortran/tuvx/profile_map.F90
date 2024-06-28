! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx_profile_map
   use iso_c_binding, only: c_ptr, c_null_ptr

   implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

   private
   public :: profile_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   interface
      function get_profile_c(profile_map, profile_name, profile_units, error)    &
          bind(C, name="GetProfile")
         use musica_util, only: error_t_c
         use iso_c_binding, only: c_ptr, c_char
         type(c_ptr), value, intent(in)            :: profile_map
         character(len=1, kind=c_char), intent(in) :: profile_name(*),           &
                                                      profile_units(*)
         type(error_t_c), intent(inout)            :: error
         type(c_ptr)                               :: get_profile_c
      end function get_profile_c
   end interface

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   
   type :: profile_map_t
      type(c_ptr) :: ptr_ = c_null_ptr
   contains
      procedure :: get => get_profile
      ! Deallocate the profile map instance
      final :: finalize_profile_map_t
   end type profile_map_t

   interface profile_map_t
      procedure profile_map_t_ptr_constructor
   end interface profile_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Construct a profile map instance
   function profile_map_t_ptr_constructor(profile_map_c_ptr) result(this)
      ! Arguments
      type(c_ptr), intent(in) :: profile_map_c_ptr
      ! Return value
      type(profile_map_t) :: this

      this%ptr_ = profile_map_c_ptr
   end function profile_map_t_ptr_constructor

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Get a profile given its name and units
   function get_profile(this, profile_name, profile_units, error) result(profile)
      use iso_c_binding, only: c_char
      use musica_tuvx_profile, only: profile_t
      use musica_util, only: error_t, error_t_c, to_c_string

      ! Arguments
      class(profile_map_t), intent(in) :: this
      character(len=*), intent(in)  :: profile_name
      character(len=*), intent(in)  :: profile_units
      type(error_t), intent(inout)  :: error

      ! Local variables
      type(error_t_c)               :: error_c
      
      ! Return value
      type(profile_t), pointer :: profile

      profile => profile_t(get_profile_c(this%ptr_, to_c_string(profile_name), &
                                         to_c_string(profile_units), error_c))

      error = error_t(error_c)

   end function get_profile

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Deallocate the profile map instance
   subroutine finalize_profile_map_t(this)
      use musica_util, only: error_t, error_t_c, assert

      ! Arguments
      type(profile_map_t), intent(inout) :: this

      ! Local variables
      type(error_t_c) :: error_c
      type(error_t)   :: error

      ! The pointer doesn't need to be deallocated because it is owned by the
      ! tuvx instance
      this%ptr_ = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())

   end subroutine finalize_profile_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_profile_map
