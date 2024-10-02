! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_cloudj
  use iso_c_binding,            only: c_ptr, c_null_ptr
  use musica_cloudj_grid,         only : grid_t
  use musica_cloudj_grid_map,     only : grid_map_t
  use musica_cloudj_profile,      only : profile_t
  use musica_cloudj_profile_map,  only : profile_map_t
  use musica_cloudj_radiator,     only : radiator_t
  use musica_cloudj_radiator_map, only : radiator_map_t

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

  private
  public :: cloudj_t, grid_map_t, grid_t, profile_map_t, profile_t, &
            radiator_map_t, radiator_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  interface
    function create_cloudj_c(config_path, grids, profiles, radiators, error) &
        bind(C, name="CreateCloudj")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr, c_int, c_char
      character(len=1, kind=c_char), intent(in)    :: config_path(*)
      type(c_ptr),            value, intent(in)    :: grids
      type(c_ptr),            value, intent(in)    :: profiles
      type(c_ptr),            value, intent(in)    :: radiators
      type(error_t_c),               intent(inout) :: error
      type(c_ptr)                                  :: create_cloudj_c
    end function create_cloudj_c

    subroutine delete_cloudj_c(cloudj, error) bind(C, name="DeleteCloudj")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: cloudj
      type(error_t_c), intent(inout) :: error
    end subroutine delete_cloudj_c

    function get_grid_map_c(cloudj, error) bind(C, name="GetGridMap")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: cloudj
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: get_grid_map_c
    end function get_grid_map_c

    function get_profile_map_c(cloudj, error) bind(C, name="GetProfileMap")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: cloudj
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: get_profile_map_c
    end function get_profile_map_c

    function get_radiator_map_c(cloudj, error) bind(C, name="GetRadiatorMap")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: cloudj
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: get_radiator_map_c
    end function get_radiator_map_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  type :: cloudj_t
    type(c_ptr), private :: ptr_ = c_null_ptr
  contains
    ! Create a grid map
    procedure :: get_grids
    ! Create a profile map
    procedure :: get_profiles
    ! Create a radiator map
    procedure :: get_radiators
    ! Deallocate the cloudj instance
    final :: finalize
  end type cloudj_t

  interface cloudj_t
    procedure constructor
  end interface cloudj_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Construct a cloudj instance
  function constructor(config_path, grids, profiles, radiators, error) &
      result( this )
    use iso_c_binding, only: c_char, c_null_char, c_loc
    use musica_util, only: error_t_c, error_t

    ! Arguments
    character(len=*),             intent(in)    :: config_path
    type(grid_map_t),     target, intent(in)    :: grids
    type(profile_map_t),  target, intent(in)    :: profiles
    type(radiator_map_t), target, intent(in)    :: radiators
    type(error_t),                intent(inout) :: error

    ! Local variables
    character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
    type(error_t_c)               :: error_c
    integer                       :: n, i

    ! Return value
    type(cloudj_t), pointer         :: this

    allocate( this )

    n = len_trim(config_path)
    do i = 1, n
      c_config_path(i) = config_path(i:i)
    end do
    c_config_path(n+1) = c_null_char

    this%ptr_ = create_cloudj_c(c_config_path, grids%ptr_, profiles%ptr_, &
                              radiators%ptr_, error_c)

    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if
  end function constructor

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the grid map
  function get_grids(this, error) result(grid_map)
    use musica_util, only: error_t, error_t_c

    ! Arguments
    class(cloudj_t), intent(inout) :: this
    type(error_t), intent(inout) :: error

    ! Local variables
    type(error_t_c)              :: error_c

    ! Return value
    type(grid_map_t), pointer    :: grid_map

    grid_map => grid_map_t(get_grid_map_c(this%ptr_, error_c))
    
    error = error_t(error_c)

  end function get_grids

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the profile map
  function get_profiles(this, error) result(profile_map)
    use musica_util, only: error_t, error_t_c

    ! Arguments
    class(cloudj_t), intent(inout) :: this
    type(error_t), intent(inout) :: error

    ! Local variables
    type(error_t_c)              :: error_c

    ! Return value
    type(profile_map_t), pointer :: profile_map

    profile_map => profile_map_t(get_profile_map_c(this%ptr_, error_c))
    
    error = error_t(error_c)

  end function get_profiles

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the radiator map
  function get_radiators(this, error) result(radiator_map)
    use musica_util, only: error_t, error_t_c

    ! Arguments
    class(cloudj_t), intent(inout)  :: this
    type(error_t), intent(inout)  :: error

    ! Local variables
    type(error_t_c)               :: error_c

    ! Return value
    type(radiator_map_t), pointer :: radiator_map

    radiator_map => radiator_map_t(get_radiator_map_c(this%ptr_, error_c))
    
    error = error_t(error_c)

  end function get_radiators

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Deallocate the cloudj instance
  subroutine finalize(this)
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(cloudj_t), intent(inout) :: this

    ! Local variables
    type(error_t_c)             :: error_c
    type(error_t)               :: error

    call delete_cloudj_c(this%ptr_, error_c)
    this%ptr_ = c_null_ptr
    error = error_t(error_c)
    ASSERT(error%is_success())

  end subroutine finalize

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_cloudj
