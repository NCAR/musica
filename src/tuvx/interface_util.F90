! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
module tuvx_interface_util

  use iso_c_binding,                   only: c_char, c_int, c_ptr, c_size_t, &
                                             c_null_ptr, c_f_pointer

  implicit none
  private

  public :: string_t_c, string_t, error_t_c, error_t, mapping_t_c, mapping_t, &
            mappings_t_c, to_c_string, to_f_string, assert, delete_string_c, &
            create_string_t_c, delete_string_t_c, allocate_mappings_c

  !> Wrapper for a c string
  type, bind(c) :: string_t_c
    type(c_ptr) :: ptr_ = c_null_ptr
    integer(c_size_t) :: size_ = 0_c_size_t
  end type string_t_c

  !> String type
  type :: string_t
  private
    character(len=:), allocatable :: value_
  contains
    procedure, private, pass(from) :: char_assign_string
    procedure, private, pass(to) :: string_assign_char
    procedure, private, pass(to) :: string_assign_string
    procedure, private, pass(to) :: string_assign_string_t_c
    generic :: assignment(=) => char_assign_string, string_assign_char, &
                                string_assign_string, string_assign_string_t_c
  end type string_t

  interface string_t
    module procedure string_t_constructor_from_string_t_c
  end interface string_t

  !> Wrapper for a c error condition
  type, bind(c) :: error_t_c
    integer(c_int) :: code_ = 0_c_int
    type(string_t_c) :: category_
    type(string_t_c) :: message_
  end type error_t_c

  !> Error type
  type :: error_t
  private
    integer :: code_
    type(string_t) :: category_
    type(string_t) :: message_
  contains
    procedure :: code => error_t_code
    procedure :: category => error_t_category
    procedure :: message => error_t_message
    procedure :: is_success => error_t_is_success
    procedure :: is_error => error_t_is_error
  end type error_t

  interface error_t
    module procedure error_t_constructor
  end interface error_t

  !> Wrapper for a c name-to-index mapping
  type, bind(c) :: mapping_t_c
    type(string_t_c) :: name_
    integer(c_size_t) :: index_ = 0_c_size_t
  end type mapping_t_c

  !> Name-to-index mapping
  type :: mapping_t
  private
    type(string_t) :: name_
    integer :: index_
  contains
    procedure :: mapping_assign_mapping_t_c
    generic :: assignment(=) => mapping_assign_mapping_t_c
    procedure :: name => mapping_name
    procedure :: index => mapping_index
  end type mapping_t

  interface mapping_t
    module procedure mapping_constructor
  end interface mapping_t

  type, bind(c) :: mappings_t_c
    type(c_ptr) :: mappings_ = c_null_ptr
    integer(c_size_t) :: size_ = 0_c_size_t
  end type mappings_t_c

  interface
    function create_string_c( string ) bind(c, name="CreateString")
      import :: string_t_c, c_char
      character(kind=c_char, len=1), intent(in) :: string(*)
      type(string_t_c) :: create_string_c
    end function create_string_c
    function allocate_mappings_c( size ) bind(c, name="AllocateMappingArray")
      import :: c_size_t, c_ptr
      integer(c_size_t), value, intent(in) :: size
      type(c_ptr) :: allocate_mappings_c
    end function allocate_mappings_c
    subroutine delete_string_c( string ) bind(c, name="DeleteString")
      import :: string_t_c
      type(string_t_c), intent(inout) :: string
    end subroutine delete_string_c
  end interface

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function string_t_constructor_from_string_t_c( c_string ) result( new_string )

    type(string_t_c), intent(inout) :: c_string
    type(string_t) :: new_string

    new_string%value_ = to_f_string( c_string )
    call delete_string_c( c_string )
    
  end function string_t_constructor_from_string_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the a string_t as a fortran string
  subroutine char_assign_string( to, from )

    character(len=:), allocatable, intent(inout) :: to
    class(string_t), intent(in) :: from

    if (allocated(to)) deallocate(to)
    if (.not. allocated(from%value_)) then
      allocate( character( len = 0 ) :: to )
      return
    end if
    allocate( to, source = from%value_ )

  end subroutine char_assign_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Assign a string to a string_t object
  subroutine string_assign_char( to, from )

    class(string_t), intent(inout) :: to
    character(len=*), intent(in) :: from

    if (allocated(to%value_)) deallocate(to%value_)
    allocate( to%value_, source = from )

  end subroutine string_assign_char

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copy a string
  subroutine string_assign_string( to, from )

    class(string_t), intent(inout) :: to
    class(string_t), intent(in) :: from

    if (allocated(to%value_)) deallocate(to%value_) 
    allocate( to%value_, source = from%value_ )

  end subroutine string_assign_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copy a c string to a string_t
  subroutine string_assign_string_t_c( to, from )

    class(string_t), intent(inout) :: to
    type(string_t_c), intent(in) :: from

    to%value_ = to_f_string( from )

  end subroutine string_assign_string_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for an error_t object that takes ownership of an error_t_c
  function error_t_constructor( c_error ) result( new_error )

    type(error_t_c), intent(inout) :: c_error
    type(error_t) :: new_error

    new_error%code_ = int( c_error%code_ )
    new_error%category_ = string_t( c_error%category_ )
    new_error%message_ = string_t( c_error%message_ )

  end function error_t_constructor

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the error code
  function error_t_code( this ) result( code )

    class(error_t), intent(in) :: this
    integer :: code

    code = this%code_

  end function error_t_code

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the error category
  function error_t_category( this ) result( category )

    class(error_t), intent(in) :: this
    character(len=:), allocatable :: category

    category = this%category_

  end function error_t_category

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the error message
  function error_t_message( this ) result( message )

    class(error_t), intent(in) :: this
    character(len=:), allocatable :: message

    message = this%message_

  end function error_t_message

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Check if an error_t_c object represents a success condition
  logical function error_t_is_success( this ) result( is_success )

    class(error_t), intent(in) :: this

    is_success = this%code_ == 0

  end function error_t_is_success

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Check if an error_t_c object matches a specific error condition
  logical function error_t_is_error( this, category, code ) result( is_error )

    class(error_t), intent(in) :: this
    character(len=*), intent(in) :: category
    integer, intent(in) :: code

    character(len=:), allocatable :: category_f

    category_f = this%category_
    is_error = int( this%code_ ) == code .and. &
               category_f == category

  end function error_t_is_error

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for a mapping_t object that takes ownership of a mapping_t_c
  !! object
  function mapping_constructor( c_mapping ) result( new_mapping )

    type(mapping_t_c), intent(inout) :: c_mapping
    type(mapping_t) :: new_mapping

    new_mapping%name_ = string_t( c_mapping%name_ )
    new_mapping%index_ = int( c_mapping%index_ )

  end function mapping_constructor

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copies a mapping_t_c object to a mapping_t object
  subroutine mapping_assign_mapping_t_c( to, from )

    class(mapping_t), intent(inout) :: to
    type(mapping_t_c), intent(in) :: from

    to%name_ = from%name_
    to%index_ = int( from%index_ )

  end subroutine mapping_assign_mapping_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the name for a mapping_t object
  function mapping_name( this ) result( name )

    class(mapping_t), intent(in) :: this
    character(len=:), allocatable :: name

    name = this%name_

  end function mapping_name

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the index for a mapping_t object
  function mapping_index( this ) result( index )

    class(mapping_t), intent(in) :: this
    integer :: index

    index = this%index_

  end function mapping_index

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

  !> Creates a new string_t_c from a fortran character array
  function create_string_t_c( f_string ) result( c_string )

    character(len=*), intent(in) :: f_string
    type(string_t_c) :: c_string

    c_string = create_string_c( to_c_string( f_string ) )

  end function create_string_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Delete a string_t_c object
  subroutine delete_string_t_c( string ) &
      bind(c, name="InternalDeleteString")
    use iso_c_binding,                 only : c_char, c_null_ptr, c_size_t, &
                                              c_f_pointer, c_associated

    type(string_t_c), intent(inout) :: string

    character(kind=c_char, len=1), pointer :: c_string_ptr(:)

    if ( c_associated( string%ptr_ ) ) then
      call c_f_pointer( string%ptr_, c_string_ptr, [ string%size_ + 1 ] )
      deallocate( c_string_ptr )
      string%ptr_ = c_null_ptr
      string%size_ = 0_c_size_t
    end if

  end subroutine delete_string_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Convert a c string to a fortran character array
  function to_f_string( c_string ) result( f_string )

    use iso_c_binding,                 only : c_char, c_ptr, c_f_pointer, &
                                              c_associated

    type(string_t_c), value, intent(in) :: c_string
    character(len=:), allocatable  :: f_string

    integer :: i
    character(len=1, kind=c_char), pointer :: c_char_array(:)

    if ( .not. c_associated( c_string%ptr_ ) ) then
      allocate( character( len = 0 ) :: f_string )
      return
    end if
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

end module tuvx_interface_util