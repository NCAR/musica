
subroutine grid_map_t_constructor(this)
  implicit none
  class(grid_map_t), intent(inout) :: this

  ! Here you can initialize the `ptr` member of `this`
  ! For example:
  this%ptr = c_null_ptr
end subroutine grid_map_t_constructor

subroutine finalize(this)
  implicit none
  type(grid_map_t), intent(inout) :: this

  ! Here you can deallocate the memory pointed to by `ptr`
  ! For example:
  if (c_associated(this%ptr)) then
    call c_f_pointer(this%ptr, cptr)
    if (associated(cptr)) deallocate(cptr)
    this%ptr = c_null_ptr
  end if
end subroutine finalize