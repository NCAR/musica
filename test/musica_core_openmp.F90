! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_musica_connection
  use musica_assert
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
  call test_musicacore( )
  !$omp end parallel

contains

  subroutine test_musicacore()
    call assert(1234312, 1 == 1)
  end subroutine test_musicacore

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end program test_musica_connection
