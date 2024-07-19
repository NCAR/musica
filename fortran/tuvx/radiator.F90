! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx_radiator
  use iso_c_binding, only: c_ptr, c_null_ptr

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

  private
  public :: radiator_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  interface
    function create_radiator_c(radiator_name, height_grid, wavelength_grid, error) &
        bind(C, name="CreateRadiator")
      use iso_c_binding, only : c_ptr, c_char
      use musica_util, only: error_t_c
      character(len=1, kind=c_char), intent(in) :: radiator_name(*)
      type(c_ptr), intent(in) :: height_grid
      type(c_ptr), intent(in) :: wavelength_grid
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: create_radiator_c
    end function create_radiator_c

    subroutine delete_radiator_c(radiator, error) bind(C, name="DeleteRadiator")
      use iso_c_binding, only : c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: radiator
      type(error_t_c), intent(inout) :: error
    end subroutine delete_radiator_c

    subroutine set_optical_depths_c(radiator, optical_depths, num_vertical_layers, &
        num_wavelength_bins, error) bind(C, name="SetOpticalDepths")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: optical_depths
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      type(error_t_c),          intent(inout) :: error
    end subroutine set_optical_depths_c

    subroutine get_optical_depths_c(radiator, optical_depths, num_vertical_layers, &
        num_wavelength_bins, error) bind(C, name="GetOpticalDepths")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: optical_depths
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      type(error_t_c),          intent(inout) :: error
    end subroutine get_optical_depths_c

    subroutine set_single_scattering_albedos_c(radiator, single_scattering_albedos, & 
        num_vertical_layers, num_wavelength_bins, error) &
        bind(C, name="SetSingleScatteringAlbedos")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: single_scattering_albedos
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      type(error_t_c),          intent(inout) :: error
    end subroutine set_single_scattering_albedos_c

    subroutine get_single_scattering_albedos_c(radiator, single_scattering_albedos, & 
        num_vertical_layers, num_wavelength_bins, error) &
        bind(C, name="GetSingleScatteringAlbedos")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: single_scattering_albedos
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      type(error_t_c),          intent(inout) :: error
    end subroutine get_single_scattering_albedos_c

    subroutine set_asymmetry_factor_c(radiator, symmetry_factor, num_vertical_layers, &
        num_wavelength_bins, num_streams, error) bind(C, name="SetAsymmetryFactor")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: asymmetry_factor
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      integer(c_size_t), value, intent(in)    :: num_streams
      type(error_t_c),          intent(inout) :: error
    end subroutine set_asymmetry_factor_c

    subroutine get_asymmetry_factor_c(radiator, symmetry_factor, num_vertical_layers, &
        num_wavelength_bins, num_streams, error) bind(C, name="GetAsymmetryFactor")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: asymmetry_factor
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      integer(c_size_t), value, intent(in)    :: num_streams
      type(error_t_c),          intent(inout) :: error
    end subroutine get_asymmetry_factor_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  type :: radiator_t
    type(c_ptr) :: ptr_ = c_null_ptr
  contains
    ! Set radiator optical depths
    procedure :: set_optical_depths
    ! Get radiator optical depths
    procedure :: get_optical_depths
    ! Set the radiator single scattering albedos
    procedure :: set_single_scattering_albedos
    ! Get the radiator single scattering albedos
    procedure :: get_single_scattering_albedos
    ! Set the radiator asymmetry_factor
    procedure :: set_asymmetry_factor
    ! Get the radiator asymmetry factor
    procedure :: get_asymmetry_factor
    ! Deallocate the radiator instance
    final :: finalize_radiator_t
  end type radiator_t

  interface radiator_t
    procedure radiator_t_ptr_constructor
    procedure radiator_t_constructor
  end interface radiator_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructs a radiator instance that wraps an existing TUV-x radiator
  function radiator_t_ptr_constructor(radiator_c_ptr) result(this)
    ! Arguments
    type(c_ptr), intent(in) :: radiator_c_ptr

    ! Return value
    type(radiator_t), pointer :: this

    allocate( this )
    this%ptr_ = radiator_c_ptr

  end function radiator_t_ptr_constructor

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructs a radiator instance that allocates a new TUV-x radiator
  function radiator_t_constructor(radiator_name, height_grid, wavelength_grid, error) &
      result(this)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, to_c_string

    ! Arguments
    character(len=*), intent(in)               :: radiator_name
    real(dk), target, dimension(:,:), intent(in) :: height_grid
    real(dk), target, dimension(:,:), intent(in) :: wavelength_grid
    type(error_t), intent(inout)               :: error

    ! Return value
    type(radiator_t), pointer :: this

    type(error_t_c) :: error_c

    allocate( this )
    !!
    !! TODO(jiwon) - is it okay to c_loc() 2d array or should i create a new function 
    !! that does memory layout conversion
    !!
    this%ptr_ = create_radiator_c(to_c_string(radiator_name), c_loc(height_grid), &
                                  c_loc(avelength_grid), error_c)
    error = error_t(error_c)

  end function radiator_t_constructor
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_edges(this, edges, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t), intent(inout) :: this
    real(dk), target, dimension(:), intent(in) :: edges
    type(error_t), intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_edges

    n_edges = size(edges)

    call set_radiator_edges_c(this%ptr_, c_loc(edges), n_edges, error_c)
    error = error_t(error_c)

  end subroutine set_edges

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_edges(this, edges, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t), intent(inout) :: this
    real(dk), target, dimension(:), intent(inout) :: edges
    type(error_t), intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_edges

    n_edges = size(edges)

    call get_radiator_edges_c(this%ptr_, c_loc(edges), n_edges, error_c)
    error = error_t(error_c)

  end subroutine get_edges
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_midpoints(this, midpoints, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t), intent(inout) :: this
    real(dk), target, dimension(:), intent(in) :: midpoints
    type(error_t), intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_midpoints

    n_midpoints = size(midpoints)

    call set_radiator_midpoints_c(this%ptr_, c_loc(midpoints), n_midpoints, error_c)
    error = error_t(error_c)

  end subroutine set_midpoints

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_midpoints(this, midpoints, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t), intent(inout) :: this
    real(dk), target, dimension(:), intent(inout) :: midpoints
    type(error_t), intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_midpoints

    n_midpoints = size(midpoints)

    call get_radiator_midpoints_c(this%ptr_, c_loc(midpoints), n_midpoints, error_c)
    error = error_t(error_c)

  end subroutine get_midpoints
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Deallocate the radiator instance
  subroutine finalize_radiator_t(this)
    use iso_c_binding, only: c_associated
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(radiator_t), intent(inout) :: this

    ! Local variables
    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (c_associated(this%ptr_)) then
        call delete_radiator_c(this%ptr_, error_c)
        this%ptr_ = c_null_ptr
        error = error_t(error_c)
        ASSERT(error%is_success())
    end if

  end subroutine finalize_radiator_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_radiator