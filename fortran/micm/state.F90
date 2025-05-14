! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_state
#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
  use iso_c_binding
  use iso_fortran_env, only: real64
  use musica_util, only: assert, error_t, error_t_c, mappings_t

  implicit none

  public :: conditions_t, state_t
  private

  !> Fortran wrappers for the C interface to the state object
  interface
    function create_state_c(micm, num_grid_cells, error) &
        bind(C, name="CreateMicmState")
      import c_ptr, error_t_c, c_int
      type(c_ptr), value,         intent(in)    :: micm
      integer(kind=c_int), value, intent(in)    :: num_grid_cells
      type(error_t_c),            intent(inout) :: error
      type(c_ptr)                               :: create_state_c
    end function create_state_c

    type(c_ptr) function get_conditions_pointer_c(state, array_size, error) &
      bind(C, name="GetConditionsPointer")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value,   intent(in)    :: state
      integer(c_size_t),    intent(inout) :: array_size
      type(error_t_c),      intent(inout) :: error
    end function get_conditions_pointer_c

    type(c_ptr) function get_concentrations_pointer_c(state, array_size, &
        error) bind(C, name="GetOrderedConcentrationsPointer")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value,   intent(in)    :: state
      integer(c_size_t),    intent(inout) :: array_size
      type(error_t_c),      intent(inout) :: error
    end function get_concentrations_pointer_c

    type(c_ptr) function get_ordered_rate_parameters_pointer_c(state, array_size, &
        error) bind(C, name="GetOrderedRateParametersPointer")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value,   intent(in)    :: state
      integer(c_size_t),    intent(inout) :: array_size
      type(error_t_c),      intent(inout) :: error
    end function get_ordered_rate_parameters_pointer_c

    subroutine delete_state_c(state, error) &
      bind(C, name="DeleteState")
      import c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: state
      type(error_t_c), intent(inout)    :: error
    end subroutine delete_state_c

    type(mappings_t_c) function get_species_ordering_c(state, error) &
        bind(c, name="GetSpeciesOrdering")
      use musica_util, only: error_t_c, mappings_t_c
      import c_ptr
      type(c_ptr), value, intent(in)      :: state
      type(error_t_c), intent(inout)      :: error
    end function get_species_ordering_c

    type(mappings_t_c) function get_user_defined_rate_parameters_ordering_c(state, error) &
      bind(c, name="GetUserDefinedRateParametersOrdering")
      use musica_util, only: error_t_c, mappings_t_c
      import c_ptr
      type(c_ptr), value, intent(in)      :: state
      type(error_t_c), intent(inout)      :: error
    end function get_user_defined_rate_parameters_ordering_c

    integer(c_size_t) function get_number_of_grid_cells_c(state, error) &
      bind(C, name="GetNumberOfGridCells")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: state
      type(error_t_c), intent(inout)    :: error
    end function get_number_of_grid_cells_c

    integer(c_size_t) function get_number_of_species_c(state, error) &
      bind(C, name="GetNumberOfSpecies")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: state
      type(error_t_c), intent(inout)    :: error
    end function get_number_of_species_c

    subroutine get_concentrations_strides_c(state, error, grid_cell_stride, species_stride) &
      bind(C, name="GetConcentrationsStrides")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: state
      type(error_t_c), intent(inout)    :: error
      integer(c_size_t), intent(inout)  :: grid_cell_stride
      integer(c_size_t), intent(inout)  :: species_stride
    end subroutine get_concentrations_strides_c

    integer(c_size_t) function get_number_of_user_defined_rate_parameters_c(state, error) &
      bind(C, name="GetNumberOfUserDefinedRateParameters")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: state
      type(error_t_c), intent(inout)    :: error
    end function get_number_of_user_defined_rate_parameters_c

    subroutine get_user_defined_rate_parameters_strides_c(state, error, &
        grid_cell_stride, user_defined_rate_parameters_stride) &
      bind(C, name="GetUserDefinedRateParametersStrides")
      import c_size_t, c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: state
      type(error_t_c), intent(inout)    :: error
      integer(c_size_t), intent(inout)  :: grid_cell_stride
      integer(c_size_t), intent(inout)  :: user_defined_rate_parameters_stride
    end subroutine get_user_defined_rate_parameters_strides_c
  end interface

  type, bind(C) :: conditions_t
    real(c_double) :: temperature
    real(c_double) :: pressure
    real(c_double) :: air_density
  end type conditions_t

  type :: strides_t
    integer :: grid_cell
    integer :: variable
  end type strides_t

  type :: state_t
    type(c_ptr) :: ptr = c_null_ptr
    type(conditions_t), pointer :: conditions(:) => null()
    real(kind=real64),  pointer :: concentrations(:) => null()
    real(kind=real64),  pointer :: rate_parameters(:) => null()
    type(mappings_t),   pointer :: species_ordering => null()
    type(mappings_t),   pointer :: rate_parameters_ordering => null()
    integer :: number_of_grid_cells
    integer :: number_of_species
    integer :: number_of_rate_parameters
    type(strides_t) :: species_strides
    type(strides_t) :: rate_parameters_strides
  contains
    procedure :: update_references
    final :: finalize
  end type state_t

  interface state_t
    procedure constructor
  end interface state_t

