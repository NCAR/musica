! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module carma_interface

  implicit none

  private

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_get_carma_version(version_ptr, version_length) &
    bind(C, name="InternalGetCarmaVersion")
    use iso_c_binding, only: c_ptr, c_int, c_f_pointer, c_null_char, c_loc, c_char
    use musica_string, only: string_t
    use carma_version, only: get_carma_version

    ! arguments
    type(c_ptr),    intent(out) :: version_ptr
    integer(c_int), intent(out) :: version_length

    ! local variables
    character(len=:),       allocatable :: version_fortran
    character(kind=c_char), pointer     :: version_string_ptr(:)
    integer :: i

    version_fortran = get_carma_version()
    version_length = len_trim(version_fortran)

    ! Allocate and copy string
    allocate(version_string_ptr(version_length + 1))
    do i = 1, version_length
       version_string_ptr(i) = version_fortran(i:i)
    end do
    version_string_ptr(version_length + 1) = c_null_char

    version_ptr = c_loc(version_string_ptr)

  end subroutine internal_get_carma_version

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  subroutine internal_free_carma_version(version_ptr, version_length) &
    bind(C, name="InternalFreeCarmaVersion")
    use iso_c_binding, only: c_char, c_ptr, c_int, c_associated, c_f_pointer

    type(c_ptr),    value, intent(in) :: version_ptr
    integer(c_int), value, intent(in) :: version_length
    character(kind=c_char), pointer :: version_string_ptr(:)

    ! Free the allocated version string pointer
    if (c_associated(version_ptr)) then
      call c_f_pointer(version_ptr, version_string_ptr, [version_length + 1])
      deallocate(version_string_ptr)
    end if

  end subroutine internal_free_carma_version

end module carma_interface