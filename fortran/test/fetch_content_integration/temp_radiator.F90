! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program combined_tuvx_tests2
    use iso_c_binding
    use musica_tuvx, only: tuvx_t, grid_map_t, grid_t, profile_map_t, profile_t, &
                           radiator_map_t, radiator_t
    use musica_util, only: assert, error_t, dk => musica_dk

    implicit none
  
#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
  
  ! Call the valid test subroutine
  call test_jiwon_solve()
  
  contains
    subroutine test_jiwon_solve()

      type(tuvx_t),         pointer :: tuvx
      type(error_t)                 :: error
      type(grid_map_t),     pointer :: grids
      character(len=256)            :: config_path
      type(grid_t),         pointer :: grid, height_grid, wavelength_grid
      type(profile_map_t),  pointer :: profiles
      type(profile_t),      pointer :: profile, profile_copy
      type(radiator_map_t), pointer :: radiators
      type(radiator_t),     pointer :: radiator, radiator_copy
      real*8, dimension(5), target  :: edges, edge_values, temp_edge
      real*8, dimension(4), target  :: midpoints, midpoint_values, layer_densities, temp_midpoint
      real*8                        :: temp_real
      integer                       :: num_vertical_layers, num_wavelength_bins
      real*8, dimension(3,2), target   :: optical_depths, temp_od
      real*8, dimension(3,2), target   :: single_scattering_albedos, temp_ssa
      real*8, dimension(3,2,1), target :: asymmetry_factors, temp_asym

      edges = (/ 1.0, 2.0, 3.0, 4.0, 5.0 /)
      midpoints = (/ 15.0, 25.0, 35.0, 45.0 /)  
      edge_values = (/ 10.0, 20.0, 30.0, 40.0, 50.0 /)
      midpoint_values = (/ 15.0, 25.0, 35.0, 45.0 /)
      layer_densities = (/ 2.0, 4.0, 1.0, 7.0 /)

      num_vertical_layers = 3
      num_wavelength_bins = 2

      optical_depths(:,1) = (/ 30.0, 20.0, 10.0 /)
      optical_depths(:,2) = (/ 70.0, 80.0, 90.0 /)
      single_scattering_albedos(:,1) = (/ 300.0, 200.0, 100.0 /)
      single_scattering_albedos(:,2) = (/ 700.0, 800.0, 900.0 /)
      asymmetry_factors(:,1,1) = (/ 3.0, 2.0, 1.0 /)
      asymmetry_factors(:,2,1) = (/ 7.0, 8.0, 9.0 /)

      config_path = "examples/ts1_tsmlt.json"

      tuvx => tuvx_t( config_path, error )
      ASSERT( error%is_success() )

