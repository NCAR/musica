module micm
   use iso_c_binding

   implicit none

   !> Interface to c reaction functions
   interface

      function getAllComponentVersions() bind(C, name="getAllComponentVersions")
         use iso_c_binding
         type(c_ptr) :: getAllComponentVersions
      end function getAllComponentVersions

   end interface

contains

   subroutine micm_init()
      use iso_c_binding
      character(kind=c_char), dimension(:), allocatable :: versions
      type(c_ptr) :: c_ptr_versions

      ! Call the C function
      c_ptr_versions = getAllComponentVersions()

      ! Convert the C pointer to a Fortran array
      if (c_ptr_versions .ne. c_null_ptr) then
         versions = c_f_pointer(c_ptr_versions, [1])
      else
         versions = []
      end if
   end subroutine micm_init

end module micm
