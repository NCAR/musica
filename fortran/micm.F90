! Copyright (C) 2023-2024 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_micm
#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
   use iso_c_binding, only: c_ptr, c_char, c_int, c_bool, c_double, c_null_char, &
                            c_size_t, c_f_pointer, c_funptr, c_null_ptr, c_associated
   use musica_util, only: assert, mapping_t
   implicit none

   public :: micm_t
   private


   interface
      function create_micm_c(config_path, error) bind(C, name="create_micm")
         use musica_util, only: error_t_c
         import c_ptr, c_int, c_char
         character(kind=c_char), intent(in)    :: config_path(*)
         type(error_t_c),        intent(inout) :: error
         type(c_ptr)                           :: create_micm_c
      end function create_micm_c

      subroutine delete_micm_c(micm, error) bind(C, name="delete_micm")
         use musica_util, only: error_t_c
         import c_ptr
         type(c_ptr), value, intent(in)    :: micm
         type(error_t_c),    intent(inout) :: error
      end subroutine delete_micm_c

      subroutine micm_solve_c(micm, time_step, temperature, pressure, num_concentrations, concentrations, &
                              num_user_defined_reaction_rates, user_defined_reaction_rates, error) bind(C, name="micm_solve")
         use musica_util, only: error_t_c
         import c_ptr, c_double, c_int
         type(c_ptr), value, intent(in)         :: micm
         real(kind=c_double), value, intent(in) :: time_step
         real(kind=c_double), value, intent(in) :: temperature
         real(kind=c_double), value, intent(in) :: pressure
         integer(kind=c_int), value, intent(in) :: num_concentrations
         real(kind=c_double), intent(inout)     :: concentrations(num_concentrations)
         integer(kind=c_int), value, intent(in) :: num_user_defined_reaction_rates
         real(kind=c_double), intent(inout)     :: user_defined_reaction_rates(num_user_defined_reaction_rates)
         type(error_t_c), intent(inout)         :: error
      end subroutine micm_solve_c

      function get_species_property_string_c(micm, species_name, property_name, error) bind(c, name="get_species_property_string")
         use musica_util, only: error_t_c, string_t_c
         import :: c_ptr, c_char
         type(c_ptr), value, intent(in) :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout) :: error
         type(string_t_c) :: get_species_property_string_c
      end function get_species_property_string_c

      function get_species_property_double_c(micm, species_name, property_name, error) bind(c, name="get_species_property_double")
         use musica_util, only: error_t_c
         import :: c_ptr, c_char, c_double
         type(c_ptr), value, intent(in) :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout) :: error
         real(kind=c_double) :: get_species_property_double_c
      end function get_species_property_double_c

      function get_species_property_int_c(micm, species_name, property_name, error) bind(c, name="get_species_property_int")
         use musica_util, only: error_t_c
         import :: c_ptr, c_char, c_int
         type(c_ptr), value, intent(in) :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout) :: error
         integer(kind=c_int) :: get_species_property_int_c
      end function get_species_property_int_c

      function get_species_property_bool_c(micm, species_name, property_name, error) bind(c, name="get_species_property_bool")
         use musica_util, only: error_t_c
         import :: c_ptr, c_char, c_bool
         type(c_ptr), value, intent(in) :: micm
         character(len=1, kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(error_t_c), intent(inout) :: error
         logical(kind=c_bool) :: get_species_property_bool_c
      end function get_species_property_bool_c      

      type(c_ptr) function get_species_ordering_c(micm, array_size, error) bind(c, name="get_species_ordering")
         use musica_util, only: error_t_c
         import :: c_ptr, c_size_t
         type(c_ptr), value, intent(in) :: micm
         integer(kind=c_size_t), intent(out) :: array_size
         type(error_t_c), intent(inout) :: error
      end function get_species_ordering_c

      type(c_ptr) function get_user_defined_reaction_rates_ordering_c(micm, array_size, error) &
         bind(c, name="get_user_defined_reaction_rates_ordering")
         use musica_util, only: error_t_c
         import :: c_ptr, c_size_t
         type(c_ptr), value, intent(in) :: micm
         integer(kind=c_size_t), intent(out) :: array_size
         type(error_t_c), intent(inout) :: error
      end function get_user_defined_reaction_rates_ordering_c

      subroutine delete_mappings_c(mappings, array_size) bind(C, name="DeleteMappings")
         import c_ptr, c_size_t
         type(c_ptr), value, intent(in)    :: mappings
         integer(kind=c_size_t), value, intent(in) :: array_size
      end subroutine delete_mappings_c
   end interface

   type :: micm_t
      type(mapping_t), allocatable :: species_ordering(:)
      type(mapping_t), allocatable :: user_defined_reaction_rates(:)
      type(c_ptr), private   :: ptr = c_null_ptr
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

contains

   function constructor(config_path, error)  result( this )
      use musica_util, only: error_t_c, error_t, copy_mappings
      type(micm_t), pointer          :: this
      character(len=*), intent(in)   :: config_path
      type(error_t), intent(inout)   :: error
      character(len=1, kind=c_char)  :: c_config_path(len_trim(config_path)+1)
      integer                        :: n, i
      type(c_ptr) :: mappings_ptr
      integer(c_size_t) :: mappings_length
      type(error_t_c) :: error_c

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

   subroutine solve(this, time_step, temperature, pressure, num_concentrations, concentrations, &
                     num_user_defined_reaction_rates, user_defined_reaction_rates, error)
      use musica_util, only: error_t_c, error_t
      class(micm_t)                  :: this
      real(c_double),  intent(in)    :: time_step
      real(c_double),  intent(in)    :: temperature
      real(c_double),  intent(in)    :: pressure
      integer(c_int),  intent(in)    :: num_concentrations
      real(c_double),  intent(inout) :: concentrations(*)
      integer(c_int),  intent(in)    :: num_user_defined_reaction_rates
      real(c_double),  intent(inout) :: user_defined_reaction_rates(*)
      type(error_t),   intent(out)   :: error

      type(error_t_c) :: error_c
      call micm_solve_c(this%ptr, time_step, temperature, pressure, num_concentrations, concentrations, &
                        num_user_defined_reaction_rates, user_defined_reaction_rates, error_c)
      error = error_t(error_c)
   end subroutine solve

   function get_species_property_string(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, string_t, string_t_c, to_c_string
      class(micm_t), intent(inout)   :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t), intent(inout)   :: error
      type(string_t)                 :: value

      type(error_t_c) :: error_c
      type(string_t_c) :: string_c
      string_c = get_species_property_string_c(this%ptr,  &
                to_c_string(species_name), to_c_string(property_name), error_c)
      value = string_t(string_c)
      error = error_t(error_c)
   end function get_species_property_string

   function get_species_property_double(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, to_c_string
      class(micm_t)                  :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t), intent(inout)   :: error
      real(c_double)                 :: value

      type(error_t_c) :: error_c
      value = get_species_property_double_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error_c)
      error = error_t(error_c)
   end function get_species_property_double

   function get_species_property_int(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, to_c_string
      class(micm_t)                  :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t), intent(inout)   :: error
      integer(c_int)                 :: value

      type(error_t_c) :: error_c
      value = get_species_property_int_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error_c)
      error = error_t(error_c)
   end function get_species_property_int

   function get_species_property_bool(this, species_name, property_name, error) result(value)
      use musica_util, only: error_t_c, error_t, to_c_string
      class(micm_t)                  :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t), intent(inout)   :: error
      logical                        :: value

      type(error_t_c) :: error_c
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
