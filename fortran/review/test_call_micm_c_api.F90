subroutine test_call_micm_c_api
    use iso_c_binding
    use micm_core, only: call_c_main
  
    implicit none
  
    call call_c_main()
end subroutine