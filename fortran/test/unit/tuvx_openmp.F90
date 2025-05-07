! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_tuvx_connection

#ifdef MUSICA_USE_OPENMP
    use omp_lib
#endif

  implicit none

#ifdef MUSICA_USE_OPENMP
    write(*,*) "Testing with ", omp_get_max_threads( ), " threads"
#else
    write(*,*) "Testing without OpenMP support"
#endif


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
