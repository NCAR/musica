! Copyright (C) 2023-2025 National Center for Atmospheric Research
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
    function create_profile_map_c(error) bind(C, name="CreateProfileMap")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: create_profile_map_c
    end function create_profile_map_c

    subroutine delete_profile_map_c(profile_map, error)                        &
      bind(C, name="DeleteProfileMap")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: profile_map
      type(error_t_c), intent(inout) :: error
    end subroutine delete_profile_map_c

    subroutine add_profile_c(profile_map, profile, error)                      &
      bind(C, name="AddProfile")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: profile_map
      type(c_ptr), value, intent(in) :: profile
      type(error_t_c), intent(inout) :: error
    end subroutine add_profile_c

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
    ! Adds a profile to the profile map
    procedure :: add => add_profile
    ! Get a profile given its name and units
    procedure :: get => get_profile
    ! Deallocate the profile map instance
    final :: finalize_profile_map_t
  end type profile_map_t

  interface profile_map_t
    procedure profile_map_t_ptr_constructor
    procedure profile_map_t_constructor
  end interface profile_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Construct a profile map instance
  function profile_map_t_ptr_constructor(profile_map_c_ptr) result(this)
    ! Arguments
    type(c_ptr), intent(in) :: profile_map_c_ptr
    ! Return value
    type(profile_map_t), pointer :: this

    allocate( this )
    this%ptr_ = profile_map_c_ptr

  end function profile_map_t_ptr_constructor

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Create a new profile map
  function profile_map_t_constructor(error) result(this)
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(error_t), intent(inout) :: error

    ! Return value
    type(profile_map_t), pointer :: this

    ! Local variables
    type(error_t_c) error_c

    allocate( this )
    this%ptr_ = create_profile_map_c(error_c)
    error = error_t(error_c)

  end function profile_map_t_constructor
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Adds a profile to the profile map
  subroutine add_profile(this, profile, error)
    use musica_tuvx_profile, only: profile_t
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    class(profile_map_t), intent(inout) :: this
    type(profile_t),      intent(in)    :: profile
    type(error_t),        intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c

    call add_profile_c(this%ptr_, profile%ptr_, error_c)
    error = error_t(error_c)

  end subroutine add_profile
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Gets a profile given its name and units
  function get_profile(this, profile_name, profile_units, error) result(profile)
    use iso_c_binding, only: c_char
    use musica_tuvx_profile, only: profile_t
    use musica_util, only: error_t, error_t_c, to_c_string

    ! Arguments
    class(profile_map_t), intent(in)    :: this
    character(len=*),     intent(in)    :: profile_name
    character(len=*),     intent(in)    :: profile_units
    type(error_t),        intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    
    ! Return value
    type(profile_t), pointer :: profile

    profile => profile_t(get_profile_c(this%ptr_, to_c_string(profile_name), &
                                        to_c_string(profile_units), error_c))

    error = error_t(error_c)

  end function get_profile

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Deallocates the profile map instance
  subroutine finalize_profile_map_t(this)
    use iso_c_binding, only: c_associated
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(profile_map_t), intent(inout) :: this

    ! Local variables
    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (c_associated(this%ptr_)) then
      call delete_profile_map_c(this%ptr_, error_c)
      this%ptr_ = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if

  end subroutine finalize_profile_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_profile_map
