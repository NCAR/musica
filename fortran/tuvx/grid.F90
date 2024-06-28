! Copyright (C) 2023-2024 National Center for Atmospheric Research
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
      subroutine set_grid_edges_c(grid, edges, n_edges, error)                   &
          bind(C, name="SetGridEdges")
         use iso_c_binding, only : c_ptr, c_double, c_size_t
         use musica_util, only: error_t_c
         type(c_ptr), value, intent(in)           :: grid
         real(c_double), dimension(*), intent(in) :: edges
         integer(c_size_t), value                 :: n_edges
         type(error_t_c), intent(inout)           :: error
      end subroutine set_grid_edges_c

      subroutine set_grid_midpoints_c(grid, midpoints, n_midpoints, error)       &
          bind(C, name="SetGridMidpoints")
         use iso_c_binding, only : c_ptr, c_double, c_size_t
         use musica_util, only: error_t_c
         type(c_ptr), value, intent(in)           :: grid
         real(c_double), dimension(*), intent(in) :: midpoints
         integer(c_size_t), value                 :: n_midpoints
         type(error_t_c), intent(inout)           :: error
      end subroutine set_grid_midpoints_c
   end interface

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   type :: grid_t
      type(c_ptr), private :: ptr_ = c_null_ptr
   contains
      ! Set grid edges
      procedure :: set_edges
      ! Set the grid midpoints
      procedure :: set_midpoints
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
      use musica_util, only: error_t, error_t_c

      ! Arguments
      character(len=*), intent(in) :: grid_name
      character(len=*), intent(in) :: grid_units
      integer, intent(in) :: number_of_sections
      type(error_t), intent(inout) :: error

      ! Return value
      type(grid_t) :: this

      type(error_t_c) :: error_c

      ! to be implemented
      error_c%code_ = -1

   end function grid_t_constructor
   
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_edges(this, edges, error)
      use iso_c_binding, only: c_size_t, c_double
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(grid_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: edges
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_edges

      n_edges = size(edges)

      call set_grid_edges_c(this%ptr_, edges, n_edges, error_c)
      error = error_t(error_c)

   end subroutine set_edges

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_midpoints(this, midpoints, error)
      use iso_c_binding, only: c_size_t, c_double
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(grid_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: midpoints
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_midpoints

      n_midpoints = size(midpoints)

      call set_grid_midpoints_c(this%ptr_, midpoints, n_midpoints, error_c)
      error = error_t(error_c)

   end subroutine set_midpoints

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Deallocate the grid instance
   subroutine finalize_grid_t(this)
      use musica_util, only: error_t, error_t_c, assert

      ! Arguments
      type(grid_t), intent(inout) :: this

      ! Local variables
      type(error_t_c) :: error_c
      type(error_t)   :: error

      ! The pointer doesn't need to be deallocated because it is owned by the
      ! tuvx instance
      this%ptr_ = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
   end subroutine finalize_grid_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx_grid
