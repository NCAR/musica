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
    function create_state_c(micm, error) &
        bind(C, name="CreateMicmState")
      import c_ptr, error_t_c
      type(c_ptr), value,   intent(in)    :: micm
      type(error_t_c),      intent(inout) :: error
      type(c_ptr)                         :: create_state_c
    end function create_state_c

    function get_conditions_pointer (state, number_of_grid_cells, error) &
      bind(C, name="GetConditionsToStateFortran")
      import c_int, c_ptr, error_t_c
      type(c_ptr), value,   intent(in)    :: state
      integer(c_int),       intent(out)   :: number_of_grid_cells
      type(error_t_c),      intent(inout) :: error
      type(c_ptr)                         :: get_conditions_pointer
    end function get_conditions_pointer

    function get_concentrations_pointer(state, number_of_species, number_of_grid_cells, error) & !(this is correct)
      bind(C, name="GetOrderedConcentrationsToStateFortran")
      import c_int, c_ptr, error_t_c
      type(c_ptr), value,   intent(in)    :: state
      integer(c_int),       intent(out)   :: number_of_species
      integer(c_int),       intent(out)   :: number_of_grid_cells
      type(error_t_c),      intent(inout) :: error
      type(c_ptr)                         :: get_concentrations_pointer
    end function get_concentrations_pointer

    function get_ordered_rate_constants_pointer(state, number_of_rate_constants, number_of_grid_cells, error) & !(this is correct)
      bind(C, name="GetOrderedRateConstantsToStateFortran")
      import c_int, c_ptr, error_t_c
      type(c_ptr), value,   intent(in)    :: state
      integer(c_int),       intent(out)   :: number_of_rate_constants
      integer(c_int),       intent(out)   :: number_of_grid_cells
      type(error_t_c),      intent(inout) :: error
      type(c_ptr)                         :: get_ordered_rate_constants_pointer
    end function get_ordered_rate_constants_pointer  

    subroutine delete_state_c(state, error) &
      bind(C, name="DeleteState")
      import c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: state
      type(error_t_c), intent(inout)    :: error
    end subroutine delete_state_c

    type(mappings_t_c) function get_species_ordering_c(micm, state, error) &
        bind(c, name="GetSpeciesOrdering")
      use musica_util, only: error_t_c, mappings_t_c
      import c_ptr, c_size_t
      type(c_ptr), value, intent(in)      :: micm
      type(c_ptr), value, intent(in)      :: state
      type(error_t_c), intent(inout)      :: error
    end function get_species_ordering_c

    type(mappings_t_c) function get_user_defined_reaction_rates_ordering_c(micm, state, error) &
      bind(c, name="GetUserDefinedReactionRatesOrdering")
      use musica_util, only: error_t_c, mappings_t_c
      import c_ptr, c_size_t
      type(c_ptr), value, intent(in)      :: micm
      type(c_ptr), value, intent(in)      :: state
      type(error_t_c), intent(inout)      :: error
    end function get_user_defined_reaction_rates_ordering_c    
  end interface

  type, bind(C) :: conditions_t
    real(c_double) :: temperature
    real(c_double) :: pressure
    real(c_double) :: air_density
  end type conditions_t

  type :: state_t
    type(c_ptr) :: ptr = c_null_ptr
    type(mappings_t), pointer :: species_ordering => null()
    type(c_ptr) :: array_pointer_conditions = c_null_ptr
    type(c_ptr) :: double_array_pointer_concentration = c_null_ptr
    type(c_ptr) :: double_array_pointer_rates = c_null_ptr
    type(conditions_t), pointer :: conditions(:) => null()
    real(kind=real64), pointer :: concentrations(:,:) => null()
    real(kind=real64), pointer :: rates(:,:) => null()
    type(mappings_t), pointer :: user_defined_reaction_rates => null()
  contains
    procedure :: update_references
    final :: finalize
  end type state_t

  interface state_t
    procedure constructor
  end interface state_t

contains

  function constructor(micm, error)  result( this )
    use iso_c_binding, only : c_f_pointer
    use musica_util, only: error_t_c, error_t, copy_mappings
    type(state_t), pointer :: this
    type(c_ptr)            :: micm
    type(error_t)          :: error

    ! local variables
    type(error_t_c)        :: error_c
    type(c_ptr)            :: array_pointer_conditions
    type(c_ptr)            :: double_array_pointer_concentration
    type(c_ptr)            :: double_array_pointer_rates
    integer                :: n_species, n_grid_cells

    allocate( this )

    this%ptr = create_state_c(micm, error_c)    
    error = error_t(error_c)

    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    array_pointer_conditions = get_conditions_pointer(this%ptr, n_grid_cells, error_c)
    call c_f_pointer( array_pointer_conditions, this%conditions, [ n_grid_cells ] )
    error = error_t(error_c)

    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    double_array_pointer_concentration = get_concentrations_pointer(this%ptr, n_species, n_grid_cells, error_c)
    call c_f_pointer( double_array_pointer_concentration, this%concentrations, [ n_species, n_grid_cells ] )
    error = error_t(error_c)

    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    double_array_pointer_rates = get_ordered_rate_constants_pointer(this%ptr, n_species, n_grid_cells, error_c)
    call c_f_pointer( double_array_pointer_rates, this%rates, [ n_species, n_grid_cells ] )        
    error = error_t(error_c)

    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    this%species_ordering => mappings_t( get_species_ordering_c(micm, this%ptr, error_c) )
    error = error_t(error_c)
    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

    this%user_defined_reaction_rates => &
        mappings_t( get_user_defined_reaction_rates_ordering_c(micm, this%ptr, error_c) )
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

    type(error_t_c) :: error_c
    integer         :: n_species, n_grid_cells

    this%double_array_pointer_concentration = get_concentrations_pointer(this%ptr, n_species, n_grid_cells, error_c)
    call c_f_pointer( this%double_array_pointer_concentration, this%concentrations, [ n_species, n_grid_cells ] )
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
  
    if (associated(this%user_defined_reaction_rates)) then
      deallocate(this%user_defined_reaction_rates)
    end if
  
    if (c_associated(this%ptr)) then
      call delete_state_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if
  end subroutine finalize

  subroutine what (this)
    use iso_c_binding, only : c_f_pointer
    class(state_t), intent(in) :: this

    write(*,*) "State object:"
  end subroutine what

end module musica_state