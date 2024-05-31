program combined_tuvx_tests
  use iso_c_binding
  use musica_tuvx, only: tuvx_t, grid_map_t
  use musica_util, only: assert, error_t

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )

  ! Call the valid test subroutine
  call test_tuvx_api()

  ! Call the invalid test subroutine
  ! disabling until tuv no longer calls stop internally
  ! call test_tuvx_api_invalid_config()

  ! Call the solve test subroutine
  call test_tuvx_solve()

contains

  ! Valid tuvx solver creation test
  subroutine test_tuvx_api()
    character(len=256)    :: config_path
    logical(c_bool)       :: bool_value
    type(tuvx_t), pointer :: tuvx
    type(error_t)         :: error


    config_path = "examples/ts1_tsmlt.json"

    tuvx => tuvx_t(config_path, error)
    ASSERT( error%is_success() )

  end subroutine test_tuvx_api

  ! Invalid tuvx solver creation test
  subroutine test_tuvx_api_invalid_config()
    type(tuvx_t), pointer :: tuvx
    type(error_t)         :: error
    character(len=256)     :: config_path

    config_path = "invalid_config"

    tuvx => tuvx_t(config_path, error)
    ! ASSERT( .not. error%is_success() )

  end subroutine test_tuvx_api_invalid_config

  subroutine test_tuvx_solve()

    type(tuvx_t), pointer :: tuvx
    type(error_t)         :: error
    type(grid_map_t) :: grids
    character(len=256) :: config_path

    config_path = "examples/ts1_tsmlt.json"
    ! type(profile_map_t) :: profiles
    ! type(radiator_map_t) :: radiators
    ! type(grid_t), pointer :: grid
    ! real(dk), allocatable :: photo_rates(:,:,:)

    tuvx => tuvx_t( config_path, error )
    ASSERT( error%is_success() )
    grids = tuvx%get_grids( error )
    ASSERT( error%is_success() )

    ! profiles = tuvx%create_profiles( )
    ! radiators = tuvx%create_radiators( )

    ! update conditions for each time step
    ! grid => grids%get( "height", "m" )
    ! grid%edges_(:) = some_input_array(:)
    ! grid%mid_points_(:) = more_input_data(:)
    
    ! profile => profiles%get( "O3", "mol m-3" )


    ! call tuvx%solve( grids, profiles, radiators, photo_rates )


  end subroutine test_tuvx_solve

end program combined_tuvx_tests
