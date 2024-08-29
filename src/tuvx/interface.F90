! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface

use iso_c_binding,           only : c_ptr, c_loc, c_int, c_size_t, c_char
use tuvx_core,               only : core_t
use tuvx_grid_warehouse,     only : grid_warehouse_t
use tuvx_profile_warehouse,  only : profile_warehouse_t
use tuvx_radiator_warehouse, only : radiator_warehouse_t
use musica_tuvx_util,        only : to_f_string, string_t_c
use musica_string,           only : string_t

implicit none

private

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_create_tuvx(c_config_path, config_path_length, grids, &
      profiles, radiators, error_code) bind(C, name="InternalCreateTuvx")
    use iso_c_binding, only: c_ptr, c_f_pointer

    ! arguments
    character(kind=c_char), dimension(*), intent(in)  :: c_config_path
    integer(kind=c_size_t), value                     :: config_path_length
    integer(kind=c_int),                  intent(out) :: error_code

    ! local variables
    character(len=:), allocatable :: f_config_path
    type(c_ptr)                   :: internal_create_tuvx
    type(core_t), pointer         :: core
    type(string_t)                :: musica_config_path
    integer                       :: i

    allocate(character(len=config_path_length) :: f_config_path)
    do i = 1, config_path_length
      f_config_path(i:i) = c_config_path(i)
    end do

    musica_config_path = string_t(f_config_path)

    core => core_t(musica_config_path)

    deallocate(f_config_path)
    error_code = 0

    internal_create_tuvx = c_loc(core)

  end function internal_create_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_delete_tuvx(tuvx, error_code) &
      bind(C, name="InternalDeleteTuvx")
    use iso_c_binding, only: c_ptr, c_f_pointer

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! local variables
    type(core_t), pointer :: core
  
    call c_f_pointer(tuvx, core)
    if (associated(core)) then
      deallocate(core)
    end if
  end subroutine internal_delete_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_grid_map(tuvx, error_code) result(grid_map_ptr) &
      bind(C, name="InternalGetGridMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int
  
    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code
  
    ! result
    type(c_ptr) :: grid_map_ptr
  
    ! variables
    type(core_t),           pointer :: core
    type(grid_warehouse_t), pointer :: grid_warehouse
  
    call c_f_pointer(tuvx, core)
    grid_warehouse => core%get_grid_warehouse()
  
    grid_map_ptr = c_loc(grid_warehouse)

  end function internal_get_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_profile_map(tuvx, error_code) result(profile_map_ptr) &
      bind(C, name="InternalGetProfileMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: profile_map_ptr

    ! variables
    type(core_t),              pointer :: core
    type(profile_warehouse_t), pointer :: profile_warehouse

    call c_f_pointer(tuvx, core)
    profile_warehouse => core%get_profile_warehouse()
    
    profile_map_ptr = c_loc(profile_warehouse)

  end function internal_get_profile_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  function internal_get_radiator_map(tuvx, error_code) result(radiator_map_ptr) &
      bind(C, name="InternalGetRadiatorMap")
    use iso_c_binding, only: c_ptr, c_f_pointer, c_int

    ! arguments
    type(c_ptr), value,  intent(in)  :: tuvx
    integer(kind=c_int), intent(out) :: error_code

    ! result
    type(c_ptr) :: radiator_map_ptr

    ! variables
    type(core_t),               pointer :: core
    type(radiator_warehouse_t), pointer :: radiator_warehouse

    call c_f_pointer(tuvx, core)
    radiator_warehouse => core%get_radiator_warehouse()

    radiator_map_ptr = c_loc(radiator_warehouse)

  end function internal_get_radiator_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface