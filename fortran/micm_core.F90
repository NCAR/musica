module micm_core
   use iso_c_binding, only: c_ptr, c_char, c_int, c_bool, c_double, c_null_char, &
                            c_size_t, c_f_pointer, c_funptr, c_null_ptr, c_associated
   use musica_util, only: error_t_c
   implicit none

   public :: micm_t, mapping_t
   private

   type, bind(c) :: mapping_t
      character(kind=c_char, len=1) :: name(256)
      integer(c_size_t) :: index
      integer(c_size_t) :: string_length
   end type mapping_t

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

      function get_species_ordering(micm, array_size, error) bind(c, name="get_species_ordering")
         use musica_util, only: error_t_c
         import :: c_ptr, c_size_t
         type(c_ptr), value, intent(in) :: micm
         integer(kind=c_size_t), intent(out) :: array_size
         type(error_t_c), intent(inout) :: error
         type(c_ptr)                         :: get_species_ordering
      end function get_species_ordering

      type(c_ptr) function get_user_defined_reaction_rates_ordering(micm, array_size, error) &
         bind(c, name="get_user_defined_reaction_rates_ordering")
         use musica_util, only: error_t_c
         import :: c_ptr, c_size_t
         type(c_ptr), value, intent(in) :: micm
         integer(kind=c_size_t), intent(out) :: array_size
         type(error_t_c), intent(inout) :: error
      end function get_user_defined_reaction_rates_ordering
   end interface

   type :: micm_t
      type(mapping_t), pointer :: species_ordering(:) => null()
      type(mapping_t), pointer :: user_defined_reaction_rates(:) => null()
      integer(kind=c_size_t) :: species_ordering_length, user_defined_reaction_rates_length
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
      type(micm_t), pointer          :: this
      character(len=*), intent(in)   :: config_path
      type(error_t_c), intent(inout) :: error
      character(len=1, kind=c_char)  :: c_config_path(len_trim(config_path)+1)
      integer                        :: n, i
      type(c_ptr) :: mappings_ptr

      allocate( this )

      n = len_trim(config_path)
      do i = 1, n
         c_config_path(i) = config_path(i:i)
      end do
      c_config_path(n+1) = c_null_char

      this%ptr = create_micm_c(c_config_path, error)
      if (.not. error%code_ == 0_c_int) then
         deallocate(this)
         nullify(this)
         return
      end if

      mappings_ptr = get_species_ordering(this%ptr, this%species_ordering_length, error)
      call c_f_pointer(mappings_ptr, this%species_ordering, [this%species_ordering_length])
      if (.not. error%code_ == 0_c_int) then
         deallocate(this)
         nullify(this)
         return
      end if

      mappings_ptr = get_user_defined_reaction_rates_ordering(this%ptr, this%user_defined_reaction_rates_length, error)
      call c_f_pointer(mappings_ptr, this%user_defined_reaction_rates, [this%user_defined_reaction_rates_length])
      if (.not. error%code_ == 0_c_int) then
         deallocate(this)
         nullify(this)
         return
      end if
   end function constructor

   subroutine solve(this, time_step, temperature, pressure, num_concentrations, concentrations, &
                     num_user_defined_reaction_rates, user_defined_reaction_rates, error)
      class(micm_t)                  :: this
      real(c_double),  intent(in)    :: time_step
      real(c_double),  intent(in)    :: temperature
      real(c_double),  intent(in)    :: pressure
      integer(c_int),  intent(in)    :: num_concentrations
      real(c_double),  intent(inout) :: concentrations(*)
      integer(c_int),  intent(in)    :: num_user_defined_reaction_rates
      real(c_double),  intent(inout) :: user_defined_reaction_rates(*)
      type(error_t_c), intent(inout) :: error
      call micm_solve_c(this%ptr, time_step, temperature, pressure, num_concentrations, concentrations, &
                        num_user_defined_reaction_rates, user_defined_reaction_rates, error)
   end subroutine solve

   function get_species_property_string(this, species_name, property_name, error) result(value)
      use musica_util,                 only: to_f_string, to_c_string
      class(micm_t)                  :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t_c), intent(inout) :: error
      character(len=:), allocatable  :: value
      value = to_f_string(get_species_property_string_c(this%ptr,  &
                to_c_string(species_name), to_c_string(property_name), error))
   end function get_species_property_string

   function get_species_property_double(this, species_name, property_name, error) result(value)
      use musica_util,                 only: to_c_string
      class(micm_t)                  :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t_c), intent(inout) :: error
      real(c_double)                 :: value
      value = get_species_property_double_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error)
   end function get_species_property_double

   function get_species_property_int(this, species_name, property_name, error) result(value)
      use musica_util,                 only: to_c_string
      class(micm_t)                  :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t_c), intent(inout) :: error
      integer(c_int)                 :: value
      value = get_species_property_int_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error)
   end function get_species_property_int

   function get_species_property_bool(this, species_name, property_name, error) result(value)
      use musica_util,                 only: to_c_string
      class(micm_t)                  :: this
      character(len=*), intent(in)   :: species_name, property_name
      type(error_t_c), intent(inout) :: error
      logical                        :: value
      value = get_species_property_bool_c(this%ptr, &
                to_c_string(species_name), to_c_string(property_name), error)
   end function get_species_property_bool

   subroutine finalize(this)
      type(micm_t), intent(inout) :: this
      type(error_t_c)             :: error
      call delete_micm_c(this%ptr, error)
      this%ptr = c_null_ptr
   end subroutine finalize

end module micm_core
