! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_util

  use iso_c_binding,                   only: c_char, c_int, c_ptr, c_size_t, &
                                             c_null_ptr, c_f_pointer
  use iso_fortran_env,                 only: real32, real64

  implicit none
  private

  public :: string_t_c, string_t, error_t_c, error_t, configuration_t, &
            mapping_t_c, mapping_t, mappings_t_c, mappings_t, index_mappings_t, &
            to_c_string, to_f_string, assert, copy_mappings, delete_string_c, &
            create_string_c, musica_rk, musica_dk, find_mapping_index, &
            MUSICA_INDEX_MAPPINGS_UNDEFINED, MUSICA_INDEX_MAPPINGS_MAP_ANY, &
            MUSICA_INDEX_MAPPINGS_MAP_ALL

  !> Single precision kind
  integer, parameter :: musica_rk = real32

  !> Double precision kind
  integer, parameter :: musica_dk = real64

  !> Wrapper for a c string
  type, bind(c) :: string_t_c
    type(c_ptr) :: ptr_ = c_null_ptr
    integer(c_size_t) :: size_ = 0_c_size_t
  end type string_t_c

  !> String type
  type :: string_t
    character(len=:), allocatable :: value_
  contains
    procedure :: get_char_array => get_char_array_string_t
    procedure, private, pass(from) :: char_assign_string
    procedure, private, pass(to) :: string_assign_char
    procedure, private, pass(to) :: string_assign_string_t_c
    generic :: assignment(=) => char_assign_string, string_assign_char, &
                                string_assign_string_t_c
  end type string_t

  interface string_t
    module procedure string_t_constructor_from_string_t_c
  end interface string_t

  !> Wrapper for an c error condition
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
    module procedure error_t_constructor_from_error_t_c
    module procedure error_t_constructor_from_fortran
  end interface error_t

  !> Wrapper for a c configuration
  type, bind(c) :: configuration_t_c
    type(c_ptr) :: data_ = c_null_ptr
  end type configuration_t_c

  !> Configuration type
  type :: configuration_t
    type(configuration_t_c) :: configuration_c_
  contains
    procedure :: load_from_string => configuration_load_from_string
    procedure :: load_from_file => configuration_load_from_file
    final :: configuration_finalize
  end type configuration_t

  !> Wrapper for a c name-to-index mapping
  !!
  !! Index is 0-based for use in C
  type, bind(c) :: mapping_t_c
    type(string_t_c) :: name_
    integer(c_size_t) :: index_ = 0_c_size_t
  end type mapping_t_c

  !> Name-to-index mapping
  !!
  !! Indexing is 1-based for use in Fortran
  type :: mapping_t
  private
    type(mapping_t_c) :: mapping_c_
  contains
    procedure :: mapping_assign_mapping_t_c
    procedure :: mapping_assign_mapping_t
    generic :: assignment(=) => mapping_assign_mapping_t_c, &
                                mapping_assign_mapping_t
    procedure :: name => mapping_name
    procedure :: index => mapping_index
    final :: mapping_finalize
  end type mapping_t

  interface mapping_t
    module procedure mapping_constructor
    module procedure mapping_constructor_from_data
  end interface mapping_t

  !> Wrapper for a c array of name-to-index mappings
  type, bind(c) :: mappings_t_c
    type(c_ptr), public :: mappings_ = c_null_ptr
    integer(c_size_t) :: size_ = 0_c_size_t
  end type mappings_t_c

  !> Array of name-to-index mappings
  type :: mappings_t
    type(mappings_t_c) :: mappings_c_
  contains
    procedure, private :: mappings_assign_mappings_t
    generic :: assignment(=) => mappings_assign_mappings_t
    procedure :: name => mappings_name
    procedure, private :: mappings_index
    procedure, private :: mappings_index_by_name
    generic :: index => mappings_index, mappings_index_by_name
    procedure :: size => mappings_size
    final :: mappings_finalize
  end type mappings_t

  interface mappings_t
    module procedure mappings_constructor_from_mappings_t_c
    module procedure mappings_constructor_from_mapping_t_array
  end interface mappings_t

  !> Index mappings options
  enum, bind(c)
    enumerator :: MUSICA_INDEX_MAPPINGS_UNDEFINED = 0
    enumerator :: MUSICA_INDEX_MAPPINGS_MAP_ANY   = 1
    enumerator :: MUSICA_INDEX_MAPPINGS_MAP_ALL   = 2
  end enum

  !> Wrapper for a c index-to-index mapping array
  type, bind(c) :: index_mappings_t_c
    type(c_ptr) :: mappings_ = c_null_ptr
    integer(c_size_t) :: size_ = 0_c_size_t
  end type index_mappings_t_c

  !> Array of index-to-index mappings
  type :: index_mappings_t
  private
    type(index_mappings_t_c) :: mappings_c_
  contains
    procedure :: size => index_mappings_size
    procedure :: copy_data
    final :: index_mappings_finalize
  end type index_mappings_t

  interface index_mappings_t
    module procedure index_mappings_constructor
  end interface index_mappings_t

  interface
    function create_string_c( string ) bind(c, name="CreateString")
      import :: string_t_c, c_char
      character(kind=c_char, len=1), intent(in) :: string(*)
      type(string_t_c) :: create_string_c
    end function create_string_c

    pure subroutine delete_string_c( string ) bind(c, name="DeleteString")
      import :: string_t_c
      type(string_t_c), intent(inout) :: string
    end subroutine delete_string_c
    
    function load_configuration_from_string_c( string, error ) &
        bind(c, name="LoadConfigurationFromString")
      import :: configuration_t_c, c_char, error_t_c
      character(kind=c_char, len=1), intent(in) :: string(*)
      type(error_t_c), intent(inout) :: error
      type(configuration_t_c) :: load_configuration_from_string_c
    end function load_configuration_from_string_c
    
    function load_configuration_from_file_c( file, error ) &
        bind(c, name="LoadConfigurationFromFile")
      import :: configuration_t_c, c_char, error_t_c
      character(kind=c_char, len=1), intent(in) :: file(*)
      type(error_t_c), intent(inout) :: error
      type(configuration_t_c) :: load_configuration_from_file_c
    end function load_configuration_from_file_c
    
    pure subroutine delete_configuration_c( configuration ) &
        bind(c, name="DeleteConfiguration")
      import :: configuration_t_c
      type(configuration_t_c), intent(inout) :: configuration
    end subroutine delete_configuration_c
    
    function create_mappings_c( size ) bind(c, name="CreateMappings")
      import :: mappings_t_c, c_size_t
      integer(c_size_t), value, intent(in) :: size
      type(mappings_t_c) :: create_mappings_c
    end function create_mappings_c
    
    function create_index_mappings_c(configuration, options, source, target, &
        error) bind(c, name="CreateIndexMappings")
      import :: index_mappings_t_c, configuration_t_c, error_t_c, &
                mappings_t_c, c_int
      type(configuration_t_c), value, intent(in)    :: configuration
      integer(c_int),          value, intent(in)    :: options
      type(mappings_t_c),      value, intent(in)    :: source
      type(mappings_t_c),      value, intent(in)    :: target
      type(error_t_c),                intent(inout) :: error
      type(index_mappings_t_c) :: create_index_mappings_c
    end function create_index_mappings_c
    
    pure subroutine delete_mapping_c( mapping ) bind(c, name="DeleteMapping")
      import :: mapping_t_c
      type(mapping_t_c), intent(inout) :: mapping
    end subroutine delete_mapping_c
    
    function find_mapping_index_c( mappings, name, error ) result( index ) &
        bind(c, name="FindMappingIndex")
      import :: mappings_t_c, error_t_c, c_char, c_size_t
      type(mappings_t_c), value,      intent(in)    :: mappings
      character(kind=c_char, len=1),  intent(in)    :: name(*)
      type(error_t_c),                intent(inout) :: error
      integer(c_size_t) :: index
    end function find_mapping_index_c
    
    pure subroutine delete_mappings_c( mappings ) bind(c, name="DeleteMappings")
      import :: mappings_t_c
      type(mappings_t_c), intent(inout) :: mappings
    end subroutine delete_mappings_c
    
    pure subroutine delete_index_mappings_c( mappings ) &
        bind(c, name="DeleteIndexMappings")
      import :: index_mappings_t_c
      type(index_mappings_t_c), intent(inout) :: mappings
    end subroutine delete_index_mappings_c
    
    function get_index_mappings_size_c( mappings ) result( size ) &
        bind(c, name="GetIndexMappingsSize")
      import :: index_mappings_t_c, c_size_t
      type(index_mappings_t_c), value, intent(in) :: mappings
      integer(c_size_t) :: size
    end function get_index_mappings_size_c
    
    subroutine copy_data_c(mappings, source, target) bind(c, name="CopyData")
      import :: index_mappings_t_c, c_ptr
      type(index_mappings_t_c), value, intent(in) :: mappings
      type(c_ptr),              value, intent(in) :: source
      type(c_ptr),              value, intent(in) :: target
    end subroutine copy_data_c
  end interface

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function get_char_array_string_t( this ) result ( value )

    class(string_t), intent(in) :: this
    character(len=:), allocatable :: value

    allocate( value, source = this%value_ )

  end function get_char_array_string_t

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

  !> Copy a c string to a string_t
  subroutine string_assign_string_t_c( to, from )

    class(string_t), intent(inout) :: to
    type(string_t_c), intent(in) :: from

    to%value_ = to_f_string( from )

  end subroutine string_assign_string_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Load a configuration from a string
  subroutine configuration_load_from_string( this, string, error )

    use iso_c_binding, only: c_associated

    class(configuration_t), intent(inout) :: this
    character(len=*), intent(in) :: string
    type(error_t), intent(out) :: error

    type(error_t_c) :: error_c

    if (c_associated(this%configuration_c_%data_)) then
      call delete_configuration_c(this%configuration_c_)
    end if
    this%configuration_c_ = &
        load_configuration_from_string_c( to_c_string( string ), error_c )
    error = error_t( error_c )

  end subroutine configuration_load_from_string

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Load a configuration from a file
  subroutine configuration_load_from_file( this, file, error )

    use iso_c_binding, only: c_associated

    class(configuration_t), intent(inout) :: this
    character(len=*), intent(in) :: file
    type(error_t), intent(out) :: error

    type(error_t_c) :: error_c

    if (c_associated(this%configuration_c_%data_)) then
      call delete_configuration_c(this%configuration_c_)
    end if
    this%configuration_c_ = &
        load_configuration_from_file_c( to_c_string( file ), error_c )
    error = error_t( error_c )

  end subroutine configuration_load_from_file

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Finalize a configuration
  elemental subroutine configuration_finalize( this )

    use iso_c_binding, only: c_associated

    type(configuration_t), intent(inout) :: this

    if (c_associated(this%configuration_c_%data_)) then
      call delete_configuration_c(this%configuration_c_)
    end if

  end subroutine configuration_finalize

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for an error_t object that takes ownership of an error_t_c
  function error_t_constructor_from_error_t_c( c_error ) result( new_error )

    type(error_t_c), intent(inout) :: c_error
    type(error_t) :: new_error

    new_error%code_ = int( c_error%code_ )
    new_error%category_ = string_t( c_error%category_ )
    new_error%message_ = string_t( c_error%message_ )
    c_error%category_%ptr_ = c_null_ptr
    c_error%category_%size_ = 0_c_size_t
    c_error%message_%ptr_ = c_null_ptr
    c_error%message_%size_ = 0_c_size_t
    c_error%code_ = 0_c_int

  end function error_t_constructor_from_error_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for an error_t object from fortran data
  function error_t_constructor_from_fortran( code, category, message ) &
      result( new_error )

    integer, intent(in) :: code
    character(len=*), intent(in) :: category
    character(len=*), intent(in) :: message
    type(error_t) :: new_error

    new_error%code_ = code
    new_error%category_ = category
    new_error%message_ = message

  end function error_t_constructor_from_fortran

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
    type(mapping_t), pointer :: new_mapping

    allocate( new_mapping )
    new_mapping%mapping_c_%name_%ptr_ = c_mapping%name_%ptr_
    new_mapping%mapping_c_%name_%size_ = c_mapping%name_%size_
    c_mapping%name_%ptr_  = c_null_ptr
    c_mapping%name_%size_ = 0_c_size_t
    new_mapping%mapping_c_%index_ = c_mapping%index_

  end function mapping_constructor

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for a mapping_t object from a name and index
  function mapping_constructor_from_data( name, index ) result( new_mapping )

    character(len=*), intent(in) :: name
    integer, intent(in) :: index
    type(mapping_t), pointer :: new_mapping

    allocate( new_mapping )
    new_mapping%mapping_c_%name_ = create_string_c( to_c_string( name ) )
    new_mapping%mapping_c_%index_ = int( index - 1, c_size_t )

  end function mapping_constructor_from_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copies a mapping_t_c object to a mapping_t object
  subroutine mapping_assign_mapping_t_c( to, from )

    class(mapping_t), intent(inout) :: to
    type(mapping_t_c), intent(in) :: from

    character(kind=c_char, len=1), pointer :: name(:)

    call delete_string_c( to%mapping_c_%name_ )
    call c_f_pointer( from%name_%ptr_, name, [ from%name_%size_ + 1 ] )
    to%mapping_c_%name_ = create_string_c( name )
    to%mapping_c_%index_ = from%index_

  end subroutine mapping_assign_mapping_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copies a mapping_t object to a mapping_t object
  subroutine mapping_assign_mapping_t( to, from )

    class(mapping_t), intent(inout) :: to
    class(mapping_t), intent(in) :: from

    character(kind=c_char, len=1), pointer :: name(:)

    call delete_mapping_c( to%mapping_c_ )
    call c_f_pointer( from%mapping_c_%name_%ptr_, name, &
                      [ from%mapping_c_%name_%size_ + 1 ] )
    to%mapping_c_%name_ = create_string_c( name )
    to%mapping_c_%index_ = from%mapping_c_%index_

  end subroutine mapping_assign_mapping_t

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the name for a mapping_t object
  function mapping_name( this ) result( name )

    class(mapping_t), intent(in) :: this
    character(len=:), allocatable :: name

    name = to_f_string( this%mapping_c_%name_ )

  end function mapping_name

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the index for a mapping_t object
  function mapping_index( this ) result( index )

    class(mapping_t), intent(in) :: this
    integer :: index

    index = int( this%mapping_c_%index_ ) + 1

  end function mapping_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copies mappings from a c array to a fortran array
  subroutine copy_mappings( c_mappings, n_mappings, f_mappings )

    type(c_ptr),                  intent(in)    :: c_mappings
    integer(c_size_t),            intent(in)    :: n_mappings
    type(mapping_t), allocatable, intent(inout) :: f_mappings(:)

    integer :: i
    type(mapping_t_c), pointer :: mappings(:)

    call c_f_pointer( c_mappings, mappings, [ n_mappings ] )
    if ( allocated( f_mappings ) ) deallocate( f_mappings )
    allocate( f_mappings( n_mappings ) )
    do i = 1, n_mappings
      f_mappings(i) = mappings(i)
    end do

  end subroutine copy_mappings

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Free memory for a mapping_t object
  elemental subroutine mapping_finalize( this )

    type(mapping_t), intent(inout) :: this

    call delete_mapping_c( this%mapping_c_ )

  end subroutine mapping_finalize

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Finds the index of a mapping in an array of mappings
  function find_mapping_index( mappings, name, found ) result( index )

    type(mapping_t),   intent(in)  :: mappings(:)
    character(len=*),  intent(in)  :: name
    logical, optional, intent(out) :: found
    integer :: index

    integer :: i

    index = -1
    if ( present( found ) ) found = .false.
    do i = 1, size( mappings )
      if ( mappings(i)%name() == name ) then
        index = mappings(i)%index()
        if ( present( found ) ) found = .true.
        return
      end if
    end do

  end function find_mapping_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for a mappings_t object from a mappings_t_c object
  !! that takes ownership of the mappings_t_c object
  function mappings_constructor_from_mappings_t_c( c_mappings ) &
      result( mappings )

    type(mappings_t_c), intent(inout) :: c_mappings
    type(mappings_t), pointer :: mappings

    allocate( mappings )
    mappings%mappings_c_ = c_mappings

  end function mappings_constructor_from_mappings_t_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for a mappings_t object from an array of mapping_t objects
  !! that copies the mapping_t objects
  function mappings_constructor_from_mapping_t_array( mappings ) &
      result( new_mappings )

    type(mapping_t), intent(in) :: mappings(:)
    type(mappings_t), pointer :: new_mappings

    integer :: i
    type(mapping_t_c), pointer :: mappings_c(:)

    allocate( new_mappings )
    new_mappings%mappings_c_ = &
        create_mappings_c( int( size( mappings ), c_size_t ) )
    call c_f_pointer( new_mappings%mappings_c_%mappings_, mappings_c, &
                      [ new_mappings%mappings_c_%size_ ] )
    do i = 1, size( mappings )
      mappings_c(i)%name_ = &
          create_string_c( to_c_string( mappings(i)%name() ) )
      mappings_c(i)%index_ = mappings(i)%index() - 1
    end do

  end function mappings_constructor_from_mapping_t_array

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copies a mappings_t object to a mappings_t object
  subroutine mappings_assign_mappings_t( to, from )

    class(mappings_t), intent(inout) :: to
    class(mappings_t), intent(in) :: from

    integer :: i
    type(mapping_t), allocatable :: mappings(:)
    type(mapping_t_c), pointer :: mappings_c(:)

    call delete_mappings_c( to%mappings_c_ )
    call copy_mappings( from%mappings_c_%mappings_, &
                      from%mappings_c_%size_, mappings )
    to%mappings_c_ = &
        create_mappings_c( int( size( mappings ), c_size_t ) )
    call c_f_pointer( to%mappings_c_%mappings_, mappings_c, &
                      [ to%mappings_c_%size_ ] )
    do i = 1, size( mappings )
      mappings_c(i)%name_ = &
          create_string_c( to_c_string( mappings(i)%name() ) )
      ! Convert 1-based Fortran index to 0-based C index
      mappings_c(i)%index_ = mappings(i)%index() - 1
    end do

  end subroutine mappings_assign_mappings_t

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the name for a mappings_t array element
  function mappings_name( this, index ) result( name )

    class(mappings_t), intent(in) :: this
    integer, intent(in) :: index
    character(len=:), allocatable :: name

    type(mapping_t_c), pointer :: mappings(:)

    call c_f_pointer( this%mappings_c_%mappings_, mappings, &
                      [ this%mappings_c_%size_ ] )
    name = to_f_string( mappings(index)%name_ )

  end function mappings_name

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the index for a mappings_t array element
  function mappings_index( this, map_index ) result( index )

    class(mappings_t), intent(in) :: this
    integer, intent(in) :: map_index
    integer :: index

    type(mapping_t_c), pointer :: mappings(:)

    call c_f_pointer( this%mappings_c_%mappings_, mappings, &
                      [ this%mappings_c_%size_ ] )
    index = int( mappings(map_index)%index_ ) + 1

  end function mappings_index

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the index for a mappings_t array element by name
  function mappings_index_by_name( this, name, error ) result( index )

    class(mappings_t), intent(in)  :: this
    character(len=*),  intent(in)  :: name
    type(error_t),     intent(out) :: error
    integer :: index

    character(kind=c_char, len=1), allocatable :: name_c(:)
    type(error_t_c) :: error_c

    name_c = to_c_string( name )
    index = find_mapping_index_c( this%mappings_c_, name_c, error_c ) + 1
    error = error_t( error_c )

  end function mappings_index_by_name

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Get the size of a mappings_t object
  function mappings_size( this ) result( size )

    class(mappings_t), intent(in) :: this
    integer :: size

    size = int( this%mappings_c_%size_ )

  end function mappings_size

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Finalize a mappings_t object
  elemental subroutine mappings_finalize( this )

    type(mappings_t), intent(inout) :: this

    call delete_mappings_c( this%mappings_c_ )

  end subroutine mappings_finalize

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructor for an index_mappings_t object
  function index_mappings_constructor( configuration, options, source, &
      target, error ) result( mappings )

    type(configuration_t), intent(in)  :: configuration
    integer,               intent(in)  :: options
    type(mappings_t),      intent(in)  :: source
    type(mappings_t),      intent(in)  :: target
    type(error_t),         intent(out) :: error
    type(index_mappings_t), pointer    :: mappings

    type(error_t_c) :: error_c

    allocate( mappings )
    mappings%mappings_c_ = create_index_mappings_c( &
        configuration%configuration_c_, int(options, kind=c_int), &
        source%mappings_c_, target%mappings_c_, error_c )
    error = error_t( error_c )

  end function index_mappings_constructor

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Returns the number of elements in an index_mappings_t object
  function index_mappings_size( this ) result( size )

    class(index_mappings_t), intent(in) :: this
    integer :: size

    size = get_index_mappings_size_c( this%mappings_c_ )

  end function index_mappings_size

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Copy data from a source to a target array using index mappings
  subroutine copy_data( this, source, target )

    use iso_c_binding, only: c_loc

    class(index_mappings_t),                  intent(inout) :: this
    real(kind=musica_dk), target, contiguous, intent(in)    :: source(:)
    real(kind=musica_dk), target, contiguous, intent(inout) :: target(:)

    call copy_data_c( this%mappings_c_, c_loc( source ), c_loc( target ) )

  end subroutine copy_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Finalize an index_mappings_t object
  elemental subroutine index_mappings_finalize( this )

    type(index_mappings_t), intent(inout) :: this

    call delete_index_mappings_c( this%mappings_c_ )

  end subroutine index_mappings_finalize

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

    use iso_c_binding,                 only : c_char, c_ptr, c_f_pointer, &
                                              c_associated

    type(string_t_c), intent(in) :: c_string
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

end module musica_util
