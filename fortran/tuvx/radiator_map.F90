! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx_radiator_map
  use iso_c_binding, only: c_ptr, c_null_ptr

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

  private
  public :: radiator_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  interface
    function create_radiator_map_c(error) bind(C, name="CreateRadiatorMap")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: create_radiator_map_c
    end function create_radiator_map_c

    subroutine delete_radiator_map_c(radiator_map, error) &
        bind(C, name="DeleteRadiatorMap")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: radiator_map
      type(error_t_c), intent(inout) :: error
    end subroutine delete_radiator_map_c

    subroutine add_radiator_c(radiator_map, radiator, error) &
        bind(C, name="AddRadiator")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: radiator_map
      type(c_ptr), value, intent(in) :: radiator
      type(error_t_c), intent(inout) :: error
    end subroutine add_radiator_c

    function get_radiator_c(radiator_map, radiator_name, error) &
        bind(C, name="GetRadiator")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr, c_char
      type(c_ptr), value, intent(in)            :: radiator_map
      character(len=1, kind=c_char), intent(in) :: radiator_name(*)
      type(error_t_c), intent(inout)            :: error
      type(c_ptr)                               :: get_radiator_c
    end function get_radiator_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  type :: radiator_map_t
    type(c_ptr) :: ptr_ = c_null_ptr
  contains
    ! Adds a radiator to the radiator map
    procedure :: add => add_radiator
    ! Get a radiator given its name
    procedure :: get => get_radiator
    ! Deallocate the radiator map instance
    final :: finalize_radiator_map_t
  end type radiator_map_t

  interface radiator_map_t
    procedure radiator_map_t_ptr_constructor
    procedure radiator_map_t_constructor
  end interface radiator_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Wraps an existing radiator map
  function radiator_map_t_ptr_constructor(radiator_map_c_ptr) result(this)
    ! Arguments
    type(c_ptr), intent(in) :: radiator_map_c_ptr
    ! Return value
    type(radiator_map_t), pointer :: this

    allocate( this )
    this%ptr_ = radiator_map_c_ptr

  end function radiator_map_t_ptr_constructor

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Creates a new radiator map
  function radiator_map_t_constructor(error) result(this)
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(error_t), intent(inout) :: error

    ! Return value
    type(radiator_map_t), pointer :: this

    ! Local variables
    type(error_t_c) :: error_c

    allocate( this )
    this%ptr_ = create_radiator_map_c(error_c)
    error = error_t(error_c)

  end function radiator_map_t_constructor
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Adds a radiator to a radiator map
  subroutine add_radiator(this, radiator, error)
    use musica_tuvx_radiator, only: radiator_t
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    class(radiator_map_t), intent(inout) :: this
    type(radiator_t), intent(in)         :: radiator
    type(error_t), intent(inout)         :: error

    ! Local variables
    type(error_t_c) :: error_c

    call add_radiator_c(this%ptr_, radiator%ptr_, error_c)
    error = error_t(error_c)
  
  end subroutine add_radiator
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Gets a radiator given its name
  function get_radiator(this, radiator_name, error) result(radiator)
    use iso_c_binding, only: c_char
    use musica_tuvx_radiator, only : radiator_t
    use musica_util, only: error_t, error_t_c, to_c_string

    ! Arguments
    class(radiator_map_t), intent(in) :: this
    character(len=*), intent(in)      :: radiator_name
    type(error_t), intent(inout)      :: error

    ! Local variables
    type(error_t_c)               :: error_c

    ! Return value
    type(radiator_t), pointer :: radiator

    radiator => radiator_t(get_radiator_c(this%ptr_, to_c_string(radiator_name), &
                           error_c))

    error = error_t(error_c)

  end function get_radiator

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Deallocates the radiator map instance
  subroutine finalize_radiator_map_t(this)
    use iso_c_binding, only: c_associated
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(radiator_map_t), intent(inout) :: this

    ! Local variables
    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (c_associated(this%ptr_)) then
      call delete_radiator_map_c(this%ptr_, error_c)
      this%ptr_ = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if

  end subroutine finalize_radiator_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_radiator_map
