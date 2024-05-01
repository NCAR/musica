! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! Tests for the musica_util module
program test_util

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )

  use, intrinsic :: iso_c_binding, only: c_char
  use musica_util
  implicit none

  interface
    function create_string_c( string ) bind(c, name="CreateString")
      import :: string_t_c, c_char
      character(kind=c_char, len=1), intent(in) :: string(*)
      type(string_t_c) :: create_string_c
    end function create_string_c
    subroutine delete_string_c( string ) bind(c, name="DeleteString")
      import :: string_t_c
      type(string_t_c), intent(inout) :: string
    end subroutine delete_string_c
  end interface

  call test_string_t()

contains

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

    a_c = create_string_c( "qux" )
    b_c = create_string_c( "quux" )
    c_c = create_string_c( "corge" )

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



end program test_util