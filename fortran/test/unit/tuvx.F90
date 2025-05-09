! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_tuvx_connection
  use musica_util, only : dk => musica_dk

  implicit none

  integer, parameter :: number_of_layers = 3
  integer, parameter :: number_of_wavelengths = 5
  integer, parameter :: number_of_reactions = 3
  integer, parameter :: number_of_heating_rates = 2

  real(dk), parameter :: expected_photolysis_rate_constants(4,3) = reshape( [ &
    8.91393763338872e-28_dk, 1.64258192104497e-20_dk, 8.48391527327371e-14_dk, 9.87420948924703e-08_dk, &
    2.49575956372508e-27_dk, 4.58686176250519e-20_dk, 2.22679622672858e-13_dk, 2.29392676897831e-07_dk, &
    1.78278752667774e-27_dk, 3.28516384208994e-20_dk, 1.69678305465474e-13_dk, 1.97484189784941e-07_dk ], [4,3] )
  real(dk), parameter :: expected_heating_rates(4,2) = reshape( [ &
    1.12394047546984e-46_dk, 2.04518267143613e-39_dk, 7.44349752571804e-33_dk, 5.42628100199216e-28_dk, &
    5.14970120496081e-46_dk, 9.37067648164478e-39_dk, 3.41659389501112e-32_dk, 5.46672356294259e-27_dk ], [4,2] )

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )
#define ASSERT_NEAR( a, b, tol ) call assert( abs(a - b) < abs(a + b) * tol, __FILE__, __LINE__ )

  call test_tuvx_fixed()
  call test_tuvx_data_from_host()

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Test TUV-x with a fixed configuration
  subroutine test_tuvx_fixed( )
    
    use musica_util,              only : assert, error_t, mappings_t
    use musica_tuvx_grid,         only : grid_t
    use musica_tuvx_grid_map,     only : grid_map_t
    use musica_tuvx_profile,      only : profile_t
    use musica_tuvx_profile_map,  only : profile_map_t
    use musica_tuvx_radiator_map, only : radiator_map_t
    use musica_tuvx,              only : tuvx_t

    character(len=*), parameter :: my_name = "TUV-x fixed configuration tests"
    character(len=*), parameter :: config_path = "test/data/tuvx/fixed/config.json"
    type(grid_map_t),     pointer :: grids
    type(profile_map_t),  pointer :: profiles
    type(radiator_map_t), pointer :: radiators
    type(tuvx_t),         pointer :: tuvx
    type(mappings_t),     pointer :: photo_mappings, heating_mappings
    type(error_t) :: error

    real(dk) :: photo_rate_consts(number_of_layers + 1, number_of_reactions)
    real(dk) :: heating_rates(number_of_layers + 1, number_of_heating_rates)
    integer :: i, j
    real(dk) :: a, b

    grids => grid_map_t( error )
    ASSERT( error%is_success() )
    profiles => profile_map_t( error )
    ASSERT( error%is_success() )
    radiators => radiator_map_t( error )
    ASSERT( error%is_success() )
    tuvx => tuvx_t( config_path, grids, profiles, radiators, error )
    ASSERT( error%is_success() )
    call tuvx%run( 0.1_dk, 1.1_dk, photo_rate_consts, heating_rates, error )
    ASSERT( error%is_success() )
    do i = 1, number_of_reactions
      do j = 1, number_of_layers + 1
        a = photo_rate_consts(j,i)
        b = expected_photolysis_rate_constants(j,i)
        ASSERT_NEAR( a, b, 1.0e-5_dk )
      end do
    end do
    do i = 1, number_of_heating_rates
      do j = 1, number_of_layers + 1
        a = heating_rates(j,i)
        b = expected_heating_rates(j,i)
        ASSERT_NEAR( a, b, 1.0e-5_dk )
      end do
    end do
    photo_mappings => tuvx%get_photolysis_rate_constants_ordering( error )
    ASSERT( error%is_success() )
    ASSERT_EQ( photo_mappings%size(), number_of_reactions )
    ASSERT_EQ( photo_mappings%name( 1 ), "jfoo" )
    ASSERT_EQ( photo_mappings%index( 1 ), 1 )
    ASSERT_EQ( photo_mappings%index( "jfoo", error ), 1 )
    ASSERT( error%is_success() )
    ASSERT_EQ( photo_mappings%name( 2 ), "jbar" )
    ASSERT_EQ( photo_mappings%index( 2 ), 2 )
    ASSERT_EQ( photo_mappings%index( "jbar", error ), 2 )
    ASSERT( error%is_success() )
    ASSERT_EQ( photo_mappings%name( 3 ), "jbaz" )
    ASSERT_EQ( photo_mappings%index( 3 ), 3 )
    ASSERT_EQ( photo_mappings%index( "jbaz", error ), 3 )
    ASSERT( error%is_success() )
    heating_mappings => tuvx%get_heating_rates_ordering( error )
    ASSERT( error%is_success() )
    ASSERT_EQ( heating_mappings%size(), number_of_heating_rates )
    ASSERT_EQ( heating_mappings%name( 1 ), "jfoo" )
    ASSERT_EQ( heating_mappings%index( 1 ), 1 )
    ASSERT_EQ( heating_mappings%index( "jfoo", error ), 1 )
    ASSERT( error%is_success() )
    ASSERT_EQ( heating_mappings%name( 2 ), "jbar" )
    ASSERT_EQ( heating_mappings%index( 2 ), 2 )
    ASSERT_EQ( heating_mappings%index( "jbar", error ), 2 )
    ASSERT( error%is_success() )
    deallocate( photo_mappings )
    deallocate( heating_mappings )
    deallocate( tuvx )
    deallocate( radiators )
    deallocate( profiles )
    deallocate( grids )

  end subroutine test_tuvx_fixed

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Test TUV-x with host-supplied grids and profiles
  subroutine test_tuvx_data_from_host( )

    use musica_util,              only : assert, error_t, mappings_t
    use musica_tuvx_grid,         only : grid_t
    use musica_tuvx_grid_map,     only : grid_map_t
    use musica_tuvx_profile,      only : profile_t
    use musica_tuvx_profile_map,  only : profile_map_t
    use musica_tuvx_radiator_map, only : radiator_map_t
    use musica_tuvx,              only : tuvx_t

    character(len=*), parameter :: my_name = "TUV-x from-host configuration tests"
    character(len=*), parameter :: config_path = "test/data/tuvx/from_host/config.json"
    type(grid_t),         pointer :: heights, wavelengths
    type(grid_map_t),     pointer :: grids
    type(profile_t),      pointer :: temperatures
    type(profile_map_t),  pointer :: profiles
    type(radiator_map_t), pointer :: radiators
    type(tuvx_t),         pointer :: tuvx
    type(mappings_t),     pointer :: photo_mappings, heating_mappings
    type(error_t) :: error

    real(dk) :: photo_rate_consts(number_of_layers + 1, number_of_reactions)
    real(dk) :: heating_rates(number_of_layers + 1, number_of_heating_rates)
    integer :: i, j
    real(dk) :: a, b
    real(dk), allocatable :: temp_vals(:)

    heights => grid_t( "height", "km", 3, error )
    ASSERT( error%is_success() )
    call heights%set_edges( [ 0.0_dk, 1.2_dk, 2.0_dk, 3.0_dk ], error )
    ASSERT( error%is_success() )
    call heights%set_midpoints( [ 0.5_dk, 1.3_dk, 2.5_dk ], error )
    wavelengths => grid_t( "wavelength", "nm", 5, error )
    ASSERT( error%is_success() )
    call wavelengths%set_edges( [ 300.0_dk, 400.0_dk, 500.0_dk, 600.0_dk, 700.0_dk, 800.0_dk ], error )
    ASSERT( error%is_success() )
    call wavelengths%set_midpoints( [ 350.0_dk, 450.0_dk, 550.0_dk, 650.0_dk, 750.0_dk ], error )
    ASSERT( error%is_success() )
    grids => grid_map_t( error )
    ASSERT( error%is_success() )
    call grids%add( heights, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( heights%number_of_sections( error ), 3 )
    ASSERT( error%is_success() )
    allocate( temp_vals(4) )
    call heights%get_edges( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 0.0_dk )
    ASSERT_EQ( temp_vals(2), 1.2_dk )
    ASSERT_EQ( temp_vals(3), 2.0_dk )
    ASSERT_EQ( temp_vals(4), 3.0_dk )
    deallocate( temp_vals )
    allocate( temp_vals(3) )
    call heights%get_midpoints( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 0.5_dk )
    ASSERT_EQ( temp_vals(2), 1.3_dk )
    ASSERT_EQ( temp_vals(3), 2.5_dk )
    deallocate( temp_vals )
    call heights%set_edges( [ 0.0_dk, 1.0_dk, 2.0_dk, 3.0_dk ], error )
    ASSERT( error%is_success() )
    call heights%set_midpoints( [ 0.5_dk, 1.5_dk, 2.5_dk ], error )
    ASSERT( error%is_success() )
    call grids%add( wavelengths, error )
    ASSERT( error%is_success() )
    temperatures => profile_t( "temperature", "K", heights, error )
    ASSERT( error%is_success() )
    profiles => profile_map_t( error )
    ASSERT( error%is_success() )
    call profiles%add( temperatures, error )
    ASSERT( error%is_success() )
    radiators => radiator_map_t( error )
    ASSERT( error%is_success() )
    tuvx => tuvx_t( config_path, grids, profiles, radiators, error )
    ASSERT( error%is_success() )
    deallocate( heights )
    deallocate( wavelengths )
    deallocate( temperatures )
    deallocate( grids )
    deallocate( profiles )
    deallocate( radiators )
    grids => tuvx%get_grids( error )
    ASSERT( error%is_success() )
    profiles => tuvx%get_profiles( error )
    ASSERT( error%is_success() )
    radiators => tuvx%get_radiators( error )
    ASSERT( error%is_success() )
    heights => grids%get( "height", "km", error )
    ASSERT( error%is_success() )
    wavelengths => grids%get( "wavelength", "nm", error )
    ASSERT( error%is_success() )
    temperatures => profiles%get( "temperature", "K", error )
    ASSERT( error%is_success() )
    call temperatures%set_edge_values( [ 300.0_dk, 275.0_dk, 260.0_dk, 255.0_dk ], error )
    ASSERT( error%is_success() )
    call temperatures%set_midpoint_values( [ 287.5_dk, 267.5_dk, 257.5_dk ], error )
    ASSERT( error%is_success() )
    call tuvx%run( 0.1_dk, 1.1_dk, photo_rate_consts, heating_rates, error )
    ASSERT( error%is_success() )
    do i = 1, number_of_reactions
      do j = 1, number_of_layers + 1
        a = photo_rate_consts(j,i)
        b = expected_photolysis_rate_constants(j,i)
        ASSERT_NEAR( a, b, 1.0e-5_dk )
      end do
    end do
    do i = 1, number_of_heating_rates
      do j = 1, number_of_layers + 1
        a = heating_rates(j,i)
        b = expected_heating_rates(j,i)
        ASSERT_NEAR( a, b, 1.0e-5_dk )
      end do
    end do
    allocate( temp_vals(4) )
    ASSERT_EQ( heights%number_of_sections( error ), 3 )
    call heights%get_edges( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 0.0_dk )
    ASSERT_EQ( temp_vals(2), 1.0_dk )
    ASSERT_EQ( temp_vals(3), 2.0_dk )
    ASSERT_EQ( temp_vals(4), 3.0_dk )
    deallocate( temp_vals )
    allocate( temp_vals(3) )
    call heights%get_midpoints( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 0.5_dk )
    ASSERT_EQ( temp_vals(2), 1.5_dk )
    ASSERT_EQ( temp_vals(3), 2.5_dk )
    deallocate( temp_vals )
    allocate( temp_vals(6) )
    call wavelengths%get_edges( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 300.0_dk )
    ASSERT_EQ( temp_vals(2), 400.0_dk )
    ASSERT_EQ( temp_vals(3), 500.0_dk )
    ASSERT_EQ( temp_vals(4), 600.0_dk )
    ASSERT_EQ( temp_vals(5), 700.0_dk )
    ASSERT_EQ( temp_vals(6), 800.0_dk )
    deallocate( temp_vals )
    allocate( temp_vals(5) )
    call wavelengths%get_midpoints( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 350.0_dk )
    ASSERT_EQ( temp_vals(2), 450.0_dk )
    ASSERT_EQ( temp_vals(3), 550.0_dk )
    ASSERT_EQ( temp_vals(4), 650.0_dk )
    ASSERT_EQ( temp_vals(5), 750.0_dk )
    deallocate( temp_vals )
    allocate( temp_vals(4) )
    call temperatures%get_edge_values( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 300.0_dk )
    ASSERT_EQ( temp_vals(2), 275.0_dk )
    ASSERT_EQ( temp_vals(3), 260.0_dk )
    ASSERT_EQ( temp_vals(4), 255.0_dk )
    deallocate( temp_vals )
    allocate( temp_vals(3) )
    call temperatures%get_midpoint_values( temp_vals, error )
    ASSERT( error%is_success() )
    ASSERT_EQ( temp_vals(1), 287.5_dk )
    ASSERT_EQ( temp_vals(2), 267.5_dk )
    ASSERT_EQ( temp_vals(3), 257.5_dk )
    photo_mappings => tuvx%get_photolysis_rate_constants_ordering( error )
    ASSERT( error%is_success() )
    ASSERT_EQ( photo_mappings%size(), number_of_reactions )
    ASSERT_EQ( photo_mappings%name( 1 ), "jfoo" )
    ASSERT_EQ( photo_mappings%index( 1 ), 1 )
    ASSERT_EQ( photo_mappings%index( "jfoo", error ), 1 )
    ASSERT( error%is_success() )
    ASSERT_EQ( photo_mappings%name( 2 ), "jbar" )
    ASSERT_EQ( photo_mappings%index( 2 ), 2 )
    ASSERT_EQ( photo_mappings%index( "jbar", error ), 2 )
    ASSERT( error%is_success() )
    ASSERT_EQ( photo_mappings%name( 3 ), "jbaz" )
    ASSERT_EQ( photo_mappings%index( 3 ), 3 )
    ASSERT_EQ( photo_mappings%index( "jbaz", error ), 3 )
    ASSERT( error%is_success() )
    heating_mappings => tuvx%get_heating_rates_ordering( error )
    ASSERT( error%is_success() )
    ASSERT_EQ( heating_mappings%size(), number_of_heating_rates )
    ASSERT_EQ( heating_mappings%name( 1 ), "jfoo" )
    ASSERT_EQ( heating_mappings%index( 1 ), 1 )
    ASSERT_EQ( heating_mappings%index( "jfoo", error ), 1 )
    ASSERT( error%is_success() )
    ASSERT_EQ( heating_mappings%name( 2 ), "jbar" )
    ASSERT_EQ( heating_mappings%index( 2 ), 2 )
    ASSERT_EQ( heating_mappings%index( "jbar", error ), 2 )
    ASSERT( error%is_success() )
    deallocate( photo_mappings )
    deallocate( heating_mappings )
    deallocate( temp_vals )
    deallocate( tuvx )
    deallocate( heights )
    deallocate( wavelengths )
    deallocate( temperatures )
    deallocate( radiators )
    deallocate( profiles )
    deallocate( grids ) 
    
  end subroutine test_tuvx_data_from_host

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end program test_tuvx_connection
