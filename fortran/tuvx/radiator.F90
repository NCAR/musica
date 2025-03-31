! Copyright (C) 2023-2025 National Center for Atmospheric Research
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
      character(len=1, kind=c_char), intent(in)    :: radiator_name(*)
      type(c_ptr),            value, intent(in)    :: height_grid
      type(c_ptr),            value, intent(in)    :: wavelength_grid
      type(error_t_c),               intent(inout) :: error
      type(c_ptr) :: create_radiator_c
    end function create_radiator_c

    subroutine delete_radiator_c(radiator, error) bind(C, name="DeleteRadiator")
      use iso_c_binding, only : c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in)    :: radiator
      type(error_t_c),    intent(inout) :: error
    end subroutine delete_radiator_c

    subroutine set_optical_depths_c(radiator, optical_depths, num_vertical_layers, &
        num_wavelength_bins, error) bind(C, name="SetRadiatorOpticalDepths")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: optical_depths
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      type(error_t_c),          intent(inout) :: error
    end subroutine set_optical_depths_c

    subroutine get_optical_depths_c(radiator, optical_depths, num_vertical_layers, &
        num_wavelength_bins, error) bind(C, name="GetRadiatorOpticalDepths")
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
        bind(C, name="SetRadiatorSingleScatteringAlbedos")
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
        bind(C, name="GetRadiatorSingleScatteringAlbedos")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: single_scattering_albedos
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      type(error_t_c),          intent(inout) :: error
    end subroutine get_single_scattering_albedos_c

    subroutine set_asymmetry_factors_c(radiator, asymmetry_factors, num_vertical_layers, &
        num_wavelength_bins, num_streams, error) bind(C, name="SetRadiatorAsymmetryFactors")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: asymmetry_factors
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      integer(c_size_t), value, intent(in)    :: num_streams
      type(error_t_c),          intent(inout) :: error
    end subroutine set_asymmetry_factors_c

    subroutine get_asymmetry_factors_c(radiator, asymmetry_factors, num_vertical_layers, &
        num_wavelength_bins, num_streams, error) bind(C, name="GetRadiatorAsymmetryFactors")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: radiator
      type(c_ptr),       value, intent(in)    :: asymmetry_factors
      integer(c_size_t), value, intent(in)    :: num_vertical_layers
      integer(c_size_t), value, intent(in)    :: num_wavelength_bins
      integer(c_size_t), value, intent(in)    :: num_streams
      type(error_t_c),          intent(inout) :: error
    end subroutine get_asymmetry_factors_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  type :: radiator_t
    type(c_ptr) :: ptr_ = c_null_ptr
  contains
    ! Set radiator optical depths
    procedure :: set_optical_depths
    ! Get radiator optical depths
    procedure :: get_optical_depths
    ! Set radiator single scattering albedos
    procedure :: set_single_scattering_albedos
    ! Get radiator single scattering albedos
    procedure :: get_single_scattering_albedos
    ! Set radiator asymmetry_factors
    procedure :: set_asymmetry_factors
    ! Get radiator asymmetry factors
    procedure :: get_asymmetry_factors
    ! Deallocate radiator instance
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
  function radiator_t_constructor(radiator_name, height_grid, wavelength_grid, &
      error) result(this)
    use musica_tuvx_grid, only: grid_t
    use musica_util, only: error_t, error_t_c, to_c_string

    ! Arguments
    character(len=*), intent(in)    :: radiator_name
    type(grid_t),     intent(in)    :: height_grid
    type(grid_t),     intent(in)    :: wavelength_grid
    type(error_t),    intent(inout) :: error

    ! Return value
    type(radiator_t), pointer :: this

    ! Local variables
    type(error_t_c) :: error_c

    allocate( this )
    this%ptr_ = create_radiator_c(to_c_string(radiator_name), height_grid%ptr_, &
                                  wavelength_grid%ptr_, error_c)
    error = error_t(error_c)

  end function radiator_t_constructor
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_optical_depths(this, optical_depths, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t),            intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: optical_depths(:,:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c)        :: error_c
    integer(kind=c_size_t) :: num_vertical_layers
    integer(kind=c_size_t) :: num_wavelength_bins

    num_vertical_layers = size(optical_depths, 1)
    num_wavelength_bins = size(optical_depths, 2)
    
    call set_optical_depths_c(this%ptr_, c_loc(optical_depths), &
                num_vertical_layers, num_wavelength_bins, error_c)
    error = error_t(error_c)

  end subroutine set_optical_depths

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_optical_depths(this, optical_depths, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t),            intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: optical_depths(:,:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c)        :: error_c
    integer(kind=c_size_t) :: num_vertical_layers
    integer(kind=c_size_t) :: num_wavelength_bins

    num_vertical_layers = size(optical_depths, 1)
    num_wavelength_bins = size(optical_depths, 2)

    call get_optical_depths_c(this%ptr_, c_loc(optical_depths), &
                num_vertical_layers, num_wavelength_bins, error_c)
    error = error_t(error_c)

  end subroutine get_optical_depths
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_single_scattering_albedos(this, single_scattering_albedos, &
      error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t),            intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: single_scattering_albedos(:,:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: num_vertical_layers
    integer(kind=c_size_t) :: num_wavelength_bins

    num_vertical_layers = size(single_scattering_albedos, 1)
    num_wavelength_bins = size(single_scattering_albedos, 2)

    call set_single_scattering_albedos_c(this%ptr_, &
          c_loc(single_scattering_albedos), num_vertical_layers, &
          num_wavelength_bins, error_c)
    error = error_t(error_c)

  end subroutine set_single_scattering_albedos

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_single_scattering_albedos(this, single_scattering_albedos, &
      error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t),            intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: single_scattering_albedos(:,:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c)        :: error_c
    integer(kind=c_size_t) :: num_vertical_layers
    integer(kind=c_size_t) :: num_wavelength_bins

    num_vertical_layers = size(single_scattering_albedos, 1)
    num_wavelength_bins = size(single_scattering_albedos, 2)

    call get_single_scattering_albedos_c(this%ptr_, &
          c_loc(single_scattering_albedos), num_vertical_layers, &
          num_wavelength_bins, error_c)
    error = error_t(error_c)

  end subroutine get_single_scattering_albedos
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_asymmetry_factors(this, asymmetry_factors, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t),            intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: asymmetry_factors(:,:,:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: num_vertical_layers
    integer(kind=c_size_t) :: num_wavelength_bins
    integer(kind=c_size_t) :: num_streams

    num_vertical_layers = size(asymmetry_factors, 1)
    num_wavelength_bins = size(asymmetry_factors, 2)
    num_streams = size(asymmetry_factors, 3)

    call set_asymmetry_factors_c(this%ptr_, c_loc(asymmetry_factors), & 
          num_vertical_layers, num_wavelength_bins, num_streams, error_c)
    error = error_t(error_c)

end subroutine set_asymmetry_factors

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine get_asymmetry_factors(this, asymmetry_factors, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(radiator_t),            intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: asymmetry_factors(:,:,:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: num_vertical_layers
    integer(kind=c_size_t) :: num_wavelength_bins
    integer(kind=c_size_t) :: num_streams

    num_vertical_layers = size(asymmetry_factors, 1)
    num_wavelength_bins = size(asymmetry_factors, 2)
    num_streams = size(asymmetry_factors, 3)

    call get_asymmetry_factors_c(this%ptr_, c_loc(asymmetry_factors), & 
          num_vertical_layers, num_wavelength_bins, num_streams, error_c)
    error = error_t(error_c)

  end subroutine get_asymmetry_factors

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