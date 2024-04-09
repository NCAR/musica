module micm_core

   use iso_c_binding, only: c_ptr, c_char, c_int, c_double, c_null_char, c_size_t, c_f_pointer
   implicit none

   public :: micm_t, mapping_t
   private

   type, bind(c) :: mapping_t
      character(kind=c_char, len=1) :: name(256)
      integer(c_size_t) :: index
      integer(c_size_t) :: string_length
   end type mapping_t

   interface
      function create_micm_c(config_path, error_code) bind(C, name="create_micm")
         import c_ptr, c_int, c_char
         character(kind=c_char), intent(in) :: config_path(*)
         integer(kind=c_int), intent(out)   :: error_code
         type(c_ptr)                        :: create_micm_c
      end function create_micm_c

      subroutine delete_micm_c(micm) bind(C, name="delete_micm")
         import c_ptr
         type(c_ptr), intent(in) :: micm
      end subroutine delete_micm_c

      subroutine micm_solve_c(micm, time_step, temperature, pressure, num_concentrations, concentrations, &
                              num_user_defined_reaction_rates, user_defined_reaction_rates) bind(C, name="micm_solve")
         import c_ptr, c_double, c_int
         type(c_ptr), value, intent(in)         :: micm
         real(kind=c_double), value, intent(in) :: time_step
         real(kind=c_double), value, intent(in) :: temperature
         real(kind=c_double), value, intent(in) :: pressure
         integer(kind=c_int), value, intent(in) :: num_concentrations
         real(kind=c_double), intent(inout)     :: concentrations(num_concentrations)
         integer(kind=c_int), value, intent(in) :: num_user_defined_reaction_rates
         real(kind=c_double), intent(inout)     :: user_defined_reaction_rates(num_user_defined_reaction_rates)
      end subroutine micm_solve_c

      function get_species_property_string_c(micm, species_name, property_name) bind(c, name="get_species_property_string")
         import :: c_ptr, c_char
         type(c_ptr), value :: micm
         character(kind=c_char), intent(in) :: species_name(*), property_name(*)
         type(c_ptr) :: get_species_property_string_c
      end function get_species_property_string_c

      function get_species_property_double_c(micm, species_name, property_name) bind(c, name="get_species_property_double")
         import :: c_ptr, c_char
         type(c_ptr), value :: micm
         character(kind=c_char), intent(in) :: species_name(*), property_name(*)
         real(kind=c_double) :: get_species_property_double_c
      end function get_species_property_double_c

      function get_species_property_int_c(micm, species_name, property_name) bind(c, name="get_species_property_int")
         import :: c_ptr, c_char, c_int
         type(c_ptr), value :: micm
         character(kind=c_char), intent(in) :: species_name(*), property_name(*)
         integer(kind=c_int) :: get_species_property_int_c
      end function get_species_property_int_c

      function get_species_property_bool_c(micm, species_name, property_name) bind(c, name="get_species_property_bool")
         import :: c_ptr, c_char, c_logical
         type(c_ptr), value :: micm
         character(kind=c_char), intent(in) :: species_name(*), property_name(*)
         logical(kind=c_logical) :: get_species_property_bool_c
      end function get_species_property_bool_c      

      function get_species_ordering(micm, array_size) bind(c, name="get_species_ordering")
         import :: c_ptr, c_size_t
         type(c_ptr), value :: micm
         integer(kind=c_size_t), intent(out) :: array_size
         type(c_ptr)                         :: get_species_ordering
      end function get_species_ordering

      type(c_ptr) function get_user_defined_reaction_rates_ordering(micm, array_size) &
         bind(c, name="get_user_defined_reaction_rates_ordering")
         import :: c_ptr, c_size_t
         type(c_ptr), value :: micm
         integer(kind=c_size_t), intent(out) :: array_size
      end function get_user_defined_reaction_rates_ordering
   end interface

   type :: micm_t
      type(mapping_t), pointer   :: species_ordering(:), user_defined_reaction_rates(:)
      integer(kind=c_size_t) :: species_ordering_length, user_defined_reaction_rates_length
      type(c_ptr), private   :: ptr
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

   function constructor(config_path, errcode)  result( this )
      type(micm_t), pointer         :: this
      character(len=*), intent(in)  :: config_path
      integer, intent(out)          :: errcode
      character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
      integer                       :: n, i
      type(c_ptr) :: mappings_ptr

      allocate( this )

      n = len_trim(config_path)
      do i = 1, n
         c_config_path(i) = config_path(i:i)
      end do
      c_config_path(n+1) = c_null_char

      this%ptr = create_micm_c(c_config_path, errcode)

      if (errcode /= 0) then
         return
      end if

      mappings_ptr = get_species_ordering(this%ptr, this%species_ordering_length)
      call c_f_pointer(mappings_ptr, this%species_ordering, [this%species_ordering_length])


      mappings_ptr = get_user_defined_reaction_rates_ordering(this%ptr, this%user_defined_reaction_rates_length)
      call c_f_pointer(mappings_ptr, this%user_defined_reaction_rates, [this%user_defined_reaction_rates_length])
   end function constructor

   subroutine solve(this, time_step, temperature, pressure, num_concentrations, concentrations, &
                     num_user_defined_reaction_rates, user_defined_reaction_rates)
      class(micm_t)                 :: this
      real(c_double), intent(in)    :: time_step
      real(c_double), intent(in)    :: temperature
      real(c_double), intent(in)    :: pressure
      integer(c_int), intent(in)    :: num_concentrations
      real(c_double), intent(inout) :: concentrations(*)
      integer(c_int), intent(in)    :: num_user_defined_reaction_rates
      real(c_double), intent(inout) :: user_defined_reaction_rates(*)
      call micm_solve_c(this%ptr, time_step, temperature, pressure, num_concentrations, concentrations, &
                        num_user_defined_reaction_rates, user_defined_reaction_rates)
   end subroutine solve

   function get_species_property_string(this, species_name, property_name) result(value)
      class(micm_t)                 :: this
      character(len=*), intent(in)  :: species_name, property_name
      character(len=:), allocatable :: value
      type(c_ptr)                   :: c_value
      value = to_f_string(get_species_property_string_c(this%ptr, species_name, property_name))
   end function get_species_property_string

   function get_species_property_double(this, species_name, property_name) result(value)
      class(micm_t)                 :: this
      character(len=*), intent(in)  :: species_name, property_name
      real(c_double)                :: value
      value = get_species_property_double_c(this%ptr, species_name, property_name)
   end function get_species_property_double

   function get_species_property_int(this, species_name, property_name) result(value)
      class(micm_t)                 :: this
      character(len=*), intent(in)  :: species_name, property_name
      integer(c_int)                :: value
      value = get_species_property_int_c(this%ptr, species_name, property_name)
   end function get_species_property_int

   function get_species_property_bool(this, species_name, property_name) result(value)
      class(micm_t)                 :: this
      character(len=*), intent(in)  :: species_name, property_name
      logical                      :: value
      value = get_species_property_bool_c(this%ptr, species_name, property_name)
   end function get_species_property_bool

   subroutine finalize(this)
      type(micm_t), intent(inout) :: this
      call delete_micm_c(this%ptr)
   end subroutine finalize

end module micm_core
