! Copyright (C) 2023-2025 National Center for Atmospheric Research
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
    function create_profile_c(profile_name, profile_units, grid, error) &
      bind(C, name="CreateProfile")
      use iso_c_binding, only: c_ptr, c_char
      use musica_util, only: error_t_c
      character(len=1, kind=c_char), intent(in) :: profile_name(*)
      character(len=1, kind=c_char), intent(in) :: profile_units(*)
      type(c_ptr), value, intent(in) :: grid
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: create_profile_c
    end function create_profile_c

    subroutine delete_profile_c(profile, error) bind(C, name="DeleteProfile")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: profile
      type(error_t_c), intent(inout) :: error
    end subroutine delete_profile_c

    subroutine set_profile_edge_values_c(profile, edge_values, n_edge_values,  &
      error) bind(C, name="SetProfileEdgeValues")
      use iso_c_binding, only: c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: profile
      type(c_ptr),       value, intent(in)    :: edge_values
      integer(c_size_t), value, intent(in)    :: n_edge_values
      type(error_t_c),          intent(inout) :: error
    end subroutine set_profile_edge_values_c

    subroutine get_profile_edge_values_c(profile, edge_values, n_edge_values,  &
      error) bind(C, name="GetProfileEdgeValues")
      use iso_c_binding, only: c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: profile
      type(c_ptr),       value, intent(in)    :: edge_values
      integer(c_size_t), value, intent(in)    :: n_edge_values
      type(error_t_c),          intent(inout) :: error
    end subroutine get_profile_edge_values_c

    subroutine set_profile_midpoint_values_c(profile, midpoint_values,         &
      n_midpoint_values, error) bind(C, name="SetProfileMidpointValues")
      use iso_c_binding, only: c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: profile
      type(c_ptr),       value, intent(in)    :: midpoint_values
      integer(c_size_t), value, intent(in)    :: n_midpoint_values
      type(error_t_c),          intent(inout) :: error
    end subroutine set_profile_midpoint_values_c

    subroutine get_profile_midpoint_values_c(profile, midpoint_values,         &
      n_midpoint_values, error) bind(C, name="GetProfileMidpointValues")
      use iso_c_binding, only: c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: profile
      type(c_ptr),       value, intent(in)    :: midpoint_values
      integer(c_size_t), value, intent(in)    :: n_midpoint_values
      type(error_t_c),          intent(inout) :: error
    end subroutine get_profile_midpoint_values_c

    subroutine set_profile_layer_densities_c(profile, layer_densities,         &
      n_layer_densities, error) bind(C, name="SetProfileLayerDensities")
      use iso_c_binding, only: c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: profile
      type(c_ptr),       value, intent(in)    :: layer_densities
      integer(c_size_t), value, intent(in)    :: n_layer_densities
      type(error_t_c),          intent(inout) :: error
    end subroutine set_profile_layer_densities_c

    subroutine get_profile_layer_densities_c(profile, layer_densities,         &
        n_layer_densities, error) bind(C, name="GetProfileLayerDensities")
        use iso_c_binding, only: c_ptr, c_size_t
        use musica_util, only: error_t_c
        type(c_ptr),       value, intent(in)    :: profile
        type(c_ptr),       value, intent(in)    :: layer_densities
        integer(c_size_t), value, intent(in)    :: n_layer_densities
        type(error_t_c),          intent(inout) :: error
    end subroutine get_profile_layer_densities_c

    subroutine set_profile_exo_layer_density_c(profile, exo_layer_density,     &
      error) bind(C, name="SetProfileExoLayerDensity")
      use iso_c_binding, only: c_ptr, c_double
      use musica_util, only: error_t_c
      type(c_ptr),    value, intent(in)    :: profile
      real(c_double), value, intent(in)    :: exo_layer_density
      type(error_t_c),       intent(inout) :: error
    end subroutine set_profile_exo_layer_density_c

    subroutine calculate_profile_exo_layer_density(profile, scale_height,      &
      error) bind(C, name="CalculateProfileExoLayerDensity")
      use iso_c_binding, only: c_ptr, c_double
      use musica_util, only: error_t_c
      type(c_ptr),    value, intent(in)    :: profile
      real(c_double), value, intent(in)    :: scale_height
      type(error_t_c),       intent(inout) :: error
    end subroutine calculate_profile_exo_layer_density

    function get_profile_exo_layer_density_c(profile, error)                   &
      bind(C, name="GetProfileExoLayerDensity")
      use iso_c_binding, only: c_ptr, c_double
      use musica_util, only: error_t_c
      type(c_ptr),    value, intent(in)    :: profile
      type(error_t_c),       intent(inout) :: error
      real(c_double) :: get_profile_exo_layer_density_c
    end function get_profile_exo_layer_density_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  type :: profile_t
    type(c_ptr) :: ptr_ = c_null_ptr
  contains
    ! Set profile edge values
    procedure :: set_edge_values
    ! Get profile edge values
    procedure :: get_edge_values
    ! Set the profile midpoint values
    procedure :: set_midpoint_values
    ! Get the profile midpoint values
    procedure :: get_midpoint_values
    ! Set the profile layer densities
    procedure :: set_layer_densities
    ! Get the profile layer densities
    procedure :: get_layer_densities
    ! Set the profile exo layer density
    procedure :: set_exo_layer_density
    ! Calculate the profile exo layer density
    procedure :: calculate_exo_layer_density
    ! Get the profile exo layer density
    procedure :: get_exo_layer_density
    ! Finalize the profile
    final :: finalize_profile
  end type profile_t

  interface profile_t
    procedure profile_t_ptr_constructor
    procedure profile_t_constructor
  end interface profile_t
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Construct a profile instance
  function profile_t_ptr_constructor(profile_c_ptr) result(this)
    ! Arguments
    type(c_ptr), intent(in) :: profile_c_ptr

    ! Return value
    type(profile_t), pointer :: this

    allocate( this )
    this%ptr_ = profile_c_ptr

  end function profile_t_ptr_constructor

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Construct a profile instance that allocates a new TUV-x profile
  function profile_t_constructor(profile_name, profile_units, grid, error) &
      result(this)
    use musica_tuvx_grid, only: grid_t
    use musica_util, only: error_t, error_t_c, to_c_string

    ! Arguments
    character(len=*), intent(in) :: profile_name
    character(len=*), intent(in) :: profile_units
    type(grid_t),     intent(in) :: grid
    type(error_t), intent(inout) :: error

    ! Return value
    type(profile_t), pointer :: this

    ! Local variables
    type(error_t_c) :: error_c
    
    allocate( this )
    this%ptr_ = create_profile_c(to_c_string(profile_name),                    &
                                  to_c_string(profile_units), grid%ptr_, error_c)
    error = error_t(error_c)

  end function profile_t_constructor
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_edge_values(this, edge_values, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t),             intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: edge_values(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_edge_values

    n_edge_values = size(edge_values)

    call set_profile_edge_values_c(this%ptr_, c_loc(edge_values),              &
                                    n_edge_values, error_c)
    error = error_t(error_c)

  end subroutine set_edge_values

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_edge_values(this, edge_values, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t),             intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: edge_values(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_edge_values

    n_edge_values = size(edge_values)

    call get_profile_edge_values_c(this%ptr_, c_loc(edge_values),              &
                                    n_edge_values, error_c)
    error = error_t(error_c)

  end subroutine get_edge_values
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_midpoint_values(this, midpoint_values, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t),             intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: midpoint_values(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_midpoint_values

    n_midpoint_values = size(midpoint_values)

    call set_profile_midpoint_values_c(this%ptr_, c_loc(midpoint_values),      &
                                        n_midpoint_values, error_c)
    error = error_t(error_c)

  end subroutine set_midpoint_values

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_midpoint_values(this, midpoint_values, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t),             intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: midpoint_values(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_midpoint_values

    n_midpoint_values = size(midpoint_values)

    call get_profile_midpoint_values_c(this%ptr_, c_loc(midpoint_values),      &
                                        n_midpoint_values, error_c)
    error = error_t(error_c)

  end subroutine get_midpoint_values
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_layer_densities(this, layer_densities, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t),             intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: layer_densities(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_layer_densities

    n_layer_densities = size(layer_densities)

    call set_profile_layer_densities_c(this%ptr_, c_loc(layer_densities),      &
                                        n_layer_densities, error_c)
    error = error_t(error_c)

  end subroutine set_layer_densities

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_layer_densities(this, layer_densities, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t),             intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: layer_densities(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_layer_densities

    n_layer_densities = size(layer_densities)

    call get_profile_layer_densities_c(this%ptr_, c_loc(layer_densities),      &
                                        n_layer_densities, error_c)
    error = error_t(error_c)

  end subroutine get_layer_densities
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_exo_layer_density(this, exo_layer_density, error)
    use iso_c_binding, only: c_double, c_size_t
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t), intent(inout) :: this
    real(dk),         intent(in)    :: exo_layer_density
    type(error_t),    intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c

    call set_profile_exo_layer_density_c(this%ptr_,                            &
                                real(exo_layer_density, kind=c_double), error_c)
    error = error_t(error_c)

  end subroutine set_exo_layer_density

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine calculate_exo_layer_density(this, scale_height, error)
    use iso_c_binding, only: c_double, c_size_t
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t), intent(inout) :: this
    real(dk),         intent(in)    :: scale_height
    type(error_t),    intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c

    call calculate_profile_exo_layer_density(this%ptr_,                        &
                                            real(scale_height, kind=dk), error_c)
    error = error_t(error_c)

  end subroutine calculate_exo_layer_density

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  function get_exo_layer_density(this, error) result(exo_layer_density)
    use iso_c_binding, only: c_size_t
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(profile_t), intent(in)    :: this
    type(error_t),    intent(inout) :: error

    ! Return value
    real(dk) :: exo_layer_density

    ! Local variables
    type(error_t_c) :: error_c

    exo_layer_density = &
        real(get_profile_exo_layer_density_c(this%ptr_, error_c), kind=dk)
    error = error_t(error_c)

  end function get_exo_layer_density
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Deallocate the profile instance
  subroutine finalize_profile(this)
    use iso_c_binding, only: c_associated
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(profile_t), intent(inout) :: this

    ! Local variables
    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (c_associated(this%ptr_)) then
      call delete_profile_c(this%ptr_, error_c)
      this%ptr_ = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if

  end subroutine finalize_profile
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_profile