!       !
!       ! TODO - Radiator starts here?
!       !     
      radiators => tuvx%get_radiators( error )
      ASSERT( error%is_success() )

      radiator => radiators%get( "foo_radiator", error )
      ASSERT( .not. error%is_success() ) ! non-accessible grid
      deallocate( radiator )
      deallocate( radiators )
 
      radiators =>radiator_map_t( error )
      ASSERT( error%is_success() )

      height_grid => grid_t( "height", "km", num_vertical_layers, error )
      wavelength_grid => grid_t( "wavelength", "nm", num_wavelength_bins, error )
      radiator => radiator_t( "foo_radiator", height_grid, wavelength_grid, error )
      ASSERT( error%is_success() )

      call radiator%set_optical_depths( optical_depths, error )
      ASSERT( error%is_success() )

      call radiator%get_optical_depths( temp_od, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( temp_od(1,1), 30.0 )
      ASSERT_EQ( temp_od(2,1), 20.0 )
      ASSERT_EQ( temp_od(3,1), 10.0 )
      ASSERT_EQ( temp_od(1,2), 70.0 )
      ASSERT_EQ( temp_od(2,2), 80.0 )
      ASSERT_EQ( temp_od(3,2), 90.0 )

      call radiator%set_single_scattering_albedos( single_scattering_albedos, error )
      ASSERT( error%is_success() )

      call radiator%get_single_scattering_albedos( temp_ssa, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( temp_ssa(1,1), 300.0 )
      ASSERT_EQ( temp_ssa(2,1), 200.0 )
      ASSERT_EQ( temp_ssa(3,1), 100.0 )
      ASSERT_EQ( temp_ssa(1,2), 700.0 )
      ASSERT_EQ( temp_ssa(2,2), 800.0 )
      ASSERT_EQ( temp_ssa(3,2), 900.0 )

      call radiator%set_asymmetry_factors( asymmetry_factors, error )
      ASSERT( error%is_success() )

      call radiator%get_asymmetry_factors( temp_asym, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( temp_asym(1,1,1), 3.0 )
      ASSERT_EQ( temp_asym(2,1,1), 2.0 )
      ASSERT_EQ( temp_asym(3,1,1), 1.0 )
      ASSERT_EQ( temp_asym(1,2,1), 7.0 )
      ASSERT_EQ( temp_asym(2,2,1), 8.0 )
      ASSERT_EQ( temp_asym(3,2,1), 9.0 )
!
      call radiators%add( radiator, error )
      radiator_copy => radiators%get( "foo_radiator", error )

      ! call radiator_copy

      optical_depths(:,:) = 0.0
      single_scattering_albedos(:,:) = 0.0
      asymmetry_factors(:,:,:) = 0.0

      call radiator_copy%get_optical_depths( optical_depths, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( optical_depths(1,1), 30.0 )
      ASSERT_EQ( optical_depths(2,1), 20.0 )
      ASSERT_EQ( optical_depths(3,1), 10.0 )
      ASSERT_EQ( optical_depths(1,2), 70.0 )
      ASSERT_EQ( optical_depths(2,2), 80.0 )
      ASSERT_EQ( optical_depths(3,2), 90.0 )

      call radiator_copy%get_single_scattering_albedos( single_scattering_albedos, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( single_scattering_albedos(1,1), 300.0 )
      ASSERT_EQ( single_scattering_albedos(2,1), 200.0 )
      ASSERT_EQ( single_scattering_albedos(3,1), 100.0 )
      ASSERT_EQ( single_scattering_albedos(1,2), 700.0 )
      ASSERT_EQ( single_scattering_albedos(2,2), 800.0 )
      ASSERT_EQ( single_scattering_albedos(3,2), 900.0 )

      call radiator_copy%get_asymmetry_factors( asymmetry_factors, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( asymmetry_factors(1,1,1), 3.0 )
      ASSERT_EQ( asymmetry_factors(2,1,1), 2.0 )
      ASSERT_EQ( asymmetry_factors(3,1,1), 1.0 )
      ASSERT_EQ( asymmetry_factors(1,2,1), 7.0 )
      ASSERT_EQ( asymmetry_factors(2,2,1), 8.0 )
      ASSERT_EQ( asymmetry_factors(3,2,1), 9.0 )

      optical_depths(:,1) = (/ 90.0, 80.0, 70.0 /)
      optical_depths(:,2) = (/ 75.0, 85.0, 95.0 /)
      single_scattering_albedos(:,1) = (/ 900.0, 800.0, 700.0 /)
      single_scattering_albedos(:,2) = (/ 750.0, 850.0, 950.0 /)
      asymmetry_factors(:,1,1) = (/ 9.0, 8.0, 7.0 /)
      asymmetry_factors(:,2,1) = (/ 5.0, 4.0, 3.0 /)

      call radiator_copy%set_optical_depths( optical_depths, error )
      call radiator_copy%set_single_scattering_albedos( single_scattering_albedos, error )
      call radiator_copy%set_asymmetry_factors( asymmetry_factors, error )
      ASSERT( error%is_success() )

      optical_depths(:,:) = 0.0
      single_scattering_albedos(:,:) = 0.0
      asymmetry_factors(:,:,:) = 0.0

      call radiator%get_optical_depths( optical_depths, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( optical_depths(1,1), 90.0 )
      ASSERT_EQ( optical_depths(2,1), 80.0 )
      ASSERT_EQ( optical_depths(3,1), 70.0 )
      ASSERT_EQ( optical_depths(1,2), 75.0 )
      ASSERT_EQ( optical_depths(2,2), 85.0 )
      ASSERT_EQ( optical_depths(3,2), 95.0 )

      call radiator%get_single_scattering_albedos( single_scattering_albedos, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( single_scattering_albedos(1,1), 900.0 )
      ASSERT_EQ( single_scattering_albedos(2,1), 800.0 )
      ASSERT_EQ( single_scattering_albedos(3,1), 700.0 )
      ASSERT_EQ( single_scattering_albedos(1,2), 750.0 )
      ASSERT_EQ( single_scattering_albedos(2,2), 850.0 )
      ASSERT_EQ( single_scattering_albedos(3,2), 950.0 )

      call radiator%get_asymmetry_factors( asymmetry_factors, error )
      ASSERT( error%is_success() )
      ASSERT_EQ( asymmetry_factors(1,1,1), 9.0 )
      ASSERT_EQ( asymmetry_factors(2,1,1), 8.0 )
      ASSERT_EQ( asymmetry_factors(3,1,1), 7.0 )
      ASSERT_EQ( asymmetry_factors(1,2,1), 5.0 )
      ASSERT_EQ( asymmetry_factors(2,2,1), 4.0 )
      ASSERT_EQ( asymmetry_factors(3,2,1), 3.0 )

      deallocate( radiator_copy )
      deallocate( radiator )
      deallocate( radiators )
      deallocate( height_grid )
      deallocate( wavelength_grid )
      deallocate( tuvx )
  end subroutine test_jiwon_solve

end program combined_tuvx_tests2