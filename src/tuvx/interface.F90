module tuvx_interface

  use iso_c_binding,       only : c_ptr, c_loc, c_int
  use tuvx_core,           only : core_t
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

      print *, "hi"
      core => core_t(musica_config_path)
      print *, "hi2"
      internal_create_tuvx = c_loc(core)
      error_code = 0

    end function internal_create_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_delete_tuvx(tuvx, error_code) bind(C, name="internal_delete_tuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      type(c_ptr), intent(in) :: tuvx
      integer(kind=c_int), intent(out)   :: error_code

      type(core_t), pointer :: core
    
      call c_f_pointer(tuvx, core)
      if (associated(core)) then
        deallocate(core)
      end if
    end subroutine internal_delete_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_get_grid_map(tuvx, error_code) bind(C, name="internal_get_grid_map")
      use iso_c_binding, only: c_ptr, c_f_pointer

      type(c_ptr), intent(in) :: tuvx
      integer(kind=c_int), intent(out)   :: error_code

      type(core_t), pointer   :: core
      type(c_ptr)             ::grid_map

      call c_f_pointer(tuvx, core)

    end subroutine internal_get_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    subroutine internal_run_tuvx(tuvx, error_code) bind(C, name="internal_run_tuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      type(c_ptr), intent(in) :: tuvx
      integer(kind=c_int), intent(out)   :: error_code

      type(core_t), pointer :: core

      ! core%update(profiles, grids, radiators)
    
      call c_f_pointer(tuvx, core)

    end subroutine internal_run_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface
