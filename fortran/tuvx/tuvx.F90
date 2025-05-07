! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx
  use iso_c_binding,            only: c_ptr, c_null_ptr
  use musica_tuvx_grid,         only : grid_t
  use musica_tuvx_grid_map,     only : grid_map_t
  use musica_tuvx_profile,      only : profile_t
  use musica_tuvx_profile_map,  only : profile_map_t
  use musica_tuvx_radiator,     only : radiator_t
  use musica_tuvx_radiator_map, only : radiator_map_t

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

  private
  public :: tuvx_t, grid_map_t, grid_t, profile_map_t, profile_t, &
            radiator_map_t, radiator_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  interface
    function create_tuvx_c(config_path, grids, profiles, radiators, error) &
        bind(C, name="CreateTuvx")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr, c_int, c_char
      character(len=1, kind=c_char), intent(in)    :: config_path(*)
      type(c_ptr),            value, intent(in)    :: grids
      type(c_ptr),            value, intent(in)    :: profiles
      type(c_ptr),            value, intent(in)    :: radiators
      type(error_t_c),               intent(inout) :: error
      type(c_ptr)                                  :: create_tuvx_c
    end function create_tuvx_c

    subroutine delete_tuvx_c(tuvx, error) bind(C, name="DeleteTuvx")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: tuvx
      type(error_t_c), intent(inout) :: error
    end subroutine delete_tuvx_c

    function get_grid_map_c(tuvx, error) bind(C, name="GetGridMap")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: tuvx
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: get_grid_map_c
    end function get_grid_map_c

    function get_profile_map_c(tuvx, error) bind(C, name="GetProfileMap")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: tuvx
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: get_profile_map_c
    end function get_profile_map_c

    function get_radiator_map_c(tuvx, error) bind(C, name="GetRadiatorMap")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: tuvx
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: get_radiator_map_c
    end function get_radiator_map_c

    function get_photolysis_rate_constants_ordering_c(tuvx, error) &
        bind(C, name="GetPhotolysisRateConstantsOrdering")
      use musica_util, only: error_t_c, mappings_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: tuvx
      type(error_t_c), intent(inout) :: error
      type(mappings_t_c)             :: get_photolysis_rate_constants_ordering_c
    end function get_photolysis_rate_constants_ordering_c

    function get_heating_rates_ordering_c(tuvx, error) &
        bind(C, name="GetHeatingRatesOrdering")
      use musica_util, only: error_t_c, mappings_t_c
      use iso_c_binding, only: c_ptr
      type(c_ptr), value, intent(in) :: tuvx
      type(error_t_c), intent(inout) :: error
      type(mappings_t_c)             :: get_heating_rates_ordering_c
    end function get_heating_rates_ordering_c

    subroutine run_tuvx_c(tuvx, solar_zenith_angle, earth_sun_distance, &
        photolysis_rate_constants, heating_rates, error) bind(C, name="RunTuvx")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr, c_double
      type(c_ptr), value,         intent(in)    :: tuvx
      real(kind=c_double), value, intent(in)    :: solar_zenith_angle
      real(kind=c_double), value, intent(in)    :: earth_sun_distance
      type(c_ptr), value,         intent(in)    :: photolysis_rate_constants
      type(c_ptr), value,         intent(in)    :: heating_rates
      type(error_t_c),            intent(inout) :: error
    end subroutine run_tuvx_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  type :: tuvx_t
    type(c_ptr), private :: ptr_ = c_null_ptr
  contains
    ! Create a grid map
    procedure :: get_grids
    ! Create a profile map
    procedure :: get_profiles
    ! Create a radiator map
    procedure :: get_radiators
    ! Get photolysis rate constant ordering
    procedure :: get_photolysis_rate_constants_ordering
    ! Get heating rate ordering
    procedure :: get_heating_rates_ordering
    ! Run the calculator
    procedure :: run
    ! Deallocate the tuvx instance
    final :: finalize
  end type tuvx_t

  interface tuvx_t
    procedure constructor
  end interface tuvx_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Construct a tuvx instance
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
    type(tuvx_t), pointer         :: this

    allocate( this )

    n = len_trim(config_path)
    do i = 1, n
      c_config_path(i) = config_path(i:i)
    end do
    c_config_path(n+1) = c_null_char

    this%ptr_ = create_tuvx_c(c_config_path, grids%ptr_, profiles%ptr_, &
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
    class(tuvx_t), intent(inout) :: this
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
    class(tuvx_t), intent(inout) :: this
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
    class(tuvx_t), intent(inout)  :: this
    type(error_t), intent(inout)  :: error

    ! Local variables
    type(error_t_c)               :: error_c

    ! Return value
    type(radiator_map_t), pointer :: radiator_map

    radiator_map => radiator_map_t(get_radiator_map_c(this%ptr_, error_c))
    
    error = error_t(error_c)

  end function get_radiators

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Get the photolysis rate constant ordering
  function get_photolysis_rate_constants_ordering(this, error) &
      result(mappings)
    use musica_util, only: error_t, error_t_c, mappings_t, mappings_t_c

    ! Arguments
    class(tuvx_t), intent(inout) :: this
    type(error_t), intent(inout) :: error

    ! Return value
    type(mappings_t), pointer :: mappings 

    ! Local variables
    type(error_t_c) :: error_c
    type(mappings_t_c) :: mapping

    mapping = get_photolysis_rate_constants_ordering_c(this%ptr_, error_c)
    mappings =>  mappings_t( mapping )
    error = error_t(error_c)

  end function get_photolysis_rate_constants_ordering
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Get the heating rate ordering
  function get_heating_rates_ordering(this, error) result(mappings)
    use musica_util, only: error_t, error_t_c, mappings_t, mappings_t_c

    ! Arguments
    class(tuvx_t), intent(inout) :: this
    type(error_t), intent(inout) :: error

    ! Return value
    type(mappings_t), pointer :: mappings

    ! Local variables
    type(error_t_c) :: error_c
    type(mappings_t_c) :: mapping

    mapping = get_heating_rates_ordering_c(this%ptr_, error_c)

    mappings => mappings_t( mapping )
    error = error_t(error_c)

  end function get_heating_rates_ordering
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Run the calculator
  subroutine run(this, solar_zenith_angle, earth_sun_distance, &
      photolysis_rate_constants, heating_rates, error)
    use iso_c_binding, only: c_double, c_ptr, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(tuvx_t),                     intent(inout) :: this
    real(kind=dk),                     intent(in)    :: solar_zenith_angle             ! radians
    real(kind=dk),                     intent(in)    :: earth_sun_distance             ! AU
    real(kind=dk), target, contiguous, intent(inout) :: photolysis_rate_constants(:,:) ! s-1 (layer, reaction)
    real(kind=dk), target, contiguous, intent(inout) :: heating_rates(:,:)             ! K s-1 (layer, reaction)
    type(error_t),                     intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    type(c_ptr)     :: photo_rate_c, heating_c

    photo_rate_c = c_loc(photolysis_rate_constants(1,1))
    heating_c    = c_loc(heating_rates(1,1))
    call run_tuvx_c(this%ptr_, &
                    real(solar_zenith_angle, kind=c_double), &
                    real(earth_sun_distance, kind=c_double), &
                    photo_rate_c, heating_c, error_c)
    error = error_t(error_c)

  end subroutine run
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Deallocate the tuvx instance
  subroutine finalize(this)
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(tuvx_t), intent(inout) :: this

    ! Local variables
    type(error_t_c)             :: error_c
    type(error_t)               :: error

    call delete_tuvx_c(this%ptr_, error_c)
    this%ptr_ = c_null_ptr
    error = error_t(error_c)
    ASSERT(error%is_success())

  end subroutine finalize

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx
