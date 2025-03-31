! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_state
  use iso_c_binding

  implicit none

  public :: state_t
  private

  !> Fortran wrappers for the C interface to the state object
  interface
    function create_state_c(micm, error) &
        bind(C, name="CreateMicmState")
      use musica_util, only: error_t_c
      import c_ptr
      type(c_ptr), value,   intent(in)    :: micm
      type(error_t_c),      intent(inout) :: error
      type(c_ptr)                         :: create_state_c
    end function create_state_c

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

    subroutine delete_conditions_vector_c(vec) &
                bind(C, name="DeleteConditionsVector")
      import c_ptr
      type(c_ptr), value :: vec
    end subroutine delete_conditions_vector_c

    function get_conditions_data_pointer_c(vec) &
              bind(C, name="GetConditionsDataPointer")
      import c_ptr
      type(c_ptr), value :: vec
      type(c_ptr)        :: get_conditions_data_pointer_c
    end function get_conditions_data_pointer_c

    function get_conditions_size_c(vec) &
              bind(C, name="GetConditionsSize")
      import c_size_t, c_ptr
      type(c_ptr), value :: vec
      integer(c_size_t)  :: get_conditions_size_c
    end function get_conditions_size_c
  
  end interface

  type :: state_t
    type(c_ptr) :: ptr
    ! TODO: MONTEK FINISH THIS
    ! these data members likely need to be moved to here
    ! type(mappings_t), pointer :: species_ordering => null()
    ! type(mappings_t), pointer :: user_defined_reaction_rates => null()
  contains
    procedure :: get_conditions
    procedure :: set_conditions
    ! Deallocate the micm instance
    final :: finalize
  end type state_t

  type, bind(C) :: conditions_t
    real(c_double) :: temperature
    real(c_double) :: pressure
    real(c_double) :: air_density
  end type conditions_t

  interface state_t
    procedure constructor
  end interface micm_t

contains

  function constructor(micm, error)  result( this )
    use musica_util, only: error_t_c, error_t, copy_mappings
    type(c_ptr)   ::          micm
    type(error_t) ::          error

    type(state_t), pointer :: this

    ! local variables
    type(error_t_c)        :: error_c

    allocate( this )

    ! TODO: MONTEK FINISH THIS
    ! initialize all of the data members of the state_t object

    error = error_t(error_c)
    if (.not. error%is_success()) then
        deallocate(this)
        nullify(this)
        return
    end if

  end function constructor

  ! function get_conditions(temperature, ..., error) result(conditions)
    ! internally these will call one of the C functions
    ! pass the pointer of the incoming data to the C function
    ! get_conditions_from_state_c(state%ptr, temperature, size(temperature), ...)
    ! as well as the size of the data array 
    ! ... or something
    ! OR
    ! make a bunch of condition_t in an array
    ! and pass that to the C function
  ! end function get_conditions

  ! function set_conditions(conditions)
  ! end function set_conditions

  subroutine finalize(this)
    use musica_util, only: error_t, error_t_c
    type(state_t), intent(inout) :: this

    type(error_t_c)             :: error_c
    type(error_t)               :: error

    ! if (associated(this%species_ordering)) deallocate(this%species_ordering)
    ! if (associated(this%user_defined_reaction_rates)) &
    !     deallocate(this%user_defined_reaction_rates)
    ! call delete_micm_c(this%ptr, error_c)
    ! this%ptr = c_null_ptr
    ! error = error_t(error_c)
    ! ASSERT(error%is_success())
  end subroutine finalize

end module musica_state