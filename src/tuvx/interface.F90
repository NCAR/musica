module tuvx_interface

  use iso_c_binding,       only : c_ptr, c_loc, c_int
  use tuvx_core,           only : core_t
  use tuvx_grid_warehouse, only : grid_warehouse_t
  use tuvx_grid,           only : grid_t
  use musica_tuvx_util,    only : to_f_string, string_t_c
  use musica_string,       only : string_t
  use tuvx_grid_warehouse, only : grid_warehouse_t

  implicit none

  private

  contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function internal_create_tuvx(config_path, error_code) bind(C, name="internal_create_tuvx")
      type(string_t_c), value, intent(in) :: config_path
      integer(kind=c_int), intent(out)   :: error_code

      type(c_ptr)                        :: internal_create_tuvx
      type(core_t), pointer              :: core
      character(len=:), allocatable  :: f_string
      type(string_t) :: musica_config_path

      f_string = to_f_string(config_path)
      musica_config_path = string_t(f_string)

      core => core_t(musica_config_path)
      internal_create_tuvx = c_loc(core)
      error_code = 0

    end function internal_create_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_delete_tuvx(tuvx, error_code) bind(C, name="internal_delete_tuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      type(c_ptr), value, intent(in) :: tuvx
      integer(kind=c_int), intent(out)   :: error_code

      type(core_t), pointer :: core
    
      call c_f_pointer(tuvx, core)
      if (associated(core)) then
        deallocate(core)
      end if
    end subroutine internal_delete_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function internal_get_grid_map(tuvx, error_code) result(grid_map_ptr) bind(C, name="internal_get_grid_map")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    
      ! arguments
      type(c_ptr), intent(in), value   :: tuvx
      integer(kind=c_int), intent(out) :: error_code
    
      ! result
      type(c_ptr) :: grid_map_ptr
    
      ! variables
      type(core_t), pointer   :: core
      type(grid_warehouse_t), pointer :: grid_warehouse
    
      call c_f_pointer(tuvx, core)
      grid_warehouse => core%get_grid_warehouse()
    
      grid_map_ptr = c_loc(grid_warehouse)
    
    end function internal_get_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_delete_grid_map(grid_map, error_code) bind(C, name="internal_delete_grid_map")
      use iso_c_binding, only: c_ptr, c_f_pointer
    
      ! arguments
      type(c_ptr), value, intent(in) :: grid_map
      integer(kind=c_int), intent(out) :: error_code
    
      ! variables
      type(grid_warehouse_t), pointer :: grid_warehouse
    
      call c_f_pointer(grid_map, grid_warehouse)
      if (associated(grid_warehouse)) then
        deallocate(grid_warehouse)
      end if
    
    end subroutine internal_delete_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    function interal_get_grid(grid_map, grid_name, grid_units, error_code) result(grid_ptr) bind(C, name="internal_get_grid")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int
    
      ! arguments
      type(c_ptr), intent(in), value   :: grid_map
      type(string_t_c), intent(in)     :: grid_name
      type(string_t_c), intent(in)     :: grid_units
      integer(kind=c_int), intent(out) :: error_code
    
      ! result
      type(c_ptr) :: grid_ptr
    
      ! variables
      type(grid_t), pointer           :: grid
      type(grid_warehouse_t), pointer :: grid_warehouse
      character(len=:), allocatable   :: f_grid_name
      character(len=:), allocatable   :: f_grid_units
    
      f_grid_name = to_f_string(grid_name)
      f_grid_units = to_f_string(grid_units)
    
      call c_f_pointer(grid_map, grid_warehouse)
    
      grid_ptr = c_loc(grid)
    
    end function interal_get_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_delete_grid(grid, error_code) bind(C, name="internal_delete_grid")
      use iso_c_binding, only: c_ptr, c_f_pointer
    
      ! arguments
      type(c_ptr), value, intent(in) :: grid
      integer(kind=c_int), intent(out) :: error_code
    
      ! variables
      type(grid_t), pointer :: f_grid
    
      call c_f_pointer(grid, f_grid)
      if (associated(f_grid)) then
        deallocate(f_grid)
      end if
    
    end subroutine internal_delete_grid

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface
