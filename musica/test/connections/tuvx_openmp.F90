! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_tuvx_connection
  use musica_assert
  use tuvx_grid, only : grid_t
  use tuvx_grid_equal_delta, only : grid_equal_delta_t
  use musica_config,         only : config_t

#ifdef MUSICA_USE_OPENMP
    use omp_lib
#endif

  implicit none

  type(config_t) :: config
  class(grid_t), pointer :: new_grid_t => null()

#ifdef MUSICA_USE_OPENMP
    write(*,*) "Testing with ", omp_get_max_threads( ), " threads"
#else
    write(*,*) "Testing without OpenMP support"
#endif


  config = '{'//                                                            &
            '   "name": "eq_int",' //                                        &
            '   "type": "equal interval",' //                                &
            '   "units": "km",' //                                           &
            '   "begins at": 0.0,' //                                        &
            '   "ends at": 120.0,' //                                        &
            '   "cell delta": 1.0' //                                        &
            '}'

  new_grid_t => grid_equal_delta_t( config )

  !$omp parallel
  call test_tuvx( new_grid_t )
  !$omp end parallel

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Test tuvx connection
  subroutine test_tuvx(a_grid_t)
    use tuvx_grid, only : grid_t

    class(grid_t), pointer :: a_grid_t

    call assert(412348394, a_grid_t%ncells_ == 120)

  end subroutine test_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end program test_tuvx_connection
