! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program combined_tuvx_tests
  use iso_c_binding
  use musica_tuvx, only: tuvx_t, grid_map_t, grid_t, profile_map_t, profile_t
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

    deallocate( tuvx )

  end subroutine test_tuvx_api

  ! Invalid tuvx solver creation test
  subroutine test_tuvx_api_invalid_config()
    type(tuvx_t), pointer :: tuvx
    type(error_t)         :: error
    character(len=256)    :: config_path

    config_path = "invalid_config"

    tuvx => tuvx_t(config_path, error)
    ASSERT( .not. error%is_success() )

  end subroutine test_tuvx_api_invalid_config

  subroutine test_tuvx_solve()

    type(tuvx_t), pointer     :: tuvx
    type(error_t)             :: error
    type(grid_map_t)          :: grids
    character(len=256)        :: config_path
    type(grid_t), pointer     :: grid
    type(profile_map_t)       :: profiles
    type(profile_t), pointer  :: profile
    ! type(profile_map_t) :: profiles
    ! type(radiator_map_t) :: radiators
    real*8, dimension(5) :: edges, edge_values
    real*8, dimension(4) :: midpoints, midpoint_values, layer_densities

    edges = (/ 1.0, 2.0, 3.0, 4.0, 5.0 /)
    midpoints = (/ 1.5, 2.5, 3.5, 4.5 /)  
    edge_values = (/ 10.0, 20.0, 30.0, 40.0, 50.0 /)
    midpoint_values = (/ 15.0, 25.0, 35.0, 45.0 /)
    layer_densities = (/ 2.0, 4.0, 1.0, 7.0 /)

    config_path = "examples/ts1_tsmlt.json"

    tuvx => tuvx_t( config_path, error )
    ASSERT( error%is_success() )
    grids = tuvx%get_grids( error )
    ASSERT( error%is_success() )

    grid => grids%get( "height", "km", error )
    ASSERT( error%is_success() )

    call grid%set_edges( edges, error )
    ASSERT( error%is_success() )

    call grid%set_midpoints( midpoints, error )
    ASSERT( error%is_success() )

    profiles = tuvx%get_profiles( error )
    ASSERT( error%is_success() )

    profile => profiles%get( "temperature", "K", error )
    ASSERT( error%is_success() )

    call profile%set_edge_values( edge_values, error )
    ASSERT( error%is_success() )

    call profile%set_midpoint_values( midpoint_values, error )
    ASSERT( error%is_success() )

    call profile%set_layer_densities( layer_densities, error )
    ASSERT( error%is_success() )

    call profile%set_exo_layer_density( 42.0d0, error )
    ASSERT( error%is_success() )

    call profile%calculate_exo_layer_density( 13.1d0, error )
    ASSERT( error%is_success() )
    
    deallocate( tuvx )
    deallocate( grid )
    deallocate( profile )

  end subroutine test_tuvx_solve

end program combined_tuvx_tests
