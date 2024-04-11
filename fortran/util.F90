! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
module musica_util

  use iso_c_binding,                   only: c_ptr, c_size_t

  implicit none
  private

  public :: string_t_c, to_c_string, to_f_string, assert

  !> Wrapper for a c string
  type, bind(c) :: string_t_c
    type(c_ptr) :: ptr_
    integer(c_size_t) :: size_
  end type string_t_c

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

end module musica_util