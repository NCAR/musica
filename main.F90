program musica
  use musica_assert, only : assert_msg

  implicit none

  call assert_msg(234, 1 > 2, "one is not greater than 2")

  call check()
  
end program musica

subroutine check()
  write(*, *) 'We are here'
  
end subroutine check