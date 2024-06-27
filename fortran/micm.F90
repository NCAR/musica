! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_micm
#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
   use iso_c_binding, only: c_ptr, c_char, c_int, c_int64_t, c_bool, c_double, c_null_char, &
                            c_size_t, c_f_pointer, c_funptr, c_null_ptr, c_associated
   use iso_fortran_env, only: int64
   use musica_util, only: assert, mapping_t, string_t, string_t_c
   implicit none

   public :: micm_t, solver_stats_t, get_micm_version
   private

   !> Wrapper for c solver stats
   type, bind(c) :: solver_stats_t_c
      integer(c_int64_t) :: function_calls_ = 0_c_int64_t
      integer(c_int64_t) :: jacobian_updates_ = 0_c_int64_t
      integer(c_int64_t) :: number_of_steps_ = 0_c_int64_t
      integer(c_int64_t) :: accepted_ = 0_c_int64_t
      integer(c_int64_t) :: rejected_ = 0_c_int64_t
      integer(c_int64_t) :: decompositions_ = 0_c_int64_t
      integer(c_int64_t) :: solves_ = 0_c_int64_t
      integer(c_int64_t) :: singular_ = 0_c_int64_t
      real(c_double)     :: final_time_ = 0._c_double
   end type solver_stats_t_c

   interface
      function create_micm_c(config_path, error) bind(C, name="CreateMicm")
         use musica_util, only: error_t_c
         import c_ptr, c_int, c_char
         character(kind=c_char), intent(in)    :: config_path(*)
         type(error_t_c),        intent(inout) :: error
         type(c_ptr)                           :: create_micm_c
      end function create_micm_c

      subroutine delete_micm_c(micm, error) bind(C, name="DeleteMicm")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), value, intent(in)    :: micm
         type(error_t_c),    intent(inout) :: error
      end subroutine delete_micm_c

      subroutine micm_solve_c(micm, time_step, temperature, pressure, air_density, num_concentrations, concentrations, &
                  num_user_defined_reaction_rates, user_defined_reaction_rates, solver_state, solver_stats, error) &
                  bind(C, name="MicmSolve")
         use musica_util, only: string_t_c, error_t_c
         import c_ptr, c_double, c_int, solver_stats_t_c
         type(c_ptr), value, intent(in)         :: micm
         real(kind=c_double), value, intent(in) :: time_step
         real(kind=c_double), value, intent(in) :: temperature
         real(kind=c_double), value, intent(in) :: pressure
         real(kind=c_double), value, intent(in) :: air_density
         integer(kind=c_int), value, intent(in) :: num_concentrations
         real(kind=c_double), intent(inout)     :: concentrations(num_concentrations)
         integer(kind=c_int), value, intent(in) :: num_user_defined_reaction_rates
         real(kind=c_double), intent(inout)     :: user_defined_reaction_rates(num_user_defined_reaction_rates)
         type(string_t_c), intent(out)          :: solver_state
         type(solver_stats_t_c), intent(out)    :: solver_stats
         type(error_t_c), intent(inout)         :: error
      end subroutine micm_solve_c

      function get_micm_version_c() bind(C, name="MicmVersion")
         use musica_util, only: string_t_c
         type(string_t_c) :: get_micm_version_c
      end function get_micm_version_c

      function get_species_property_string_c(micm, species_name, property_name, error) bind(c, name="GetSpeciesPropertyString")
         use musica_util, only: error_t_c, string_t_c
         import c_ptr, c_char
         type(c_ptr), value, intent(in)            :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout)            :: error
         type(string_t_c)                          :: get_species_property_string_c
      end function get_species_property_string_c

      function get_species_property_double_c(micm, species_name, property_name, error) bind(c, name="GetSpeciesPropertyDouble")
         use musica_util, only: error_t_c
         import c_ptr, c_char, c_double
         type(c_ptr), value, intent(in)            :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout)            :: error
         real(kind=c_double)                       :: get_species_property_double_c
      end function get_species_property_double_c

      function get_species_property_int_c(micm, species_name, property_name, error) bind(c, name="GetSpeciesPropertyInt")
         use musica_util, only: error_t_c
         import c_ptr, c_char, c_int
         type(c_ptr), value, intent(in)            :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout)            :: error
         integer(kind=c_int)                       :: get_species_property_int_c
      end function get_species_property_int_c

      function get_species_property_bool_c(micm, species_name, property_name, error) bind(c, name="GetSpeciesPropertyBool")
         use musica_util, only: error_t_c
         import c_ptr, c_char, c_bool
         type(c_ptr), value, intent(in)            :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout)            :: error
         logical(kind=c_bool)                      :: get_species_property_bool_c
      end function get_species_property_bool_c      

      type(c_ptr) function get_species_ordering_c(micm, array_size, error) bind(c, name="GetSpeciesOrdering")
         use musica_util, only: error_t_c
         import c_ptr, c_size_t
         type(c_ptr), value, intent(in)      :: micm
         integer(kind=c_size_t), intent(out) :: array_size
         type(error_t_c), intent(inout)      :: error
      end function get_species_ordering_c

      type(c_ptr) function get_user_defined_reaction_rates_ordering_c(micm, array_size, error) &
         bind(c, name="GetUserDefinedReactionRatesOrdering")
         use musica_util, only: error_t_c
         import c_ptr, c_size_t
         type(c_ptr), value, intent(in)      :: micm
         integer(kind=c_size_t), intent(out) :: array_size
         type(error_t_c), intent(inout)      :: error
      end function get_user_defined_reaction_rates_ordering_c

      subroutine delete_mappings_c(mappings, array_size) bind(C, name="DeleteMappings")
         import c_ptr, c_size_t
         type(c_ptr), value, intent(in)            :: mappings
         integer(kind=c_size_t), value, intent(in) :: array_size
      end subroutine delete_mappings_c
   end interface

   type :: micm_t
      type(mapping_t), allocatable :: species_ordering(:)
      type(mapping_t), allocatable :: user_defined_reaction_rates(:)
      type(c_ptr), private         :: ptr = c_null_ptr
   contains
      ! Solve the chemical system
      procedure :: solve
      ! Get species properties
      procedure :: get_species_property_string
      procedure :: get_species_property_double
      procedure :: get_species_property_int
      procedure :: get_species_property_bool
      ! Deallocate the micm instance
      final :: finalize
   end type micm_t

   interface micm_t
      procedure constructor
   end interface micm_t

   !> Solver stats type
   type :: solver_stats_t
      integer(int64) :: function_calls_
      integer(int64) :: jacobian_updates_
      integer(int64) :: number_of_steps_
      integer(int64) :: accepted_
      integer(int64) :: rejected_
      integer(int64) :: decompositions_
      integer(int64) :: solves_
      integer(int64) :: singular_
      real           :: final_time_
   contains
      procedure :: function_calls => solver_stats_t_function_calls
      procedure :: jacobian_updates => solver_stats_t_jacobian_updates
      procedure :: number_of_steps => solver_stats_t_number_of_steps
      procedure :: accepted => solver_stats_t_accepted
      procedure :: rejected => solver_stats_t_rejected
      procedure :: decompositions => solver_stats_t_decompositions
      procedure :: solves => solver_stats_t_solves
      procedure :: singular => solver_stats_t_singular
      procedure :: final_time => solver_stats_t_final_time
   end type solver_stats_t

   interface solver_stats_t
      procedure solver_stats_t_constructor
   end interface solver_stats_t

contains

   function get_micm_version() result(value)
      use musica_util, only: string_t, string_t_c
      type(string_t)   :: value
      type(string_t_c) :: string_c
      string_c = get_micm_version_c()
      value = string_t(string_c)
   end function get_micm_version

   function constructor(config_path, error)  result( this )
      use musica_util, only: error_t_c, error_t, copy_mappings
      type(micm_t), pointer         :: this
      character(len=*), intent(in)  :: config_path
      type(error_t), intent(inout)  :: error
      character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
      integer                       :: n, i
      type(c_ptr)                   :: mappings_ptr
      integer(c_size_t)             :: mappings_length
      type(error_t_c)               :: error_c

      allocate( this )

      n = len_trim(config_path)
      do i = 1, n
         c_config_path(i) = config_path(i:i)
      end do
      c_config_path(n+1) = c_null_char

      this%ptr = create_micm_c(c_config_path, error_c)
      error = error_t(error_c)
      if (.not. error%is_success()) then
         deallocate(this)
         nullify(this)
         return
      end if

      mappings_ptr = get_species_ordering_c(this%ptr, mappings_length, error_c)
      error = error_t(error_c)
      if (.not. error%is_success()) then
         deallocate(this)
         nullify(this)
         return
      end if
      this%species_ordering = copy_mappings(mappings_ptr, mappings_length)
      call delete_mappings_c(mappings_ptr, mappings_length)

      mappings_ptr = get_user_defined_reaction_rates_ordering_c(this%ptr, &
                         mappings_length, error_c)
      error = error_t(error_c)
      if (.not. error%is_success()) then
         deallocate(this)
         nullify(this)
         return
      end if
      this%user_defined_reaction_rates = copy_mappings(mappings_ptr, mappings_length)
      call delete_mappings_c(mappings_ptr, mappings_length)

   end function constructor

   subroutine solve(this, time_step, temperature, pressure, air_density, num_concentrations, concentrations, &
               num_user_defined_reaction_rates, user_defined_reaction_rates, solver_state, solver_stats, error)
      use musica_util, only: string_t, string_t_c, error_t_c, error_t
      class(micm_t)                       :: this
      real(c_double),       intent(in)    :: time_step
      real(c_double),       intent(in)    :: temperature
      real(c_double),       intent(in)    :: pressure
      real(c_double),       intent(in)    :: air_density
      integer(c_int),       intent(in)    :: num_concentrations
      real(c_double),       intent(inout) :: concentrations(*)
      integer(c_int),       intent(in)    :: num_user_defined_reaction_rates
      real(c_double),       intent(inout) :: user_defined_reaction_rates(*)
      type(string_t),       intent(out)   :: solver_state
      type(solver_stats_t), intent(out)   :: solver_stats
      type(error_t),        intent(out)   :: error

      type(string_t_c)                    :: solver_state_c
      type(solver_stats_t_c)              :: solver_stats_c
      type(error_t_c)                     :: error_c

      call micm_solve_c(this%ptr, time_step, temperature, pressure, air_density, num_concentrations, concentrations, &
            num_user_defined_reaction_rates, user_defined_reaction_rates, solver_state_c, solver_stats_c, error_c)
            
      solver_state = string_t(solver_state_c)
      solver_stats = solver_stats_t(solver_stats_c)
      error = error_t(error_c)

   end subroutine solve

   !> Constructor for solver_stats_t object that takes ownership of solver_stats_t_c
   function solver_stats_t_constructor( c_solver_stats ) result( new_solver_stats )
      use iso_fortran_env, only: int64
      use musica_util, only: string_t
      type(solver_stats_t_c), intent(inout) :: c_solver_stats
      type(solver_stats_t)                  :: new_solver_stats

      new_solver_stats%function_calls_ = c_solver_stats%function_calls_
      new_solver_stats%jacobian_updates_ = c_solver_stats%jacobian_updates_
      new_solver_stats%number_of_steps_ = c_solver_stats%number_of_steps_
      new_solver_stats%accepted_ = c_solver_stats%accepted_
      new_solver_stats%rejected_ = c_solver_stats%rejected_
      new_solver_stats%decompositions_ = c_solver_stats%decompositions_
      new_solver_stats%solves_ = c_solver_stats%solves_
      new_solver_stats%singular_ = c_solver_stats%singular_
      new_solver_stats%final_time_ = real( c_solver_stats%final_time_ )

   end function solver_stats_t_constructor

   !> Get the number of forcing function calls
   function solver_stats_t_function_calls( this ) result( function_calls )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: function_calls

      function_calls = this%function_calls_

   end function solver_stats_t_function_calls

   !> Get the number of jacobian function calls
   function solver_stats_t_jacobian_updates( this ) result( jacobian_updates )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: jacobian_updates

      jacobian_updates = this%jacobian_updates_

   end function solver_stats_t_jacobian_updates

   !> Get the total number of internal time steps taken
   function solver_stats_t_number_of_steps( this ) result( number_of_steps )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: number_of_steps

      number_of_steps = this%number_of_steps_

   end function solver_stats_t_number_of_steps

   !> Get the number of accepted integrations
   function solver_stats_t_accepted( this ) result( accepted )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: accepted

      accepted = this%accepted_

   end function solver_stats_t_accepted

   !> Get the number of rejected integrations
   function solver_stats_t_rejected( this ) result( rejected )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: rejected

      rejected = this%rejected_

   end function solver_stats_t_rejected

   !> Get the number of LU decompositions
   function solver_stats_t_decompositions( this ) result( decompositions )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: decompositions

      decompositions = this%decompositions_

   end function solver_stats_t_decompositions

   !> Get the number of linear solves
   function solver_stats_t_solves( this ) result( solves )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: solves

      solves = this%solves_

   end function solver_stats_t_solves

   !> Get the number of times a singular matrix is detected
   function solver_stats_t_singular( this ) result( singular )
      use iso_fortran_env, only: int64
      class(solver_stats_t), intent(in) :: this
      integer(int64)                    :: singular

      singular = this%function_calls_

   end function solver_stats_t_singular

   !> Get the final time the solver iterated to
   function solver_stats_t_final_time( this ) result( final_time )
      class(solver_stats_t), intent(in) :: this
      real                              :: final_time

      final_time = this%final_time_

   end function solver_stats_t_final_time

   function get_species_property_string(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, string_t, string_t_c, to_c_string
      class(micm_t), intent(inout) :: this
      character(len=*), intent(in) :: species_name, property_name
      type(error_t), intent(inout) :: error
      type(string_t)               :: value

      type(error_t_c)              :: error_c
      type(string_t_c)             :: string_c
      string_c = get_species_property_string_c(this%ptr,  &
                to_c_string(species_name), to_c_string(property_name), error_c)
      value = string_t(string_c)
      error = error_t(error_c)
   end function get_species_property_string

   function get_species_property_double(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, to_c_string
      class(micm_t)                :: this
      character(len=*), intent(in) :: species_name, property_name
      type(error_t), intent(inout) :: error
      real(c_double)               :: value

      type(error_t_c)              :: error_c
      value = get_species_property_double_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error_c)
      error = error_t(error_c)
   end function get_species_property_double

   function get_species_property_int(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, to_c_string
      class(micm_t)                :: this
      character(len=*), intent(in) :: species_name, property_name
      type(error_t), intent(inout) :: error
      integer(c_int)               :: value

      type(error_t_c)              :: error_c
      value = get_species_property_int_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error_c)
      error = error_t(error_c)
   end function get_species_property_int

   function get_species_property_bool(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, to_c_string
      class(micm_t)                :: this
      character(len=*), intent(in) :: species_name, property_name
      type(error_t), intent(inout) :: error
      logical                      :: value

      type(error_t_c)              :: error_c
      value = get_species_property_bool_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error_c)
      error = error_t(error_c)
   end function get_species_property_bool

   subroutine finalize(this)
      use musica_util, only: error_t, error_t_c
      type(micm_t), intent(inout) :: this

      type(error_t_c)             :: error_c
      type(error_t)               :: error
      call delete_micm_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
   end subroutine finalize

end module musica_micm