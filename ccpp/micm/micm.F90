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
        character(kind=c_char,len=:), allocatable, intent(out) :: str
        integer(c_int), intent(inout)                          :: irc
    end subroutine getAllComponentVersions

   end interface

contains

   subroutine micm_init()
    use iso_c_binding
    character(kind=c_char,len=:), allocatable :: versions
    integer(c_int) :: irc
 
    ! Call the C function
    call getAllComponentVersions(versions, irc)
 
    ! Print the result
    print *, "Result:", NEW_LINE('a'), versions
   end subroutine micm_init

end module micm
