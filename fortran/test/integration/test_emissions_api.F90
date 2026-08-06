! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program test_emissions_api

  use, intrinsic :: iso_c_binding
  use iso_fortran_env, only: real64
  use musica_util, only: assert, error_t
  use musica_emissions, only: mechanism_t, emissions_t, &
    MUSICA_EMISSIONS_GRID_GEOMETRY_PLANAR

#include "musica/utils/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NEAR( a, b, tol ) call assert( abs(a - b) < abs(a + b) * tol, __FILE__, __LINE__ )

  implicit none

  write(*,*) "Testing miem Fortran API..."
  call test_api()

  write(*,*) "Testing selected-cell scalability API..."
  call test_selected_api()

  write(*,*) "Testing bad configuration path..."
  call test_bad_config_path()

contains

  subroutine test_api()
    integer, parameter :: NUM_CELLS = 4097  ! configs/miem/x1.163842_2024_nox_subset.nc
    real(real64), parameter :: EPOCH_2024_07_01 = 1719792000.0_real64

    type(mechanism_t), pointer :: mechanism
    type(emissions_t), pointer :: emissions
    type(error_t) :: error
    real(real64) :: flux_no, flux_no2
    integer :: cell

    mechanism => mechanism_t("configs/miem/nox_emissions_config.yaml", error)
    ASSERT( error%is_success() )

    emissions => emissions_t(mechanism, NUM_CELLS, 1, error)
    ASSERT( error%is_success() )

    ASSERT_EQ( emissions%number_of_species, 2 )
    ASSERT_EQ( emissions%species_ordering%size(), 2 )

    call emissions%run(EPOCH_2024_07_01, 3600.0_real64, error)
    ASSERT( error%is_success() )

    do cell = 1, NUM_CELLS
      flux_no  = emissions%flux(cell, "NO", error)
      ASSERT( error%is_success() )
      flux_no2 = emissions%flux(cell, "NO2", error)
      ASSERT( error%is_success() )
      ASSERT( flux_no >= 0.0_real64 )
      ASSERT( flux_no2 >= 0.0_real64 )
      if (flux_no2 > 0.0_real64) then
        ASSERT_NEAR( flux_no, 9.0_real64 * flux_no2, 1.0e-9_real64 )
      end if
    end do

    ! The pointer must be re-fetched after every run() -- confirm a second
    ! run() still produces a valid, readable flux (rather than asserting on
    ! Fortran pointer identity, which isn't directly comparable here).
    call emissions%run(EPOCH_2024_07_01 + 3600.0_real64, 3600.0_real64, error)
    ASSERT( error%is_success() )
    flux_no = emissions%flux(1, "NO", error)
    ASSERT( error%is_success() )

    deallocate(emissions)
    deallocate(mechanism)

    write(*,*) "[test emissions fort api] Finished."
  end subroutine test_api

  subroutine test_selected_api()
    integer, parameter :: GLOBAL_CELLS = 6
    integer, parameter :: LEVELS = 3
    real(real64), parameter :: EPOCH_MIDPOINT = 1800.0_real64
    integer, parameter :: selected_ids(3) = [ 6, 2, 3 ]
    integer, parameter :: categories(2) = [ 0, 1 ]
    character(len=6), parameter :: sectors(2) = [ "ground", "stack " ]

    type(mechanism_t), pointer :: mechanism
    type(emissions_t), pointer :: emissions
    type(emissions_t), pointer :: over_cap
    type(error_t) :: error
    real(real64) :: flux_no, layer_sum
    integer :: cell

    mechanism => mechanism_t(MIEM_SELECTED_CONFIG_PATH, error)
    ASSERT( error%is_success() )

    over_cap => emissions_t(mechanism, GLOBAL_CELLS, LEVELS, selected_ids, error, &
      diagnostic_sectors=sectors, diagnostic_categories=categories, &
      layered_diagnostics=.true., max_diagnostic_fields=23)
    ASSERT( .not. error%is_success() )
    ASSERT( .not. associated(over_cap) )

    emissions => emissions_t(mechanism, GLOBAL_CELLS, LEVELS, selected_ids, error, &
      diagnostic_sectors=sectors, diagnostic_categories=categories, &
      layered_diagnostics=.true., max_diagnostic_fields=24)
    ASSERT( error%is_success() )

    ASSERT_EQ( emissions%number_of_species, 2 )
    ASSERT_EQ( emissions%global_number_of_cells, GLOBAL_CELLS )
    ASSERT_EQ( emissions%number_of_cells, 3 )
    ASSERT_EQ( emissions%number_of_vertical_levels, LEVELS )
    ASSERT( associated(emissions%selected_global_cell_ids) )
    ASSERT_EQ( emissions%selected_global_cell_ids(1), 6 )
    ASSERT_EQ( emissions%selected_global_cell_ids(2), 2 )
    ASSERT_EQ( emissions%selected_global_cell_ids(3), 3 )
    ASSERT_EQ( emissions%sector_ordering%size(), 2 )
    ASSERT( emissions%sector_ordering%name(1) == "ground" )
    ASSERT( emissions%sector_ordering%name(2) == "stack" )
    ASSERT( associated(emissions%category_ids) )
    ASSERT_EQ( emissions%category_ids(1), 0 )
    ASSERT_EQ( emissions%category_ids(2), 1 )

    call emissions%run(EPOCH_MIDPOINT, 60.0_real64, error)
    ASSERT( error%is_success() )

    do cell = 1, emissions%number_of_cells
      flux_no = emissions%flux(cell, "NO", error)
      ASSERT( error%is_success() )
      ASSERT_NEAR( flux_no, 7.2e-9_real64, 1.0e-12_real64 )
      ASSERT_NEAR( emissions%flux(cell, "NO2", error), 0.8e-9_real64, 1.0e-12_real64 )
      ASSERT( error%is_success() )
      ASSERT_NEAR( emissions%layer_flux_at(cell, 1, "NO", error), 1.8e-9_real64, 1.0e-12_real64 )
      ASSERT( error%is_success() )
      ASSERT_NEAR( emissions%layer_flux_at(cell, 2, "NO", error), 1.35e-9_real64, 1.0e-12_real64 )
      ASSERT( error%is_success() )
      ASSERT_NEAR( emissions%layer_flux_at(cell, 3, "NO", error), 4.05e-9_real64, 1.0e-12_real64 )
      ASSERT( error%is_success() )
      layer_sum = emissions%layer_flux_at(cell, 1, "NO", error) + &
        emissions%layer_flux_at(cell, 2, "NO", error) + &
        emissions%layer_flux_at(cell, 3, "NO", error)
      ASSERT_NEAR( layer_sum, flux_no, 1.0e-12_real64 )
    end do

    ASSERT_EQ( size(emissions%sector_flux), 2 )
    ASSERT_EQ( size(emissions%category_flux), 2 )
    ASSERT_EQ( size(emissions%sector_layer_flux), 2 )
    ASSERT_EQ( size(emissions%category_layer_flux), 2 )
    ASSERT( associated(emissions%sector_flux(2)%values) )
    ASSERT_NEAR( emissions%sector_flux(2)%values(1), 5.4e-9_real64, 1.0e-12_real64 )
    ASSERT_NEAR( emissions%category_flux(2)%values(1), 5.4e-9_real64, 1.0e-12_real64 )
    ASSERT_EQ( emissions%category_layer_flux(2)%values(1), 0.0_real64 )
    ASSERT_NEAR( emissions%category_layer_flux(2)%values(4), 1.35e-9_real64, 1.0e-12_real64 )
    ASSERT_NEAR( emissions%category_layer_flux(2)%values(7), 4.05e-9_real64, 1.0e-12_real64 )

    ASSERT( emissions%grid_metadata%available )
    ASSERT( emissions%grid_metadata%exact_grid )
    ASSERT_EQ( emissions%grid_metadata%global_number_of_cells, GLOBAL_CELLS )
    ASSERT_EQ( emissions%grid_metadata%geometry, MUSICA_EMISSIONS_GRID_GEOMETRY_PLANAR )
    ASSERT( emissions%grid_metadata%on_a_sphere == "NO" )
    ASSERT( emissions%grid_metadata%fingerprint_algorithm == "chempas-mesh-sha256-v1" )
    ASSERT_EQ( len(emissions%grid_metadata%fingerprint), 64 )
    ASSERT( associated(emissions%grid_metadata%index_to_cell_id) )
    ASSERT_EQ( emissions%grid_metadata%index_to_cell_id(1), 6_c_int64_t )
    ASSERT_EQ( emissions%grid_metadata%index_to_cell_id(2), 2_c_int64_t )
    ASSERT_EQ( emissions%grid_metadata%index_to_cell_id(3), 3_c_int64_t )
    ASSERT( associated(emissions%grid_metadata%area_cell) )
    ASSERT_EQ( emissions%grid_metadata%area_cell(1), 1005.0_real64 )
    ASSERT_EQ( emissions%grid_metadata%area_cell(2), 1001.0_real64 )
    ASSERT_EQ( emissions%grid_metadata%area_cell(3), 1002.0_real64 )
    ASSERT( emissions%grid_metadata%area_cell_units == "m2" )
    ASSERT( associated(emissions%grid_metadata%x_cell) )
    ASSERT( associated(emissions%grid_metadata%y_cell) )
    ASSERT( associated(emissions%grid_metadata%z_cell) )
    ASSERT( .not. associated(emissions%grid_metadata%lat_cell) )
    ASSERT( .not. associated(emissions%grid_metadata%lon_cell) )

    call emissions%run(EPOCH_MIDPOINT + 900.0_real64, 60.0_real64, error)
    ASSERT( error%is_success() )
    ASSERT( associated(emissions%layer_flux) )
    ASSERT( associated(emissions%grid_metadata%area_cell) )

    deallocate(emissions)
    deallocate(mechanism)

    write(*,*) "[test selected emissions fort api] Finished."
  end subroutine test_selected_api

  subroutine test_bad_config_path()
    type(mechanism_t), pointer :: mechanism
    type(error_t) :: error

    mechanism => mechanism_t("bad config path", error)
    ASSERT( error%is_error( MUSICA_PARSE_ERROR_CATEGORY, MUSICA_PARSE_ERROR_CODE_INVALID_CONFIG_FILE ) )
    ASSERT( .not. associated( mechanism ) )
  end subroutine test_bad_config_path

end program test_emissions_api
