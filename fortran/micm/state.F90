! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_state
  use iso_c_binding
  use iso_fortran_env, only: real64
  use musica_util, only: error_t, error_t_c, mappings_t

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

    function get_conditions_from_state_c(state) &
              bind(C, name="GetConditionsFromState")
      import c_ptr
      type(c_ptr), value :: state
      type(c_ptr)        :: get_conditions_from_state_c
    end function get_conditions_from_state_c

    subroutine set_conditions_to_state_c(state, conditions, size) & 
                bind(C, name="SetConditionsToState")
      import c_ptr, c_size_t
      type(c_ptr), value :: state
      type(c_ptr), value :: conditions
      integer(c_size_t), value :: size
    end subroutine set_conditions_to_state_c

    function get_conditions_size_c(vec) &
              bind(C, name="GetConditionsSize")
      import c_size_t, c_ptr
      type(c_ptr), value :: vec
      integer(c_size_t)  :: get_conditions_size_c
    end function get_conditions_size_c

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

  type :: state_t
    type(c_ptr) :: ptr = c_null_ptr
    type(mappings_t), pointer :: species_ordering => null()
    type(c_ptr) :: double_array_pointer_concentration = c_null_ptr
    type(c_ptr) :: double_array_pointer_rates = c_null_ptr
    real(kind=real64), pointer :: concentrations(:,:) => null()
    real(kind=real64), pointer :: rates(:,:) => null()
    type(mappings_t), pointer :: user_defined_reaction_rates => null()
  contains
    procedure :: get_conditions
    procedure :: set_conditions
    final :: finalize
  end type state_t

  type, bind(C) :: conditions_t
    real(c_double) :: temperature
    real(c_double) :: pressure
    real(c_double) :: air_density
  end type conditions_t

  interface state_t
    procedure constructor
  end interface state_t

contains

  function constructor(micm, error)  result( this )
    use iso_c_binding, only : c_f_pointer
    use musica_util, only: error_t_c, error_t, copy_mappings
    type(state_t), pointer :: this
    type(c_ptr)   ::          micm
    type(error_t) ::          error

    ! local variables
    type(error_t_c)        :: error_c
    type(c_ptr) :: double_array_pointer_concentration
    type(c_ptr) :: double_array_pointer_rates
    integer :: n_species, n_grid_cells

    allocate( this )

    this%ptr = create_state_c(micm, error_c)

    double_array_pointer_concentration = get_concentrations_pointer(this%ptr, n_species, n_grid_cells, error_c) !double*, size
    call c_f_pointer( double_array_pointer_concentration, this%concentrations, [ n_species, n_grid_cells ] )

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


  subroutine get_conditions(this, vec_ptr, size)
    class(state_t), intent(in)  :: this
    type(c_ptr),    intent(out) :: vec_ptr
    integer(c_size_t), intent(out) :: size

    vec_ptr = get_conditions_from_state_c(this%ptr)
    size = get_conditions_size_c(vec_ptr)
  end subroutine get_conditions

  subroutine set_conditions(this, vec_ptr, size)
    class(state_t), intent(inout) :: this
    type(c_ptr),    intent(in)    :: vec_ptr
    integer(c_size_t), intent(in) :: size

    call set_conditions_to_state_c(this%ptr, vec_ptr, size)
  end subroutine set_conditions

  subroutine set_ordered_concentrations(this, vec_ptr, size)
    class(state_t), intent(inout) :: this
    type(c_ptr),    intent(in)    :: vec_ptr
    integer(c_size_t), intent(in) :: size

    call set_ordered_concentrations_c(this%ptr, vec_ptr, size)
  end subroutine set_ordered_concentrations

  subroutine set_ordered_rate_constants(this, vec_ptr, size)
    class(state_t), intent(inout) :: this
    type(c_ptr),    intent(in)    :: vec_ptr
    integer(c_size_t), intent(in) :: size

    call set_ordered_rate_constants_c(this%ptr, vec_ptr, size)
  end subroutine set_ordered_rate_constants

  subroutine finalize(this)
    use musica_util, only: error_t, error_t_c
    type(state_t), intent(inout) :: this
  
    type(error_t_c) :: error_c
  
    if (associated(this%species_ordering)) then
      deallocate(this%species_ordering)
    end if
  
    if (associated(this%user_defined_reaction_rates)) then
      deallocate(this%user_defined_reaction_rates)
    end if
  
    if (c_associated(this%ptr)) then
      call delete_state_c(this%ptr, error_c)
      this%ptr = c_null_ptr
    end if
  end subroutine finalize
end module musica_state