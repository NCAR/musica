module micm
   use iso_c_binding

   implicit none

   !> Interface to c reaction functions
   interface

      ! Based on this: https://fortran-lang.discourse.group/t/iso-c-binding-interface-to-a-c-function-returning-a-string/527/6
      ! and this https://community.intel.com/t5/Intel-Fortran-Compiler/Calling-a-C-function-and-returning-a-string/m-p/1179874/highlight/true#M148382
      subroutine getAllComponentVersions( str, irc ) bind(C, name="c_getAllComponentVersions" )
         import :: c_char, c_int

         ! Argument list
         character(kind=c_char,len=:), pointer, intent(out) :: str
         integer(c_int), intent(inout)                      :: irc
      end subroutine getAllComponentVersions

      ! Interface to C function returning an integer
      function c_returnInteger() bind(C, name="c_returnInteger") result(num)
         use iso_c_binding
         implicit none
         integer(c_int) :: num
      end function c_returnInteger

   end interface

contains

   subroutine micm_init()
      use iso_c_binding
      character(kind=c_char,len=:), pointer :: versions
      integer(c_int) :: irc, ret

      ! Call the C function
      call getAllComponentVersions(versions, irc)

      print *, "Fortran Pointer Address:", loc(versions)

      ! Print the result
      print *, "Result:"
      print *, trim(versions)

      ret = c_returnInteger()
      print *, "Returned Integer:", ret

      ! Deallocate the allocated memory for the string
      if (associated(versions)) deallocate(versions)
   end subroutine micm_init

end module micm
