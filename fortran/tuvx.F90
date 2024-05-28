module musica_tuvx

  use iso_c_binding, only: c_ptr, c_char, c_int, c_bool, c_double, c_null_char, c_size_t, c_f_pointer
  implicit none

  public :: tuvx_t
  private

  interface
     function create_tuvx_c(config_path, error_code) bind(C, name="create_tuvx")
        import c_ptr, c_int, c_char
        character(kind=c_char), intent(in) :: config_path(*)
        integer(kind=c_int), intent(out)   :: error_code
        type(c_ptr)                        :: create_tuvx_c
     end function create_tuvx_c

     subroutine delete_tuvx_c(tuvx) bind(C, name="delete_tuvx")
        import c_ptr
        type(c_ptr), intent(in) :: tuvx
     end subroutine delete_tuvx_c
  end interface

  ! Data types

  type :: tuvx_t
     type(c_ptr), private   :: ptr
  contains
     ! Create a grid map
     procedure :: get_grids
     ! Deallocate the tuvx instance
     final :: finalize
  end type tuvx_t

  interface tuvx_t
     procedure constructor
  end interface tuvx_t

  type :: grid_map_t
     type(c_ptr), private   :: ptr
  contains
     ! Deallocate the tuvx instance
     final :: finalize
  end type grid_map_t

  interface grid_map_t
     procedure grid_map_t_constructor
  end interface grid_map_t

contains

  include 'tuvx_grids.F90'

  function constructor(config_path, errcode)  result( this )
     type(tuvx_t), pointer         :: this
     character(len=*), intent(in)  :: config_path
     integer, intent(out)          :: errcode
     character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
     integer                       :: n, i
     type(c_ptr)                   :: mappings_ptr

     allocate( this )

     n = len_trim(config_path)
     do i = 1, n
        c_config_path(i) = config_path(i:i)
     end do
     c_config_path(n+1) = c_null_char

     this%ptr = create_tuvx_c(c_config_path, errcode)

     if (errcode /= 0) then
        return
     end if
  end function constructor

  subroutine run(this, some_input_array, more_inupt, concentrations, errcode)

     type(tuvx_t), intent(inout) :: this
     integer, intent(out)        :: errcode

     call run_tuvx_c(this%ptr, errcode)

  end subroutine run

  subroutine finalize(this)
     type(tuvx_t), intent(inout) :: this
     call delete_tuvx_c(this%ptr)
  end subroutine finalize

end module musica_tuvx
