! Copyright (C) 2023-2024 National Center for Atmospheric Research
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
    subroutine test_jiwon_solve()

      type(tuvx_t),         pointer :: tuvx
      type(error_t)                 :: error
      type(grid_map_t),     pointer :: grids
      character(len=256)            :: config_path
      type(grid_t),         pointer :: grid
      type(profile_map_t),  pointer :: profiles
      type(profile_t),      pointer :: profile, profile_copy
      type(radiator_map_t), pointer :: radiators
      type(radiator_t),     pointer :: radiator, radiator_copy
      real*8, dimension(5), target  :: edges, edge_values, temp_edge
      real*8, dimension(4), target  :: midpoints, midpoint_values, layer_densities, temp_midpoint
      real*8                        :: temp_real

      edges = (/ 1.0, 2.0, 3.0, 4.0, 5.0 /)
      midpoints = (/ 15.0, 25.0, 35.0, 45.0 /)  
      edge_values = (/ 10.0, 20.0, 30.0, 40.0, 50.0 /)
      midpoint_values = (/ 15.0, 25.0, 35.0, 45.0 /)
      layer_densities = (/ 2.0, 4.0, 1.0, 7.0 /)

      config_path = "examples/ts1_tsmlt.json"

      tuvx => tuvx_t( config_path, error )
      ASSERT( error%is_success() )
      radiators => tuvx%get_radiators( error )
      ASSERT( error%is_success() )

      ! grid => grids%get( "height", "km", error )
      ! ASSERT( .not. error%is_success() ) ! non-accessible grid
      ! deallocate( grid )
      ! deallocate( grids )
 
  end subroutine test_tuvx_solve

end program combined_tuvx_tests