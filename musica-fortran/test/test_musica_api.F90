program test_musica_api
  use iso_c_binding
  use micm

  character(len=5, kind=c_char) :: c_filepath
  type(c_funptr)                   :: csolver_func_pointer
  c_filepath = 'asdf'
  csolver_func_pointer = get_solver(c_filepath)

end program test_musica_api
