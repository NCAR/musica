
subroutine grid_map_t_constructor(self)
  implicit none
  class(grid_map_t), intent(inout) :: self

  ! Here you can initialize the `ptr` member of `self`
  ! For example:
  self%ptr = c_null_ptr
end subroutine grid_map_t_constructor

subroutine finalize(self)
  implicit none
  type(grid_map_t), intent(inout) :: self

  ! Here you can deallocate the memory pointed to by `ptr`
  ! For example:
  if (c_associated(self%ptr)) then
    call c_f_pointer(self%ptr, cptr)
    if (associated(cptr)) deallocate(cptr)
    self%ptr = c_null_ptr
  end if
end subroutine finalize