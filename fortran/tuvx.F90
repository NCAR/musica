module musica_tuvx
#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
   use iso_c_binding, only: c_ptr, c_char, c_int, c_bool, c_double, c_null_char, c_size_t, c_f_pointer
   implicit none

   public :: tuvx_t
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
         type(c_ptr), intent(in)        :: tuvx
         type(error_t_c), intent(inout) :: error
      end subroutine delete_tuvx_c

      subroutine get_grid_map_c(tuvx, error) bind(C, name="get_grid_map")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), intent(in)        :: tuvx
         type(error_t_c), intent(inout) :: error
         type(c_ptr)                    :: get_grid_map_c
      end subroutine get_grid_map_c
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
      final :: finalize
   end type grid_map_t

   interface grid_map_t
      procedure grid_map_t_constructor
   end interface grid_map_t

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

contains

   include 'tuvx_grids.F90'

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

   subroutine run(this, error)
      use musica_util, only: error_t_c, error_t

      type(tuvx_t), intent(inout) :: this
      type(error_t), intent(inout)  :: error
      type(error_t_c)               :: error_c

      call run_tuvx_c(this%ptr, error_c)
      error = error_t(error_c)

   end subroutine run

   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function get_grids(this, error) result(grid_map)
      use musica_util, only: error_t_c, error_t

      type(tuvx_t), intent(inout) :: this
      type(grid_map_t) :: grid_map
      type(error_t), intent(inout)  :: error
      type(error_t_c)               :: error_c

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
