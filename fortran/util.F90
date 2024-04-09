! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
module musica_util

  use iso_c_binding,                   only: c_ptr, c_size_t

  implicit none
  private

  public :: string_t_c, to_string_t_c, to_f_string, assert

  !> Wrapper for a c string
  type, bind(c) :: string_t_c
    type(c_ptr) :: ptr_
    integer(c_size_t) :: size_
  end type string_t_c

contains

interface

  !> Create an instance of string_t_c from a c string
  function to_string_t_c( c_string ), bind(c, name='ToConstString')
    import :: string_t_c
    type(c_ptr), value, intent(in) :: c_string
    type(string_t_c) :: to_string_t_c
  end function to_string_t_c

end interface

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Convert a c string to a fortran character array
  function to_f_string( c_string ) result( f_string )

    use iso_c_binding,                 only : c_char, c_ptr

    type(c_ptr), value, intent(in) :: c_string
    character(len=:), allocatable  :: f_string

    integer :: i
    type(string_t_c) :: string
    character(len=1, kind=c_char), pointer :: c_char_array(:)

    string = to_string_t_c( c_string )
    call c_f_pointer( string%ptr_, c_char_array, [ string%size_ + 1 ] )
    allocate( character( len = string%size_ ) :: f_string )
    do i = 1, string%size_
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
        write(*,*) "[MUSICA ERROR at ", file, ":" line, "] ", error_message
      else
      write(*,*) "[MUSICA ERROR at ", file, ":", line, "] Assertion failed"
      stop 3
    end if

  end subroutine assert


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_util