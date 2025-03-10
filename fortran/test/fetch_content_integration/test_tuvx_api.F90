! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program combined_tuvx_tests
  use iso_c_binding
  use musica_tuvx, only: tuvx_t, grid_map_t, grid_t, profile_map_t, profile_t, &
                         radiator_map_t, radiator_t
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
    character(len=256)            :: config_path
    type(grid_map_t),     pointer :: grids
    type(profile_map_t),  pointer :: profiles
    type(radiator_map_t), pointer :: radiators
    type(tuvx_t),         pointer :: tuvx
    type(error_t)                 :: error

    config_path = "examples/ts1_tsmlt.json"

    grids => grid_map_t( error )
    ASSERT( error%is_success() )
    profiles => profile_map_t( error )
    ASSERT( error%is_success() )
    radiators => radiator_map_t( error )
    ASSERT( error%is_success() )
    tuvx => tuvx_t(config_path, grids, profiles, radiators, error)
    ASSERT( error%is_success() )

    deallocate( grids )
    deallocate( profiles )
    deallocate( radiators )
    deallocate( tuvx )

  end subroutine test_tuvx_api

  ! Invalid tuvx solver creation test
  subroutine test_tuvx_api_invalid_config()
    character(len=256)            :: config_path
    type(grid_map_t),     pointer :: grids
    type(profile_map_t),  pointer :: profiles
    type(radiator_map_t), pointer :: radiators
    type(tuvx_t),         pointer :: tuvx
    type(error_t)                 :: error
    config_path = "invalid_config"

    grids => grid_map_t( error )
    ASSERT( error%is_success() )
    profiles => profile_map_t( error )
    ASSERT( error%is_success() )
    radiators => radiator_map_t( error )
    ASSERT( error%is_success() )
    tuvx => tuvx_t(config_path, grids, profiles, radiators, error)
    ASSERT( .not. error%is_success() )

    deallocate( grids )
    deallocate( profiles )
    deallocate( radiators )

  end subroutine test_tuvx_api_invalid_config

  subroutine test_tuvx_solve()

    type(tuvx_t),         pointer      :: tuvx
    type(error_t)                      :: error
    character(len=256)                 :: config_path
    type(grid_map_t),     pointer      :: grids, grids_from_host
    type(grid_t),         pointer      :: grid, height_grid, wavelength_grid
    type(profile_map_t),  pointer      :: profiles, profiles_from_host
    type(profile_t),      pointer      :: profile, profile_copy
    type(radiator_map_t), pointer      :: radiators, radiators_from_host
    type(radiator_t),     pointer      :: radiator, radiator_copy
    ! set up arrays with extra dimensions to test whether arrays passed to
    ! c functions are contiguous
    real*8, dimension(3,5), target     :: edges, edge_values, temp_edge
    real*8, dimension(2,4), target     :: midpoints, midpoint_values, layer_densities, temp_midpoint
    real*8                             :: temp_real
    integer                            :: num_vertical_layers, num_wavelength_bins
    real*8, dimension(4,3,2), target   :: optical_depths, temp_od
    real*8, dimension(3,3,2), target   :: single_scattering_albedos, temp_ssa
    real*8, dimension(2,3,2,1), target :: asymmetry_factors, temp_asym

    edges(2,:) = (/ 1.0, 2.0, 3.0, 4.0, 5.0 /)
    midpoints(2,:) = (/ 15.0, 25.0, 35.0, 45.0 /)  
    edge_values(2,:) = (/ 10.0, 20.0, 30.0, 40.0, 50.0 /)
    midpoint_values(2,:) = (/ 15.0, 25.0, 35.0, 45.0 /)
    layer_densities(2,:) = (/ 2.0, 4.0, 1.0, 7.0 /)
    num_vertical_layers = 3
    num_wavelength_bins = 2
    optical_depths(2,:,1) = (/ 30.0, 20.0, 10.0 /)
    optical_depths(2,:,2) = (/ 70.0, 80.0, 90.0 /)
    single_scattering_albedos(2,:,1) = (/ 300.0, 200.0, 100.0 /)
    single_scattering_albedos(2,:,2) = (/ 700.0, 800.0, 900.0 /)
    asymmetry_factors(2,:,1,1) = (/ 3.0, 2.0, 1.0 /)
    asymmetry_factors(2,:,2,1) = (/ 7.0, 8.0, 9.0 /)

    config_path = "examples/ts1_tsmlt.json"

    grids_from_host => grid_map_t( error )
    ASSERT( error%is_success() )
    profiles_from_host => profile_map_t( error )
    ASSERT( error%is_success() )
    radiators_from_host => radiator_map_t( error )
    ASSERT( error%is_success() )
    tuvx => tuvx_t(config_path, grids_from_host, profiles_from_host, radiators_from_host, error)
    ASSERT( error%is_success() )

    grids => tuvx%get_grids( error )
    ASSERT( error%is_success() )

    grid => grids%get( "height", "km", error )
    ASSERT( .not. error%is_success() ) ! non-accessible grid
    deallocate( grid )
    deallocate( grids )

    grids => grid_map_t( error )
    ASSERT( error%is_success() )

    grid => grid_t( "foo", "bars", 4, error )
    ASSERT( error%is_success() )

    ASSERT_EQ( grid%number_of_sections( error ), 4 )
    ASSERT( error%is_success() )

    call grid%set_edges( edges(2,:), error )
    ASSERT( error%is_success() )

    call grid%get_edges( temp_edge(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_edge(2,1), 1.0 )
    ASSERT_EQ( temp_edge(2,2), 2.0 )
    ASSERT_EQ( temp_edge(2,3), 3.0 )
    ASSERT_EQ( temp_edge(2,4), 4.0 )
    ASSERT_EQ( temp_edge(2,5), 5.0 )

    edges(2,:) = (/ 10.0, 20.0, 30.0, 40.0, 50.0 /)

    call grid%set_edges( edges(2,:), error )
    ASSERT( error%is_success() )
    call grid%set_midpoints( midpoints(2,:), error )
    ASSERT( error%is_success() )

    call grid%get_edges( temp_edge(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_edge(2,1), 10.0 )
    ASSERT_EQ( temp_edge(2,2), 20.0 )
    ASSERT_EQ( temp_edge(2,3), 30.0 )
    ASSERT_EQ( temp_edge(2,4), 40.0 )
    ASSERT_EQ( temp_edge(2,5), 50.0 )

    call grid%get_midpoints( temp_midpoint(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_midpoint(2,1), 15.0 )
    ASSERT_EQ( temp_midpoint(2,2), 25.0 )
    ASSERT_EQ( temp_midpoint(2,3), 35.0 )
    ASSERT_EQ( temp_midpoint(2,4), 45.0 )

    call grids%add( grid, error )

    edges(2,:) = 0.0
    midpoints(2,:) = 0.0

    call grid%get_edges( edges(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( edges(2,1), 10.0 )
    ASSERT_EQ( edges(2,2), 20.0 )
    ASSERT_EQ( edges(2,3), 30.0 )
    ASSERT_EQ( edges(2,4), 40.0 )
    ASSERT_EQ( edges(2,5), 50.0 )

    call grid%get_midpoints( midpoints(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( midpoints(2,1), 15.0 )
    ASSERT_EQ( midpoints(2,2), 25.0 )
    ASSERT_EQ( midpoints(2,3), 35.0 )
    ASSERT_EQ( midpoints(2,4), 45.0 )

    edges(2,:) = (/ 1.0, 2.0, 3.0, 4.0, 5.0 /)
    midpoints(2,:) = (/ 1.5, 2.5, 3.5, 4.5 /)

    call grid%set_edges( edges(2,:), error )
    ASSERT( error%is_success() )
    call grid%set_midpoints( midpoints(2,:), error )
    ASSERT( error%is_success() )

    edges(2,:) = 0.0
    midpoints(2,:) = 0.0

    call grid%get_edges( edges(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( edges(2,1), 1.0 )
    ASSERT_EQ( edges(2,2), 2.0 )
    ASSERT_EQ( edges(2,3), 3.0 )
    ASSERT_EQ( edges(2,4), 4.0 )
    ASSERT_EQ( edges(2,5), 5.0 )

    call grid%get_midpoints( midpoints(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( midpoints(2,1), 1.5 )
    ASSERT_EQ( midpoints(2,2), 2.5 )
    ASSERT_EQ( midpoints(2,3), 3.5 )
    ASSERT_EQ( midpoints(2,4), 4.5 )

    deallocate( grid )

    grid => grids%get( "foo", "bars", error )
    ASSERT( error%is_success() )

    edges(2,:) = 0.0
    midpoints(2,:) = 0.0

    call grid%get_edges( edges(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( edges(2,1), 1.0 )
    ASSERT_EQ( edges(2,2), 2.0 )
    ASSERT_EQ( edges(2,3), 3.0 )
    ASSERT_EQ( edges(2,4), 4.0 )
    ASSERT_EQ( edges(2,5), 5.0 )

    call grid%get_midpoints( midpoints(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( midpoints(2,1), 1.5 )
    ASSERT_EQ( midpoints(2,2), 2.5 )
    ASSERT_EQ( midpoints(2,3), 3.5 )
    ASSERT_EQ( midpoints(2,4), 4.5 )

    edges(2,:) = (/ 10.0, 20.0, 30.0, 40.0, 50.0 /)
    midpoints(2,:) = (/ 15.0, 25.0, 35.0, 45.0 /)

    call grid%set_edges( edges(2,:), error )
    call grid%set_midpoints( midpoints(2,:), error )
    ASSERT( error%is_success() )

    edges(2,:) = 0.0
    midpoints(2,:) = 0.0

    call grid%get_edges( edges(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( edges(2,1), 10.0 )
    ASSERT_EQ( edges(2,2), 20.0 )
    ASSERT_EQ( edges(2,3), 30.0 )
    ASSERT_EQ( edges(2,4), 40.0 )
    ASSERT_EQ( edges(2,5), 50.0 )

    call grid%get_midpoints( midpoints(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( midpoints(2,1), 15.0 )
    ASSERT_EQ( midpoints(2,2), 25.0 )
    ASSERT_EQ( midpoints(2,3), 35.0 )
    ASSERT_EQ( midpoints(2,4), 45.0 )

    profiles => tuvx%get_profiles( error )
    ASSERT( error%is_success() )

    profile => profiles%get( "temperature", "K", error )
    ASSERT( .not. error%is_success() ) ! non-accessible profile
    deallocate( profile )
    deallocate( profiles )

    profiles => profile_map_t( error )
    ASSERT( error%is_success() )

    profile => profile_t( "baz", "qux", grid, error )
    ASSERT( error%is_success() )

    call profile%set_edge_values( edge_values(2,:), error )
    ASSERT( error%is_success() )

    call profile%get_edge_values( temp_edge(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_edge(2,1), 10.0 )
    ASSERT_EQ( temp_edge(2,2), 20.0 )
    ASSERT_EQ( temp_edge(2,3), 30.0 )
    ASSERT_EQ( temp_edge(2,4), 40.0 )
    ASSERT_EQ( temp_edge(2,5), 50.0 )

    call profile%set_midpoint_values( midpoint_values(2,:), error )
    ASSERT( error%is_success() )

    call profile%get_midpoint_values( temp_midpoint(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_midpoint(2,1), 15.0 )
    ASSERT_EQ( temp_midpoint(2,2), 25.0 )
    ASSERT_EQ( temp_midpoint(2,3), 35.0 )
    ASSERT_EQ( temp_midpoint(2,4), 45.0 )

    call profile%set_layer_densities( layer_densities(2,:), error )
    ASSERT( error%is_success() )

    call profile%get_layer_densities( temp_midpoint(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_midpoint(2,1), 2.0 )
    ASSERT_EQ( temp_midpoint(2,2), 4.0 )
    ASSERT_EQ( temp_midpoint(2,3), 1.0 )
    ASSERT_EQ( temp_midpoint(2,4), 7.0 )

    call profile%set_exo_layer_density( 1.0d0, error )
    ASSERT( error%is_success() )

    temp_real = profile%get_exo_layer_density( error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_real, 1.0 )

    call profile%get_layer_densities( temp_midpoint(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_midpoint(2,1), 2.0 )
    ASSERT_EQ( temp_midpoint(2,2), 4.0 )
    ASSERT_EQ( temp_midpoint(2,3), 1.0 )
    ASSERT_EQ( temp_midpoint(2,4), 7.0 + 1.0 )

    call profile%calculate_exo_layer_density( 10.0d0, error )
    ASSERT( error%is_success() )

    temp_real = profile%get_exo_layer_density( error )
    ASSERT( error%is_success() )
    ! Revisit this after non-SI units are converted in the TUV-x internal functions
    ASSERT_EQ( temp_real, 10.0 * 7.0 * 100.0 )

    call profile%get_layer_densities( temp_midpoint(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_midpoint(2,1), 2.0 )
    ASSERT_EQ( temp_midpoint(2,2), 4.0 )
    ASSERT_EQ( temp_midpoint(2,3), 1.0 )
    ASSERT_EQ( temp_midpoint(2,4), 7.0 + 10.0 * 7.0 * 100.0 )

    call profiles%add( profile, error )
    profile_copy => profiles%get( "baz", "qux", error )

    call profile_copy%get_edge_values( temp_edge(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_edge(2,1), 10.0 )
    ASSERT_EQ( temp_edge(2,2), 20.0 )
    ASSERT_EQ( temp_edge(2,3), 30.0 )
    ASSERT_EQ( temp_edge(2,4), 40.0 )
    ASSERT_EQ( temp_edge(2,5), 50.0 )

    edge_values(2,:) = (/ 32.0, 34.0, 36.0, 38.0, 40.0 /)
    call profile_copy%set_edge_values( edge_values(2,:), error )

    call profile%get_edge_values( temp_edge(2,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_edge(2,1), 32.0 )
    ASSERT_EQ( temp_edge(2,2), 34.0 )
    ASSERT_EQ( temp_edge(2,3), 36.0 )
    ASSERT_EQ( temp_edge(2,4), 38.0 )
    ASSERT_EQ( temp_edge(2,5), 40.0 )

    radiators => tuvx%get_radiators( error )
    ASSERT( error%is_success() )

    radiator => radiators%get( "foo_radiator", error )
    ASSERT( .not. error%is_success() )
    deallocate( radiator )
    deallocate( radiators )

    radiators =>radiator_map_t( error )
    ASSERT( error%is_success() )

    height_grid => grid_t( "height", "km", num_vertical_layers, error )
    wavelength_grid => grid_t( "wavelength", "nm", num_wavelength_bins, error )
    radiator => radiator_t( "foo_radiator", height_grid, wavelength_grid, error )
    ASSERT( error%is_success() )

    call radiator%set_optical_depths( optical_depths(2,:,:), error )
    ASSERT( error%is_success() )

    call radiator%get_optical_depths( temp_od(2,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_od(2,1,1), 30.0 )
    ASSERT_EQ( temp_od(2,2,1), 20.0 )
    ASSERT_EQ( temp_od(2,3,1), 10.0 )
    ASSERT_EQ( temp_od(2,1,2), 70.0 )
    ASSERT_EQ( temp_od(2,2,2), 80.0 )
    ASSERT_EQ( temp_od(2,3,2), 90.0 )

    call radiator%set_single_scattering_albedos( single_scattering_albedos(2,:,:), error )
    ASSERT( error%is_success() )

    call radiator%get_single_scattering_albedos( temp_ssa(2,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_ssa(2,1,1), 300.0 )
    ASSERT_EQ( temp_ssa(2,2,1), 200.0 )
    ASSERT_EQ( temp_ssa(2,3,1), 100.0 )
    ASSERT_EQ( temp_ssa(2,1,2), 700.0 )
    ASSERT_EQ( temp_ssa(2,2,2), 800.0 )
    ASSERT_EQ( temp_ssa(2,3,2), 900.0 )

    call radiator%set_asymmetry_factors( asymmetry_factors(2,:,:,:), error )
    ASSERT( error%is_success() )

    call radiator%get_asymmetry_factors( temp_asym(2,:,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_asym(2,1,1,1), 3.0 )
    ASSERT_EQ( temp_asym(2,2,1,1), 2.0 )
    ASSERT_EQ( temp_asym(2,3,1,1), 1.0 )
    ASSERT_EQ( temp_asym(2,1,2,1), 7.0 )
    ASSERT_EQ( temp_asym(2,2,2,1), 8.0 )
    ASSERT_EQ( temp_asym(2,3,2,1), 9.0 )
!
    call radiators%add( radiator, error )
    radiator_copy => radiators%get( "foo_radiator", error )

    optical_depths(2,:,:) = 0.0
    single_scattering_albedos(2,:,:) = 0.0
    asymmetry_factors(2,:,:,:) = 0.0

    call radiator_copy%get_optical_depths( optical_depths(2,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( optical_depths(2,1,1), 30.0 )
    ASSERT_EQ( optical_depths(2,2,1), 20.0 )
    ASSERT_EQ( optical_depths(2,3,1), 10.0 )
    ASSERT_EQ( optical_depths(2,1,2), 70.0 )
    ASSERT_EQ( optical_depths(2,2,2), 80.0 )
    ASSERT_EQ( optical_depths(2,3,2), 90.0 )

    call radiator_copy%get_single_scattering_albedos( single_scattering_albedos(2,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( single_scattering_albedos(2,1,1), 300.0 )
    ASSERT_EQ( single_scattering_albedos(2,2,1), 200.0 )
    ASSERT_EQ( single_scattering_albedos(2,3,1), 100.0 )
    ASSERT_EQ( single_scattering_albedos(2,1,2), 700.0 )
    ASSERT_EQ( single_scattering_albedos(2,2,2), 800.0 )
    ASSERT_EQ( single_scattering_albedos(2,3,2), 900.0 )

    call radiator_copy%get_asymmetry_factors( asymmetry_factors(2,:,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( asymmetry_factors(2,1,1,1), 3.0 )
    ASSERT_EQ( asymmetry_factors(2,2,1,1), 2.0 )
    ASSERT_EQ( asymmetry_factors(2,3,1,1), 1.0 )
    ASSERT_EQ( asymmetry_factors(2,1,2,1), 7.0 )
    ASSERT_EQ( asymmetry_factors(2,2,2,1), 8.0 )
    ASSERT_EQ( asymmetry_factors(2,3,2,1), 9.0 )

    optical_depths(2,:,1) = (/ 90.0, 80.0, 70.0 /)
    optical_depths(2,:,2) = (/ 75.0, 85.0, 95.0 /)
    single_scattering_albedos(2,:,1) = (/ 900.0, 800.0, 700.0 /)
    single_scattering_albedos(2,:,2) = (/ 750.0, 850.0, 950.0 /)
    asymmetry_factors(2,:,1,1) = (/ 9.0, 8.0, 7.0 /)
    asymmetry_factors(2,:,2,1) = (/ 5.0, 4.0, 3.0 /)

    call radiator_copy%set_optical_depths( optical_depths(2,:,:), error )
    call radiator_copy%set_single_scattering_albedos( single_scattering_albedos(2,:,:), error )
    call radiator_copy%set_asymmetry_factors( asymmetry_factors(2,:,:,:), error )
    ASSERT( error%is_success() )

    optical_depths(:,:,:) = 0.0
    single_scattering_albedos(:,:,:) = 0.0
    asymmetry_factors(:,:,:,:) = 0.0

    call radiator%get_optical_depths( optical_depths(2,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( optical_depths(2,1,1), 90.0 )
    ASSERT_EQ( optical_depths(2,2,1), 80.0 )
    ASSERT_EQ( optical_depths(2,3,1), 70.0 )
    ASSERT_EQ( optical_depths(2,1,2), 75.0 )
    ASSERT_EQ( optical_depths(2,2,2), 85.0 )
    ASSERT_EQ( optical_depths(2,3,2), 95.0 )

    call radiator%get_single_scattering_albedos( single_scattering_albedos(2,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( single_scattering_albedos(2,1,1), 900.0 )
    ASSERT_EQ( single_scattering_albedos(2,2,1), 800.0 )
    ASSERT_EQ( single_scattering_albedos(2,3,1), 700.0 )
    ASSERT_EQ( single_scattering_albedos(2,1,2), 750.0 )
    ASSERT_EQ( single_scattering_albedos(2,2,2), 850.0 )
    ASSERT_EQ( single_scattering_albedos(2,3,2), 950.0 )

    call radiator%get_asymmetry_factors( asymmetry_factors(2,:,:,:), error )
    ASSERT( error%is_success() )
    ASSERT_EQ( asymmetry_factors(2,1,1,1), 9.0 )
    ASSERT_EQ( asymmetry_factors(2,2,1,1), 8.0 )
    ASSERT_EQ( asymmetry_factors(2,3,1,1), 7.0 )
    ASSERT_EQ( asymmetry_factors(2,1,2,1), 5.0 )
    ASSERT_EQ( asymmetry_factors(2,2,2,1), 4.0 )
    ASSERT_EQ( asymmetry_factors(2,3,2,1), 3.0 )

    deallocate( grid )
    deallocate( grids )
    deallocate( profile )
    deallocate( profile_copy )
    deallocate( profiles )
    deallocate( radiator_copy )
    deallocate( radiator )
    deallocate( radiators )
    deallocate( height_grid )
    deallocate( wavelength_grid )
    deallocate( grids_from_host )
    deallocate( profiles_from_host )
    deallocate( radiators_from_host )
    deallocate( tuvx )

  end subroutine test_tuvx_solve

end program combined_tuvx_tests