contains

  function constructor(micm, number_of_grid_cells, error)  result( this )
    use iso_c_binding, only : c_f_pointer, c_int
    use musica_util, only: error_t_c, error_t, copy_mappings, mappings_t_c
    type(state_t), pointer :: this
    type(c_ptr)            :: micm
    integer                :: number_of_grid_cells
    type(error_t)          :: error

    ! local variables
    type(error_t_c)        :: error_c
    type(c_ptr)            :: temp_c_ptr
    type(mappings_t_c)     :: mapping
    integer(c_size_t)      :: array_size_c, grid_stride_c, var_stride_c

    allocate( this )

    this%ptr = create_state_c(micm, int(number_of_grid_cells, kind=c_int), error_c)    
    error = error_t(error_c)

    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    this%number_of_grid_cells = number_of_grid_cells
    this%number_of_species = get_number_of_species_c(this%ptr, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    this%number_of_rate_parameters = &
        get_number_of_user_defined_rate_parameters_c(this%ptr, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    call get_concentrations_strides_c(this%ptr, error_c, grid_stride_c, var_stride_c)
    this%species_strides%grid_cell = int(grid_stride_c)
    this%species_strides%variable  = int(var_stride_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    call get_user_defined_rate_parameters_strides_c(this%ptr, error_c, grid_stride_c, var_stride_c)
    this%rate_parameters_strides%grid_cell = int(grid_stride_c)
    this%rate_parameters_strides%variable  = int(var_stride_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    temp_c_ptr = get_conditions_pointer_c(this%ptr, array_size_c, error_c)
    call c_f_pointer( temp_c_ptr, this%conditions, [ array_size_c ] )
    error = error_t(error_c)
    temp_c_ptr = c_null_ptr

    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    temp_c_ptr = get_concentrations_pointer_c(this%ptr, array_size_c, error_c)
    call c_f_pointer( temp_c_ptr, this%concentrations, [ array_size_c ] )
    error = error_t(error_c)
    temp_c_ptr = c_null_ptr

    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    temp_c_ptr = get_ordered_rate_parameters_pointer_c(this%ptr, array_size_c, error_c)
    call c_f_pointer( temp_c_ptr, this%rate_parameters, [ array_size_c ] )        
    error = error_t(error_c)
    temp_c_ptr = c_null_ptr

    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    mapping = get_species_ordering_c(this%ptr, error_c)
    this%species_ordering => mappings_t( mapping )
    error = error_t(error_c)
    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    mapping = get_user_defined_rate_parameters_ordering_c(this%ptr, error_c) 
    this%rate_parameters_ordering => mappings_t( mapping )
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if
    
  end function constructor
  
  !> Update the references to the concentrations and rates arrays
  !> in the state object. This is necessary because C++ may swap the
  !> pointers to the arrays when the state is updated.
  subroutine update_references(this, error)
    use iso_c_binding, only : c_f_pointer
    use musica_util, only: error_t, error_t_c
    class(state_t), intent(inout) :: this
    type(error_t),  intent(out)   :: error

    type(error_t_c)   :: error_c
    integer(c_size_t) :: array_size_c
    type(c_ptr)       :: temp_c_ptr

    temp_c_ptr = get_concentrations_pointer_c(this%ptr, array_size_c, error_c)
    call c_f_pointer( temp_c_ptr, this%concentrations, [ array_size_c ] )
    error = error_t(error_c) 
  end subroutine update_references

  subroutine finalize(this)
    use musica_util, only: error_t, error_t_c
    type(state_t), intent(inout) :: this
  
    type(error_t_c) :: error_c
    type(error_t)               :: error
  
    if (associated(this%species_ordering)) then
      deallocate(this%species_ordering)
    end if
  
    if (associated(this%rate_parameters_ordering)) then
      deallocate(this%rate_parameters_ordering)
    end if
  
    if (c_associated(this%ptr)) then
      call delete_state_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if
  end subroutine finalize

  subroutine what(this, file_unit)
    use iso_c_binding, only : c_f_pointer
    class(state_t),    intent(in) :: this
    integer, optional, intent(in) :: file_unit

    integer :: f_unit

    if (present(file_unit)) then
      f_unit = file_unit
    else
      f_unit = 6
    end if
    write(f_unit,*) "State object:" 
    write(f_unit,*) "  - Number of grid cells: ", this%number_of_grid_cells
    write(f_unit,*) "  - Number of species: ", this%number_of_species
    write(f_unit,*) "  - Number of rate parameters: ", this%number_of_rate_parameters
    write(f_unit,*) "  - Species strides: ", this%species_strides%grid_cell, &
        this%species_strides%variable
    write(f_unit,*) "  - Rate parameters strides: ", this%rate_parameters_strides%grid_cell, &
        this%rate_parameters_strides%variable
    
  end subroutine what

end module musica_state