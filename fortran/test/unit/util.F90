! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! Tests for the musica_util module
program test_util

#include "musica/error.hpp"

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )

  use, intrinsic :: iso_c_binding, only: c_char, c_ptr, c_loc, c_size_t, c_null_char
  use musica_util
  implicit none

  integer, parameter :: dk = musica_dk

  call test_string_t()
  call test_error_t()
  call test_mapping_t()
  call test_index_mapping_t()

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function create_c_string( f_string ) result( c_string )
    character(len=*), intent(in) :: f_string
    type(string_t_c) :: c_string
    c_string = create_string_c( to_c_string( f_string ) )
  end function create_c_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Tests the string_t type
  subroutine test_string_t()

    type(string_t) :: a, b, c
    type(string_t_c) :: a_c, b_c, c_c
    character(len=:), allocatable :: a_char, b_char, c_char

    a = "foo"
    b = "bar"
    c = a

    a_char = a
    b_char = b
    c_char = c
    ASSERT_EQ( a_char, "foo" )
    ASSERT_EQ( b_char, "bar" )
    ASSERT_EQ( c_char, "foo" )

    a = b
    b = c
    c = "baz"

    a_char = a
    b_char = b
    c_char = c
    ASSERT_EQ( a_char, "bar" )
    ASSERT_EQ( b_char, "foo" )
    ASSERT_EQ( c_char, "baz" )

    a_c = create_c_string( "qux" )
    b_c = create_c_string( "quux" )
    c_c = create_c_string( "corge" )

    ! take ownership of the string
    a = string_t( a_c )
    b = string_t( b_c )
    c = string_t( c_c )

    a = c
    c = "grault"

    a_char = a
    b_char = b
    c_char = c

    ASSERT_EQ( a_char, "corge" )
    ASSERT_EQ( b_char, "quux" )
    ASSERT_EQ( c_char, "grault" )

  end subroutine test_string_t

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine test_error_t()

    type(error_t) :: a, b, c
    type(error_t_c) :: a_c, b_c, c_c

    a_c%code_ = 12
    a_c%category_ = create_c_string( "bar" )
    a_c%message_ = create_c_string( "foo" )

    b_c%code_ = 0
    b_c%category_ = create_c_string( "" )
    b_c%message_ = create_c_string( "Success" )

    c_c%code_ = 56
    c_c%category_ = create_c_string( "corge" )
    c_c%message_ = create_c_string( "grault" )

    a = error_t( a_c )
    b = error_t( b_c )
    c = error_t( c_c )

    ASSERT_EQ( a%code(), 12 )
    ASSERT_EQ( a%category(), "bar" )
    ASSERT_EQ( a%message(), "foo" )
    ASSERT( a%is_error( "bar", 12 ) )
    ASSERT( .not. a%is_error( "baz", 12 ) )
    ASSERT( .not. a%is_error( "bar", 13 ) )

    ASSERT_EQ( b%code(), 0 )
    ASSERT_EQ( b%category(), "" )
    ASSERT_EQ( b%message(), "Success" )
    ASSERT( b%is_success() )
    ASSERT( .not. b%is_error( "", 1 ) )

    ASSERT_EQ( c%code(), 56 )
    ASSERT_EQ( c%category(), "corge" )
    ASSERT_EQ( c%message(), "grault" )
    ASSERT( c%is_error( "corge", 56 ) )
    ASSERT( .not. c%is_error( "baz", 56 ) )
    ASSERT( .not. c%is_error( "corge", 57 ) )

    c = b
    b = a

    ASSERT_EQ( c%code(), 0 )
    ASSERT_EQ( c%category(), "" )
    ASSERT_EQ( c%message(), "Success" )

    ASSERT_EQ( b%code(), 12 )
    ASSERT_EQ( b%category(), "bar" )
    ASSERT_EQ( b%message(), "foo" )

    a = error_t( 34, "qux", "quux" )
    
    ASSERT_EQ( a%code(), 34 )
    ASSERT_EQ( a%category(), "qux" )
    ASSERT_EQ( a%message(), "quux" )

  end subroutine test_error_t

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine test_mapping_t()

    type(mapping_t), pointer :: a, b, c
    type(mapping_t_c) :: a_c, b_c, c_c
    type(mapping_t_c), target :: c_mappings(3)
    type(c_ptr) :: c_mappings_ptr
    type(mapping_t), allocatable :: f_mappings(:)
    type(mappings_t), pointer :: mappings
    type(mappings_t) :: mappings_copy
    logical :: found
    type(error_t) :: error
    integer :: index

    a_c%index_ = 3
    a_c%name_ = create_c_string( "foo" )

    b_c%index_ = 5
    b_c%name_ = create_c_string( "bar" )

    c_c%index_ = 8
    c_c%name_ = create_c_string( "baz" )

    ! construct and take ownership of the c mapping
    a => mapping_t( a_c )
    b => mapping_t( b_c )
    c => mapping_t( c_c )

    ASSERT_EQ( a%index(), 4 )
    ASSERT_EQ( a%name(), "foo" )

    ASSERT_EQ( b%index(), 6 )
    ASSERT_EQ( b%name(), "bar" )

    ASSERT_EQ( c%index(), 9 )
    ASSERT_EQ( c%name(), "baz" )

    ! copy assignment
    c = b
    b = a

    ASSERT_EQ( c%index(), 6 )
    ASSERT_EQ( c%name(), "bar" )

    ASSERT_EQ( b%index(), 4 )
    ASSERT_EQ( b%name(), "foo" )

    a_c%index_ = 13
    a_c%name_ = create_c_string( "qux" )

    ! copy assignment of c mapping
    a = a_c
    b = a_c

    ! construct and take ownership of the c mapping
    deallocate( c )
    c => mapping_t( a_c )

    ASSERT_EQ( a%index(), 14 )
    ASSERT_EQ( a%name(), "qux" )

    ASSERT_EQ( b%index(), 14 )
    ASSERT_EQ( b%name(), "qux" )

    ASSERT_EQ( c%index(), 14 )
    ASSERT_EQ( c%name(), "qux" )

    deallocate( a )
    deallocate( b )
    deallocate( c )

    c_mappings(1)%index_ = 21
    c_mappings(1)%name_ = create_c_string( "quux" )
    c_mappings(2)%index_ = 34
    c_mappings(2)%name_ = create_c_string( "corge" )
    c_mappings(3)%index_ = 55
    c_mappings(3)%name_ = create_c_string( "grault" )

    c_mappings_ptr = c_loc( c_mappings )
    call copy_mappings( c_mappings_ptr, 3_c_size_t, f_mappings )
    call delete_string_c( c_mappings(1)%name_ )
    call delete_string_c( c_mappings(2)%name_ )
    call delete_string_c( c_mappings(3)%name_ )

    ! indices should be shifted by 1 for fortran
    ASSERT_EQ( size( f_mappings ), 3 )
    ASSERT_EQ( f_mappings(1)%index(), 22 )
    ASSERT_EQ( f_mappings(1)%name(), "quux" )
    ASSERT_EQ( f_mappings(2)%index(), 35 )
    ASSERT_EQ( f_mappings(2)%name(), "corge" )
    ASSERT_EQ( f_mappings(3)%index(), 56 )
    ASSERT_EQ( f_mappings(3)%name(), "grault" )

    ! find mappings by name
    ASSERT_EQ( find_mapping_index( f_mappings, "quux"   ), 22 )
    ASSERT_EQ( find_mapping_index( f_mappings, "corge"  ), 35 )
    ASSERT_EQ( find_mapping_index( f_mappings, "grault" ), 56 )
    ASSERT_EQ( find_mapping_index( f_mappings, "foo"    ), -1 )
    ASSERT_EQ( find_mapping_index( f_mappings, "corge", found ), 35 )
    ASSERT( found )
    ASSERT_EQ( find_mapping_index( f_mappings, "foo",   found ), -1 )
    ASSERT( .not. found )

    ! create mappings object from array
    mappings => mappings_t( f_mappings )
    ASSERT_EQ( mappings%size(), 3 )
    ASSERT_EQ( mappings%index( 1 ), 22 )
    ASSERT_EQ( mappings%name( 1 ), "quux" )
    ASSERT_EQ( mappings%index( 2 ), 35 )
    ASSERT_EQ( mappings%name( 2 ), "corge" )
    ASSERT_EQ( mappings%index( 3 ), 56 )
    ASSERT_EQ( mappings%name( 3 ), "grault" )

    ! find mappings by name
    ASSERT_EQ( mappings%index( "quux", error ), 22 )
    ASSERT( error%is_success() )
    ASSERT_EQ( mappings%index( "corge", error ), 35 )
    ASSERT( error%is_success() )
    ASSERT_EQ( mappings%index( "grault", error ), 56 )
    ASSERT( error%is_success() )
    index = mappings%index( "foo", error )
    ASSERT( .not. error%is_success() )

    ! copy mappings
    mappings_copy = mappings
    deallocate( mappings )
    ASSERT_EQ( mappings_copy%size(), 3 )
    ASSERT_EQ( mappings_copy%index( 1 ), 22 )
    ASSERT_EQ( mappings_copy%name( 1 ), "quux" )
    ASSERT_EQ( mappings_copy%index( 2 ), 35 )
    ASSERT_EQ( mappings_copy%name( 2 ), "corge" )
    ASSERT_EQ( mappings_copy%index( 3 ), 56 )
    ASSERT_EQ( mappings_copy%name( 3 ), "grault" )
    ASSERT_EQ( mappings_copy%index( "quux", error ), 22 )
    ASSERT( error%is_success() )
    ASSERT_EQ( mappings_copy%index( "corge", error ), 35 )
    ASSERT( error%is_success() )
    ASSERT_EQ( mappings_copy%index( "grault", error ), 56 )
    ASSERT( error%is_success() )
    index = mappings_copy%index( "foo", error )
    ASSERT( .not. error%is_success() )
    
  end subroutine test_mapping_t

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine build_and_check_index_mapping_t(config, should_pass_map_all)

    type(configuration_t), intent(inout) :: config
    logical, intent(in) :: should_pass_map_all

    type(mapping_t), pointer :: map
    type(mapping_t), allocatable :: f_map(:)
    type(mappings_t), pointer :: source_map, target_map
    type(index_mappings_t), pointer :: index_mappings
    type(error_t) :: error
    real(dk) :: source_data(2,5), target_data(3,4)

    allocate( f_map( 2 ) )
    map => mapping_t( "Test", 2 )
    f_map( 1 ) = map
    deallocate( map )
    map => mapping_t( "Test2", 5 )
    f_map( 2 ) = map
    deallocate( map )
    source_map => mappings_t( f_map )
    map => mapping_t( "Test2", 3 )
    f_map( 1 ) = map
    deallocate( map )
    map => mapping_t( "Test3", 1 )
    f_map( 2 ) = map
    deallocate( map )
    target_map => mappings_t( f_map )
    deallocate( f_map )

    index_mappings => index_mappings_t( config, MUSICA_INDEX_MAPPINGS_MAP_ALL, &
        source_map, target_map, error )
    if ( should_pass_map_all ) then
      ASSERT( error%is_success() )
      source_data(2,:) = (/ 1.0_dk, 2.0_dk, 3.0_dk, 4.0_dk, 5.0_dk /)
      target_data(2,:) = (/ 10.0_dk, 20.0_dk, 30.0_dk, 40.0_dk /)

      ASSERT_EQ( index_mappings%size( ), 2 )
      call index_mappings%copy_data( source_data(2,:), target_data(2,:) )
      ASSERT_EQ( target_data( 2, 1 ), 5.0_dk * 0.82_dk )
      ASSERT_EQ( target_data( 2, 2 ), 20.0_dk )
      ASSERT_EQ( target_data( 2, 3 ), 2.0_dk )
      ASSERT_EQ( target_data( 2, 4 ), 40.0_dk )
    else
      ASSERT( error%code() == MUSICA_ERROR_CODE_MAPPING_NOT_FOUND )
    end if
    deallocate( index_mappings )
    index_mappings => index_mappings_t( config, MUSICA_INDEX_MAPPINGS_MAP_ANY, &
        source_map, target_map, error )
    ASSERT( error%is_success() )

    source_data(2,:) = (/ 1.0_dk, 2.0_dk, 3.0_dk, 4.0_dk, 5.0_dk /)
    target_data(2,:) = (/ 10.0_dk, 20.0_dk, 30.0_dk, 40.0_dk /)

    ASSERT_EQ( index_mappings%size( ), 2 )
    call index_mappings%copy_data( source_data(2,:), target_data(2,:) )
    ASSERT_EQ( target_data( 2, 1 ), 5.0_dk * 0.82_dk )
    ASSERT_EQ( target_data( 2, 2 ), 20.0_dk )
    ASSERT_EQ( target_data( 2, 3 ), 2.0_dk )
    ASSERT_EQ( target_data( 2, 4 ), 40.0_dk )
    deallocate( index_mappings )
    deallocate( source_map )
    deallocate( target_map )

  end subroutine build_and_check_index_mapping_t 

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Tests the index_mapping_t type
  subroutine test_index_mapping_t()

    type(configuration_t) :: config
    type(error_t) :: error

    call config%load_from_string( &
      "- source: Test"//new_line('a')// &
      "  target: Test2"//new_line('a')// &
      "- source: Test2"//new_line('a')// & 
      "  target: Test3"//new_line('a')// &
      "  scale factor: 0.82"//new_line('a'), error )
    ASSERT( error%is_success() )
    call build_and_check_index_mapping_t( config, .true. )

    call config%load_from_file( "test/data/util_index_mapping_from_file.json", &
                                error )
    ASSERT( error%is_success() )
    call build_and_check_index_mapping_t( config, .true. )

    call config%load_from_string( &
      "- source: Test"//new_line('a')// &
      "  target: Test2"//new_line('a')// &
      "- source: Test2"//new_line('a')// & 
      "  target: Test3"//new_line('a')// &
      "  scale factor: 0.82"//new_line('a')// &
      "- source: Test"//new_line('a')// &
      "  target: Test4"//new_line('a')//new_line('a'), error )
    ASSERT( error%is_success() )
    call build_and_check_index_mapping_t( config, .false. )

  end subroutine test_index_mapping_t

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end program test_util