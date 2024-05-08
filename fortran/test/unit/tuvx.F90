! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_tuvx_connection
  use musica_assert

  implicit none

  call test_tuvx()

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

end program test_tuvx_connection
