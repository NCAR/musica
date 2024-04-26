! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
module musica_util

  use iso_c_binding,                   only: c_int, c_ptr, c_size_t

  implicit none
  private

  public :: string_t_c, error_t_c, to_c_string, to_f_string, assert, &
            is_success, is_error, code, category, message

  !> Wrapper for a c string
  type, bind(c) :: string_t_c
    type(c_ptr) :: ptr_
    integer(c_size_t) :: size_
  end type string_t_c

  !> Wrapper for an error condition
  type, bind(c) :: error_t_c
    integer(c_int) :: code_
    type(string_t_c) :: category_
    type(string_t_c) :: message_
  end type error_t_c

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Convert a fortran character array to a c string
  function to_c_string( f_string ) result( c_string )

    use iso_c_binding,                 only : c_char, c_null_char

    character(len=*), intent(in) :: f_string
    character(len=1, kind=c_char), allocatable :: c_string(:)

    integer :: N, i

    N = len_trim( f_string )
    allocate( c_string( N + 1 ) )
    do i = 1, N
      c_string(i) = f_string(i:i)
    end do
    c_string(N + 1) = c_null_char

  end function to_c_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Convert a c string to a fortran character array
  function to_f_string( c_string ) result( f_string )

    use iso_c_binding,                 only : c_char, c_ptr, c_f_pointer

    type(string_t_c), value, intent(in) :: c_string
    character(len=:), allocatable  :: f_string

    integer :: i
    character(len=1, kind=c_char), pointer :: c_char_array(:)

    call c_f_pointer( c_string%ptr_, c_char_array, [ c_string%size_ + 1 ] )
    allocate( character( len = c_string%size_ ) :: f_string )
    do i = 1, c_string%size_
      f_string(i:i) = c_char_array(i)
    end do

  end function to_f_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Assert that a condition is true
  subroutine assert( condition, file, line, error_message )

    logical, intent(in) :: condition
    character(len=*), intent(in) :: file
    integer, intent(in) :: line
    character(len=*), intent(in), optional :: error_message

    if ( .not. condition ) then
      if ( present( error_message ) ) then
        write(*,*) "[MUSICA ERROR at ", file, ":", line, "] ", error_message
      else
        write(*,*) "[MUSICA ERROR at ", file, ":", line, "] Assertion failed"
      end if
      stop 3
    end if

  end subroutine assert


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Check if an error_t_c object represents a success condition
  logical function is_success( error )

    type(error_t_c), intent(in) :: error

    is_success = error%code_ == 0_c_int

  end function is_success

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Check if an error_t_c object matches a specific error condition
  logical function is_error( error, category, code )

    type(error_t_c), intent(in) :: error
    character(len=*), intent(in) :: category
    integer, intent(in) :: code

    is_error = int( error%code_ ) == code .and. &
               to_f_string( error%category_ ) == category

  end function is_error

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Extract the error code from an error_t_c object
  integer function code( error )

    type(error_t_c), intent(in) :: error

    code = int( error%code_ )

  end function code

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Extract the error category from an error_t_c object
  function category( error )

    type(error_t_c), intent(in) :: error
    character(len=:), allocatable :: category

    category = to_f_string( error%category_ )

  end function category

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Extract the error message from an error_t_c object
  function message( error )

    type(error_t_c), intent(in) :: error
    character(len=:), allocatable :: message

    message = to_f_string( error%message_ )

  end function message

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_util