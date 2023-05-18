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
      character(len=:), pointer :: result
      type(c_ptr) :: cResult
    
      ! Call the C function
      cResult = getAllComponentVersions()
    
      ! Convert the C pointer to Fortran character array
      call c_f_pointer(cResult, result)
    
      ! Print the result
      print *, "Result:", trim(result)
    
      ! Deallocate the Fortran character array
      if (associated(result)) deallocate(result)
   end subroutine micm_init

end module micm
