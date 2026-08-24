! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
program test_emissions_api

  use, intrinsic :: iso_c_binding
  use iso_fortran_env, only: real64
  use musica_util, only: assert, error_t
  use musica_emissions, only: mechanism_t, emissions_t

#include "musica/utils/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NEAR( a, b, tol ) call assert( abs(a - b) < abs(a + b) * tol, __FILE__, __LINE__ )

  implicit none

  write(*,*) "Testing miem Fortran API..."
  call test_api()

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

  subroutine test_bad_config_path()
    type(mechanism_t), pointer :: mechanism
    type(error_t) :: error

    mechanism => mechanism_t("bad config path", error)
    ASSERT( error%is_error( MUSICA_PARSE_ERROR_CATEGORY, MUSICA_PARSE_ERROR_CODE_INVALID_CONFIG_FILE ) )
    ASSERT( .not. associated( mechanism ) )
  end subroutine test_bad_config_path

end program test_emissions_api
