! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx
   use iso_c_binding, only: c_ptr, c_char, c_int, c_bool, c_double, c_null_char, c_size_t, c_f_pointer, c_null_ptr
   use musica_util, only: assert

   implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

   public :: tuvx_t, grid_map_t, grid_t
   private

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   interface
      function create_tuvx_c(config_path, error) bind(C, name="CreateTuvx")
         use musica_util, only: error_t_c
         import c_ptr, c_int, c_char
         character(len=1, kind=c_char), intent(in) :: config_path(*)
         type(error_t_c), intent(inout)     :: error
         type(c_ptr)                        :: create_tuvx_c
      end function create_tuvx_c

      subroutine delete_tuvx_c(tuvx, error) bind(C, name="DeleteTuvx")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), value, intent(in) :: tuvx
         type(error_t_c), intent(inout) :: error
      end subroutine delete_tuvx_c

      function get_grid_map_c(tuvx, error) bind(C, name="GetGridMap")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), value, intent(in) :: tuvx
         type(error_t_c), intent(inout) :: error
         type(c_ptr)                    :: get_grid_map_c
      end function get_grid_map_c

      function get_grid_c(grid_map, grid_name, grid_units, error) bind(C, name="GetGrid")
         use musica_util, only: error_t_c
         import c_ptr, c_char
         type(c_ptr), value, intent(in)            :: grid_map
         character(len=1, kind=c_char), intent(in) :: grid_name(*), grid_units(*)
         type(error_t_c), intent(inout)            :: error
         type(c_ptr)                               :: get_grid_c
      end function get_grid_c

      subroutine set_edges_c(grid, edges, n_edges, error) bind(C, name="SetEdges")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: grid
         real(c_double), dimension(*), intent(in) :: edges
         integer(c_size_t), value                 :: n_edges
         type(error_t_c), intent(inout)           :: error
      end subroutine set_edges_c

      subroutine set_midpoints_c(grid, midpoints, n_midpoints, error) bind(C, name="SetMidpoints")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: grid
         real(c_double), dimension(*), intent(in) :: midpoints
         integer(c_size_t), value                 :: n_midpoints
         type(error_t_c), intent(inout)           :: error
      end subroutine set_midpoints_c

   end interface

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! Data types

   type :: tuvx_t
      type(c_ptr), private :: ptr = c_null_ptr
   contains
      ! Create a grid map
      procedure :: get_grids
      ! Deallocate the tuvx instance
      final :: finalize
   end type tuvx_t

   interface tuvx_t
      procedure constructor
   end interface tuvx_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   type :: grid_map_t
      type(c_ptr) :: ptr = c_null_ptr
   contains
      procedure :: get
      ! Deallocate the tuvx instance
      final :: finalize_grid_map_t
   end type grid_map_t

   interface grid_map_t
      procedure grid_map_t_constructor
   end interface grid_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   type :: grid_t
      type(c_ptr), private :: ptr = c_null_ptr
   contains
      ! Set grid edges
      procedure :: set_edges
      ! Set the grid midpoints
      procedure :: set_midpoints
      ! Deallocate the tuvx instance
      final :: finalize_grid_t
   end type grid_t

   interface grid_t
      procedure grid_t_constructor
   end interface grid_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! Grid map type
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Construct a grid map instance
   function grid_map_t_constructor() result(this)
      ! Return value
      type(grid_map_t) :: this

      this%ptr = c_null_ptr
   end function grid_map_t_constructor

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Get a grid given its name and units
   function get(this, grid_name, grid_units, error) result(grid)
      use musica_util, only: error_t, error_t_c, to_c_string

      ! Arguments
      class(grid_map_t), intent(in) :: this
      character(len=*), intent(in)  :: grid_name
      character(len=*), intent(in)  :: grid_units
      type(error_t), intent(inout)  :: error

      ! Local variables
      type(error_t_c)               :: error_c
      character(len=1, kind=c_char) :: c_grid_name(len_trim(grid_name)+1)
      character(len=1, kind=c_char) :: c_grid_units(len_trim(grid_name)+1)

      ! Return value
      type(grid_t), pointer :: grid

      grid => grid_t()
      grid%ptr = get_grid_c(this%ptr, to_c_string(grid_name), to_c_string(grid_units), error_c)

      error = error_t(error_c)

   end function get

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Deallocate the grid map instance
   subroutine finalize_grid_map_t(this)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      type(grid_map_t), intent(inout) :: this

      ! Local variables
      type(error_t_c) :: error_c
      type(error_t)   :: error

      ! The pointer doesn't need to be deallocated because it is owned by the tuvx instance
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())

   end subroutine finalize_grid_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! Grid type
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Construct a grid map instance
   function grid_t_constructor() result(this)
      ! Return value
      type(grid_t), pointer :: this

      allocate( this )

   end function grid_t_constructor

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_edges(this, edges, error)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(grid_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: edges
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_edges

      n_edges = size(edges)

      call set_edges_c(this%ptr, edges, n_edges, error_c)
      error = error_t(error_c)

   end subroutine set_edges

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_midpoints(this, midpoints, error)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(grid_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: midpoints
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_midpoints

      n_midpoints = size(midpoints)

      call set_midpoints_c(this%ptr, midpoints, n_midpoints, error_c)
      error = error_t(error_c)

   end subroutine set_midpoints

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Deallocate the grid instance
   subroutine finalize_grid_t(this)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      type(grid_t), intent(inout) :: this

      ! Local variables
      type(error_t_c) :: error_c
      type(error_t)   :: error

      ! The pointer doesn't need to be deallocated because it is owned by the tuvx instance
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
   end subroutine finalize_grid_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! tuvx type
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Construct a tuvx instance
   function constructor(config_path, error)  result( this )
      use musica_util, only: error_t_c, error_t

      ! Arguments
      type(error_t), intent(inout)  :: error
      character(len=*), intent(in)  :: config_path

      ! Local variables
      character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
      integer                       :: n, i
      type(error_t_c)               :: error_c

      ! Return value
      type(tuvx_t), pointer         :: this

      allocate( this )

      n = len_trim(config_path)
      do i = 1, n
         c_config_path(i) = config_path(i:i)
      end do
      c_config_path(n+1) = c_null_char

      this%ptr = create_tuvx_c(c_config_path, error_c)

      error = error_t(error_c)
      if (.not. error%is_success()) then
         deallocate(this)
         nullify(this)
         return
      end if
   end function constructor

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Get the grid map
   function get_grids(this, error) result(grid_map)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(tuvx_t), intent(inout) :: this
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c)              :: error_c

      ! Return value
      type(grid_map_t)    :: grid_map

      grid_map = grid_map_t()
      grid_map%ptr = get_grid_map_c(this%ptr, error_c)
      
      error = error_t(error_c)

   end function get_grids

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Deallocate the tuvx instance
   subroutine finalize(this)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      type(tuvx_t), intent(inout) :: this

      ! Local variables
      type(error_t_c)             :: error_c
      type(error_t)               :: error

      call delete_tuvx_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())

   end subroutine finalize

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx
