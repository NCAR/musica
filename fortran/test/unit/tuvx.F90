! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_tuvx_connection
  use musica_assert
  use musica_constants, only : dk => musica_dk

  implicit none

  integer, parameter :: number_of_layers = 3
  integer, parameter :: number_of_wavelengths = 5
  integer, parameter :: number_of_reactions = 2

  real(dk), parameter :: expected_photolysis_rate_constants(4,2) = reshape( [ &
    8.91393763338872e-28_dk, 1.64258192104497e-20_dk, 8.48391527327371e-14_dk, 9.87420948924703e-08_dk, &
    2.49575956372508e-27_dk, 4.58686176250519e-20_dk, 2.22679622672858e-13_dk, 2.29392676897831e-07_dk ], [4,2] )
  real(dk), parameter :: expected_heating_rates(4,2) = reshape( [ &
    1.12394047546984e-46_dk, 2.04518267143613e-39_dk, 7.44349752571804e-33_dk, 5.42628100199216e-28_dk, &
    5.14970120496081e-46_dk, 9.37067648164478e-39_dk, 3.41659389501112e-32_dk, 5.46672356294259e-27_dk ], [4,2] )

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )
#define ASSERT_NEAR( a, b, tol ) call assert( abs(a - b) < abs(a + b) * tol, __FILE__, __LINE__ )

  call test_tuvx()
  call test_tuvx_fixed()

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Test tuvx connection
  subroutine test_tuvx( )
    use tuvx_grid,             only : grid_t
    use tuvx_grid_equal_delta, only : grid_equal_delta_t
    use musica_config,         only : config_t

    character(len=*), parameter :: my_name = "tuvx connection tests"
    type(config_t) :: config
    class(grid_t), pointer :: new_grid_t => null()

    config = '{'//                                                            &
             '   "name": "eq_int",' //                                        &
             '   "type": "equal interval",' //                                &
             '   "units": "km",' //                                           &
             '   "begins at": 0.0,' //                                        &
             '   "ends at": 120.0,' //                                        &
             '   "cell delta": 1.0' //                                        &
             '}'

    new_grid_t => grid_equal_delta_t( config )
    call assert(412348394, new_grid_t%ncells_ == 120)
    deallocate(new_grid_t)

  end subroutine test_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Test TUV-x with a fixed configuration
  subroutine test_tuvx_fixed( )
    
    use musica_util,              only : assert, error_t
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
    type(error_t) :: error

    real(dk) :: photo_rate_consts(number_of_layers + 1, number_of_reactions)
    real(dk) :: heating_rates(number_of_layers + 1, number_of_reactions)
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
        a = heating_rates(j,i)
        b = expected_heating_rates(j,i)
        ASSERT_NEAR( a, b, 1.0e-5_dk )
      end do
    end do
    deallocate( tuvx )
    deallocate( radiators )
    deallocate( profiles )
    deallocate( grids )

  end subroutine test_tuvx_fixed

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end program test_tuvx_connection
