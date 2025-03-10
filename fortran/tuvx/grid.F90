! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx_grid
  use iso_c_binding, only: c_ptr, c_null_ptr

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

  private
  public :: grid_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  interface
    function create_grid_c(grid_name, grid_units, number_of_sections, error) &
      bind(C, name="CreateGrid")
      use iso_c_binding, only : c_ptr, c_size_t, c_char
      use musica_util, only: error_t_c
      character(len=1, kind=c_char), intent(in) :: grid_name(*)
      character(len=1, kind=c_char), intent(in) :: grid_units(*)
      integer(c_size_t), value, intent(in) :: number_of_sections
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: create_grid_c
    end function create_grid_c

    subroutine delete_grid_c(grid, error) bind(C, name="DeleteGrid")
      use iso_c_binding, only : c_ptr
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: grid
      type(error_t_c), intent(inout) :: error
    end subroutine delete_grid_c

    function get_grid_num_sections_c(grid, error) &
      bind(C, name="GetGridNumSections")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr), value, intent(in) :: grid
      type(error_t_c), intent(inout) :: error
      integer(c_size_t) :: get_grid_num_sections_c
    end function get_grid_num_sections_c

    subroutine set_grid_edges_c(grid, edges, n_edges, error) &
      bind(C, name="SetGridEdges")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: grid
      type(c_ptr),       value, intent(in)    :: edges
      integer(c_size_t), value, intent(in)    :: n_edges
      type(error_t_c),          intent(inout) :: error
    end subroutine set_grid_edges_c

    subroutine get_grid_edges_c(grid, edges, n_edges, error) &
      bind(C, name="GetGridEdges")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: grid
      type(c_ptr),       value, intent(in)    :: edges
      integer(c_size_t), value, intent(in)    :: n_edges
      type(error_t_c),          intent(inout) :: error
    end subroutine get_grid_edges_c

    subroutine set_grid_midpoints_c(grid, midpoints, n_midpoints, error) &
      bind(C, name="SetGridMidpoints")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: grid
      type(c_ptr),       value, intent(in)    :: midpoints
      integer(c_size_t), value, intent(in)    :: n_midpoints
      type(error_t_c),          intent(inout) :: error
    end subroutine set_grid_midpoints_c

    subroutine get_grid_midpoints_c(grid, midpoints, n_midpoints, error) &
      bind(C, name="GetGridMidpoints")
      use iso_c_binding, only : c_ptr, c_size_t
      use musica_util, only: error_t_c
      type(c_ptr),       value, intent(in)    :: grid
      type(c_ptr),       value, intent(in)    :: midpoints
      integer(c_size_t), value, intent(in)    :: n_midpoints
      type(error_t_c),          intent(inout) :: error
    end subroutine get_grid_midpoints_c
  end interface

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  type :: grid_t
    type(c_ptr) :: ptr_ = c_null_ptr
  contains
    ! Returns the number of sections in the grid
    procedure :: number_of_sections
    ! Set grid edges
    procedure :: set_edges
    ! Get grid edges
    procedure :: get_edges
    ! Set the grid midpoints
    procedure :: set_midpoints
    ! Get the grid midpoints
    procedure :: get_midpoints
    ! Deallocate the grid instance
    final :: finalize_grid_t
  end type grid_t

  interface grid_t
    procedure grid_t_ptr_constructor
    procedure grid_t_constructor
  end interface grid_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Constructs a grid instance that wraps an existing TUV-x grid
  function grid_t_ptr_constructor(grid_c_ptr) result(this)
    ! Arguments
    type(c_ptr), intent(in) :: grid_c_ptr

    ! Return value
    type(grid_t), pointer :: this

    allocate( this )
    this%ptr_ = grid_c_ptr

  end function grid_t_ptr_constructor

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  !> Constructs a grid instance that allocates a new TUV-x grid
  function grid_t_constructor(grid_name, grid_units, number_of_sections, error) &
      result(this)
    use iso_c_binding, only: c_size_t
    use musica_util, only: error_t, error_t_c, to_c_string

    ! Arguments
    character(len=*), intent(in) :: grid_name
    character(len=*), intent(in) :: grid_units
    integer, intent(in) :: number_of_sections
    type(error_t), intent(inout) :: error

    ! Return value
    type(grid_t), pointer :: this

    type(error_t_c) :: error_c

    allocate( this )
    this%ptr_ = create_grid_c(to_c_string(grid_name), to_c_string(grid_units), &
                              int(number_of_sections, kind=c_size_t), error_c)
    error = error_t(error_c)

  end function grid_t_constructor
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  integer function number_of_sections(this, error) result( n_sections )
    use musica_util, only: error_t, error_t_c

    ! Arguments
    class(grid_t), intent(in)    :: this
    type(error_t), intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c

    n_sections = int( get_grid_num_sections_c(this%ptr_, error_c) )
    error = error_t(error_c)

  end function number_of_sections
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_edges(this, edges, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(grid_t),                intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: edges(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_edges

    n_edges = size(edges)

    call set_grid_edges_c(this%ptr_, c_loc(edges), n_edges, error_c)
    error = error_t(error_c)

  end subroutine set_edges

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_edges(this, edges, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(grid_t),                intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: edges(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_edges

    n_edges = size(edges)

    call get_grid_edges_c(this%ptr_, c_loc(edges), n_edges, error_c)
    error = error_t(error_c)

  end subroutine get_edges
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine set_midpoints(this, midpoints, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(grid_t),                intent(inout) :: this
    real(dk), target, contiguous, intent(in)    :: midpoints(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_midpoints

    n_midpoints = size(midpoints)

    call set_grid_midpoints_c(this%ptr_, c_loc(midpoints), n_midpoints, error_c)
    error = error_t(error_c)

  end subroutine set_midpoints

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  subroutine get_midpoints(this, midpoints, error)
    use iso_c_binding, only: c_size_t, c_loc
    use musica_util, only: error_t, error_t_c, dk => musica_dk

    ! Arguments
    class(grid_t),                intent(in)    :: this
    real(dk), target, contiguous, intent(out)   :: midpoints(:)
    type(error_t),                intent(inout) :: error

    ! Local variables
    type(error_t_c) :: error_c
    integer(kind=c_size_t) :: n_midpoints

    n_midpoints = size(midpoints)

    call get_grid_midpoints_c(this%ptr_, c_loc(midpoints), n_midpoints, error_c)
    error = error_t(error_c)

  end subroutine get_midpoints
  
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  !> Deallocate the grid instance
  subroutine finalize_grid_t(this)
    use iso_c_binding, only: c_associated
    use musica_util, only: error_t, error_t_c, assert

    ! Arguments
    type(grid_t), intent(inout) :: this

    ! Local variables
    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (c_associated(this%ptr_)) then
        call delete_grid_c(this%ptr_, error_c)
        this%ptr_ = c_null_ptr
        error = error_t(error_c)
        ASSERT(error%is_success())
    end if

  end subroutine finalize_grid_t

  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_grid