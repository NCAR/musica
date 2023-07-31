! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_musica_connection
  use musica_assert
  use musica_mpi

  implicit none

  call musica_mpi_init( )
  call test_musicacore( )
  call musica_mpi_finalize( )

contains

  subroutine test_musicacore()
    integer, parameter :: comm = MPI_COMM_WORLD
    integer :: send_integer, recv_integer

    call assert( 357761664, musica_mpi_support( ) )
    call assert( 455191678, musica_mpi_size( comm ) > 1 )

    call musica_mpi_barrier( comm )

    send_integer = 0
    if( musica_mpi_rank( comm ) == 0 ) send_integer = 42
    call musica_mpi_bcast( send_integer, comm )
    call assert( 353714667, send_integer == 42 )
  end subroutine test_musicacore

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end program test_musica_connection
