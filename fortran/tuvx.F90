module musica_tuvx
   use iso_c_binding, only: c_ptr, c_char, c_int, c_bool, c_double, c_null_char, c_size_t, c_f_pointer, c_null_ptr
   use musica_util, only: assert

   implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )

   public :: tuvx_t, grid_map_t
   private

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   interface
      function create_tuvx_c(config_path, error) bind(C, name="create_tuvx")
         use musica_util, only: error_t_c
         import c_ptr, c_int, c_char
         character(kind=c_char), intent(in) :: config_path(*)
         type(error_t_c), intent(inout)     :: error
         type(c_ptr)                        :: create_tuvx_c
      end function create_tuvx_c

      subroutine delete_tuvx_c(tuvx, error) bind(C, name="delete_tuvx")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), value, intent(in) :: tuvx
         type(error_t_c), intent(inout) :: error
      end subroutine delete_tuvx_c

      function get_grid_map_c(tuvx, error) bind(C, name="get_grid_map")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), value, intent(in) :: tuvx
         type(error_t_c), intent(inout) :: error
         type(c_ptr)                    :: get_grid_map_c
      end function get_grid_map_c
   end interface

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! Data types

   type :: tuvx_t
      type(c_ptr), private   :: ptr
   contains
      ! Create a grid map
      procedure :: get_grids
      ! Deallocate the tuvx instance
      final :: finalize
   end type tuvx_t

   interface tuvx_t
      procedure constructor
   end interface tuvx_t

   type :: grid_map_t
      type(c_ptr), private   :: ptr
   contains
      ! Deallocate the tuvx instance
      final :: finalize_grid_map_t
   end type grid_map_t

   interface grid_map_t
      procedure grid_map_t_constructor
   end interface grid_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains


   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! Grid map type
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function grid_map_t_constructor() result(this)
      type(grid_map_t), pointer :: this

      allocate( this )

      this%ptr = c_null_ptr
   end function grid_map_t_constructor

   subroutine finalize_grid_map_t(this)
      type(grid_map_t), intent(inout) :: this

   end subroutine finalize_grid_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ! tuvx type
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function constructor(config_path, error)  result( this )
      use musica_util, only: error_t_c, error_t

      type(error_t), intent(inout)  :: error
      character(len=*), intent(in)  :: config_path
      type(tuvx_t), pointer         :: this
      character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
      integer                       :: n, i
      type(c_ptr)                   :: mappings_ptr
      type(error_t_c)               :: error_c

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

   function get_grids(this, error) result(grid_map)
      use musica_util, only: error_t, error_t_c

      class(tuvx_t), intent(inout) :: this
      type(grid_map_t) :: grid_map
      type(error_t), intent(inout)  :: error
      type(error_t_c)             :: error_c

      grid_map = grid_map_t()
      grid_map%ptr = get_grid_map_c(this%ptr, error_c)

      error = error_t(error_c)

   end function get_grids

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine finalize(this)
      use musica_util, only: error_t, error_t_c
      type(tuvx_t), intent(inout) :: this
      type(error_t_c)             :: error_c
      type(error_t)               :: error

      call delete_tuvx_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
   end subroutine finalize

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module musica_tuvx
