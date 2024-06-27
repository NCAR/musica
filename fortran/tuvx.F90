! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_tuvx
   use iso_c_binding, only: c_ptr, c_char, c_int, c_bool, c_double, c_null_char, c_size_t, c_f_pointer, c_null_ptr
   use musica_util, only: assert

   implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

   private
   public :: tuvx_t, grid_map_t, grid_t, profile_map_t, profile_t

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

      subroutine set_grid_edges_c(grid, edges, n_edges, error) bind(C, name="SetGridEdges")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: grid
         real(c_double), dimension(*), intent(in) :: edges
         integer(c_size_t), value                 :: n_edges
         type(error_t_c), intent(inout)           :: error
      end subroutine set_grid_edges_c

      subroutine set_grid_midpoints_c(grid, midpoints, n_midpoints, error) bind(C, name="SetGridMidpoints")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: grid
         real(c_double), dimension(*), intent(in) :: midpoints
         integer(c_size_t), value                 :: n_midpoints
         type(error_t_c), intent(inout)           :: error
      end subroutine set_grid_midpoints_c

      function get_profile_map_c(tuvx, error) bind(C, name="GetProfileMap")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), value, intent(in) :: tuvx
         type(error_t_c), intent(inout)  :: error
         type(c_ptr)                     :: get_profile_map_c
      end function get_profile_map_c

      function get_profile_c(profile_map, profile_name, profile_units, error) bind(C, name="GetProfile")
         use musica_util, only: error_t_c
         import c_ptr, c_char
         type(c_ptr), value, intent(in)            :: profile_map
         character(len=1, kind=c_char), intent(in) :: profile_name(*), profile_units(*)
         type(error_t_c), intent(inout)            :: error
         type(c_ptr)                               :: get_profile_c
      end function get_profile_c

      subroutine set_profile_edge_values_c(profile, edge_values, n_edge_values, error) bind(C, name="SetProfileEdgeValues")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: profile
         real(c_double), dimension(*), intent(in) :: edge_values
         integer(c_size_t), value                 :: n_edge_values
         type(error_t_c), intent(inout)           :: error
      end subroutine set_profile_edge_values_c

      subroutine set_profile_midpoint_values_c(profile, midpoint_values, n_midpoint_values, error) bind(C, name="SetProfileMidpointValues")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: profile
         real(c_double), dimension(*), intent(in) :: midpoint_values
         integer(c_size_t), value                 :: n_midpoint_values
         type(error_t_c), intent(inout)           :: error
      end subroutine set_profile_midpoint_values_c

      subroutine set_profile_layer_densities_c(profile, layer_densities, n_layer_densities, error) bind(C, name="SetProfileLayerDensities")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_size_t
         type(c_ptr), value, intent(in)           :: profile
         real(c_double), dimension(*), intent(in) :: layer_densities
         integer(c_size_t), value                 :: n_layer_densities
         type(error_t_c), intent(inout)           :: error
      end subroutine set_profile_layer_densities_c

      subroutine set_profile_exo_layer_density_c(profile, exo_layer_density, error) bind(C, name="SetProfileExoLayerDensity")
         use musica_util, only: error_t_c
         import c_ptr, c_double
         type(c_ptr), value, intent(in) :: profile
         real(c_double), value, intent(in) :: exo_layer_density
         type(error_t_c), intent(inout) :: error
      end subroutine set_profile_exo_layer_density_c

      subroutine calculate_profile_exo_layer_density(profile, scale_height, error) bind(C, name="CalculateProfileExoLayerDensity")
         use musica_util, only: error_t_c
         import c_ptr, c_double
         type(c_ptr), value, intent(in) :: profile
         real(c_double), value, intent(in) :: scale_height
         type(error_t_c), intent(inout) :: error
      end subroutine calculate_profile_exo_layer_density

   end interface

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! Data types

   type :: tuvx_t
      type(c_ptr), private :: ptr = c_null_ptr
   contains
      ! Create a grid map
      procedure :: get_grids
      ! Create a profile map
      procedure :: get_profiles
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
      procedure :: get => get_grid
      ! Deallocate the grid map instance
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
      ! Deallocate the grid instance
      final :: finalize_grid_t
   end type grid_t

   interface grid_t
      procedure grid_t_constructor
   end interface grid_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   
   type :: profile_map_t
      type(c_ptr) :: ptr = c_null_ptr
   contains
      procedure :: get => get_profile
      ! Deallocate the profile map instance
      final :: finalize_profile_map_t
   end type profile_map_t

   interface profile_map_t
      procedure profile_map_t_constructor
   end interface profile_map_t
   
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   type :: profile_t
      type(c_ptr), private :: ptr = c_null_ptr
   contains
      ! Set profile edge values
      procedure :: set_edge_values
      ! Set the profile midpoint values
      procedure :: set_midpoint_values
      ! Set the profile layer densities
      procedure :: set_layer_densities
      ! Set the profile exo layer density
      procedure :: set_exo_layer_density
      ! Calculate the profile exo layer density
      procedure :: calculate_exo_layer_density
   end type profile_t

   interface profile_t
      procedure profile_t_constructor
   end interface profile_t

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
   function get_grid(this, grid_name, grid_units, error) result(grid)
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

   end function get_grid

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

   !> Construct a grid instance
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

      call set_grid_edges_c(this%ptr, edges, n_edges, error_c)
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

      call set_grid_midpoints_c(this%ptr, midpoints, n_midpoints, error_c)
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
   ! Profile map type
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Construct a profile map instance
   function profile_map_t_constructor() result(this)
      ! Return value
      type(profile_map_t) :: this

      this%ptr = c_null_ptr
   end function profile_map_t_constructor

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Get a profile given its name and units
   function get_profile(this, profile_name, profile_units, error) result(profile)
      use musica_util, only: error_t, error_t_c, to_c_string

      ! Arguments
      class(profile_map_t), intent(in) :: this
      character(len=*), intent(in)  :: profile_name
      character(len=*), intent(in)  :: profile_units
      type(error_t), intent(inout)  :: error

      ! Local variables
      type(error_t_c)               :: error_c
      character(len=1, kind=c_char) :: c_profile_name(len_trim(profile_name)+1)
      character(len=1, kind=c_char) :: c_profile_units(len_trim(profile_name)+1)

      ! Return value
      type(profile_t), pointer :: profile

      profile => profile_t()
      profile%ptr = get_profile_c(this%ptr, to_c_string(profile_name), &
                                  to_c_string(profile_units), error_c)

      error = error_t(error_c)

   end function get_profile

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Deallocate the profile map instance
   subroutine finalize_profile_map_t(this)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      type(profile_map_t), intent(inout) :: this

      ! Local variables
      type(error_t_c) :: error_c
      type(error_t)   :: error

      ! The pointer doesn't need to be deallocated because it is owned by the tuvx instance
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())

   end subroutine finalize_profile_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! Profile type
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   !> Construct a profile instance
   function profile_t_constructor() result(this)
      ! Return value
      type(profile_t), pointer :: this

      allocate( this )

   end function profile_t_constructor

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_edge_values(this, edge_values, error)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: edge_values
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_edge_values

      n_edge_values = size(edge_values)

      call set_profile_edge_values_c(this%ptr, edge_values, n_edge_values, error_c)
      error = error_t(error_c)

   end subroutine set_edge_values

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_midpoint_values(this, midpoint_values, error)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: midpoint_values
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_midpoint_values

      n_midpoint_values = size(midpoint_values)

      call set_profile_midpoint_values_c(this%ptr, midpoint_values, n_midpoint_values, error_c)
      error = error_t(error_c)

   end subroutine set_midpoint_values

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_layer_densities(this, layer_densities, error)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), dimension(:), intent(in) :: layer_densities
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c
      integer(kind=c_size_t) :: n_layer_densities

      n_layer_densities = size(layer_densities)

      call set_profile_layer_densities_c(this%ptr, layer_densities, n_layer_densities, error_c)
      error = error_t(error_c)

   end subroutine set_layer_densities

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine set_exo_layer_density(this, exo_layer_density, error)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), intent(in) :: exo_layer_density
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c

      call set_profile_exo_layer_density_c(this%ptr, exo_layer_density, error_c)
      error = error_t(error_c)

   end subroutine set_exo_layer_density

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine calculate_exo_layer_density(this, scale_height, error)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(profile_t), intent(inout) :: this
      real(c_double), intent(in) :: scale_height
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c) :: error_c

      call calculate_profile_exo_layer_density(this%ptr, scale_height, error_c)
      error = error_t(error_c)

   end subroutine calculate_exo_layer_density

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

   !> Get the profile map
   function get_profiles(this, error) result(profile_map)
      use musica_util, only: error_t, error_t_c

      ! Arguments
      class(tuvx_t), intent(inout) :: this
      type(error_t), intent(inout) :: error

      ! Local variables
      type(error_t_c)              :: error_c

      ! Return value
      type(profile_map_t) :: profile_map

      profile_map = profile_map_t()
      profile_map%ptr = get_profile_map_c(this%ptr, error_c)
      
      error = error_t(error_c)

   end function get_profiles

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
