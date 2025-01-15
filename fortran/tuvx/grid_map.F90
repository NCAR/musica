! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx_grid_map
  use iso_c_binding, only: c_ptr, c_null_ptr

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

  private
  public :: grid_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  interface
    function create_grid_map_c(error) bind(C, name="CreateGridMap")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(error_t_c), intent(inout) :: error
      type(c_ptr)                    :: create_grid_map_c
    end function create_grid_map_c

    subroutine delete_grid_map_c(grid_map, error) bind(C, name="DeleteGridMap")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: grid_map
      type(error_t_c), intent(inout) :: error
    end subroutine delete_grid_map_c

    subroutine add_grid_c(grid_map, grid, error) bind(C, name="AddGrid")
      use iso_c_binding, only: c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: grid_map
      type(c_ptr), value, intent(in) :: grid
      type(error_t_c), intent(inout) :: error
    end subroutine add_grid_c

    function get_grid_c(grid_map, grid_name, grid_units, error)                &
      bind(C, name="GetGrid")
      use musica_util, only: error_t_c
      use iso_c_binding, only: c_ptr, c_char
      type(c_ptr), value, intent(in)            :: grid_map
      character(len=1, kind=c_char), intent(in) :: grid_name(*), grid_units(*)
      type(error_t_c), intent(inout)            :: error
      type(c_ptr)                               :: get_grid_c
    end function get_grid_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  type :: grid_map_t
    type(c_ptr) :: ptr_ = c_null_ptr
  contains
    ! Adds a grid to the grid map
    procedure :: add => add_grid
    ! Get a grid given its name and units
    procedure :: get => get_grid
    ! Deallocate the grid map instance
    final :: finalize_grid_map_t
  end type grid_map_t

  interface grid_map_t
    procedure grid_map_t_ptr_constructor
    procedure grid_map_t_constructor
  end interface grid_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Wraps an existing grid map
  function grid_map_t_ptr_constructor(grid_map_c_ptr) result(this)
    ! Arguments
    type(c_ptr), intent(in) :: grid_map_c_ptr
    ! Return value
    type(grid_map_t), pointer :: this

    allocate( this )
    this%ptr_ = grid_map_c_ptr

  end function grid_map_t_ptr_constructor

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Creates a new grid map
  function grid_map_t_constructor(error) result(this)
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(error_t), intent(inout) :: error

    ! Return value
    type(grid_map_t), pointer :: this

    ! Local variables
    type(error_t_c) :: error_c

    allocate( this )
    this%ptr_ = create_grid_map_c(error_c)
    error = error_t(error_c)

  end function grid_map_t_constructor
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Adds a grid to a grid map
  subroutine add_grid(this, grid, error)
    use musica_tuvx_grid, only: grid_t
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    class(grid_map_t), intent(inout) :: this
    type(grid_t),      intent(in)    :: grid
    type(error_t),     intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c

    call add_grid_c(this%ptr_, grid%ptr_, error_c)
    error = error_t(error_c)
  
  end subroutine add_grid
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Gets a grid given its name and units
  function get_grid(this, grid_name, grid_units, error) result(grid)
    use iso_c_binding, only: c_char
    use musica_tuvx_grid, only : grid_t
    use musica_util, only: error_t, error_t_c, to_c_string

    ! Arguments
    class(grid_map_t), intent(in) :: this
    character(len=*), intent(in)  :: grid_name
    character(len=*), intent(in)  :: grid_units
    type(error_t), intent(inout)  :: error

    ! Local variables
    type(error_t_c)               :: error_c

    ! Return value
    type(grid_t), pointer :: grid

    grid => grid_t(get_grid_c(this%ptr_, to_c_string(grid_name), &
                              to_c_string(grid_units), error_c))

    error = error_t(error_c)

  end function get_grid

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Deallocates the grid map instance
  subroutine finalize_grid_map_t(this)
    use iso_c_binding, only: c_associated
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(grid_map_t), intent(inout) :: this

    ! Local variables
    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (c_associated(this%ptr_)) then
      call delete_grid_map_c(this%ptr_, error_c)
      this%ptr_ = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if

  end subroutine finalize_grid_map_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_grid_map
