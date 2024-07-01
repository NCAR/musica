! Copyright (C) 2022 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
!> \file
!> Tests for that we can connect to tuvx using the monolith static library for musica

!> Test module for the tuvx connection
program test_tuvx_connection
  use musica_assert
  use musica_mpi

  implicit none

  call musica_mpi_init( )
  call test_tuvx( )
  call musica_mpi_finalize( )

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Test tuvx connection
  subroutine test_tuvx( )
    use tuvx_grid,             only : grid_t
    use tuvx_grid_equal_delta, only : grid_equal_delta_t
    use tuvx_grid_factory,     only : grid_type_name, grid_allocate
    use musica_config,         only : config_t
    use musica_string,         only : string_t

    character(len=*), parameter :: my_name = "tuvx connection tests"
    type(config_t) :: config
    class(grid_t), pointer :: new_grid_t => null()
    type(string_t) :: type_name
    character, allocatable :: buffer(:)
    integer :: pos, pack_size
    integer, parameter :: comm = MPI_COMM_WORLD

    config = '{'//                                                            &
             '   "name": "eq_int",' //                                        &
             '   "type": "equal interval",' //                                &
             '   "units": "km",' //                                           &
             '   "begins at": 0.0,' //                                        &
             '   "ends at": 120.0,' //                                        &
             '   "cell delta": 1.0' //                                        &
             '}'


    if( musica_mpi_rank( comm ) == 0 ) then
      new_grid_t => grid_equal_delta_t( config )
      call assert(412348394, new_grid_t%ncells_ == 120)

      type_name = grid_type_name( new_grid_t )
      pack_size = type_name%pack_size( comm ) + new_grid_t%pack_size( comm )

      allocate( buffer( pack_size ) )
      pos = 0

      call type_name%mpi_pack(  buffer, pos , comm )
      call new_grid_t%mpi_pack( buffer, pos , comm )

      call assert( 319321152, pos <= pack_size )
    end if

    call musica_mpi_bcast( pack_size , comm )
    if( musica_mpi_rank( comm ) .ne. 0 ) allocate( buffer( pack_size ) )
    call musica_mpi_bcast( buffer , comm )

    if( musica_mpi_rank( comm ) .ne. 0 ) then
      pos = 0
      call type_name%mpi_unpack(   buffer, pos , comm )
      new_grid_t => grid_allocate( type_name )
      call new_grid_t%mpi_unpack(  buffer, pos , comm )
    end if


  end subroutine test_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end program test_tuvx_connection
