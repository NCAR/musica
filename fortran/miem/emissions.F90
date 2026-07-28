! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_emissions
#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
  use iso_c_binding
  use iso_fortran_env, only: real64
  use musica_util, only: assert, error_t, error_t_c, mappings_t

  implicit none

  public :: mechanism_t, emissions_t
  private

  !> Fortran wrappers for the C interface to the Mechanism* handle and the
  !> EmissionsModel object
  interface
    function read_mechanism_c(config_path, error) &
        bind(C, name="ReadMechanismC")
      import c_ptr, error_t_c, c_char
      character(kind=c_char), intent(in)     :: config_path(*)
      type(error_t_c),        intent(inout)  :: error
      type(c_ptr)                            :: read_mechanism_c
    end function read_mechanism_c

    subroutine delete_mechanism_c(mechanism, error) bind(C, name="DeleteMechanism")
      import c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: mechanism
      type(error_t_c),    intent(inout) :: error
    end subroutine delete_mechanism_c

    function create_emissions_c(mechanism, n_cells, n_vert_levels, error) &
        bind(C, name="CreateEmissions")
      import c_ptr, error_t_c, c_int
      type(c_ptr),         value, intent(in)    :: mechanism
      integer(kind=c_int), value, intent(in)    :: n_cells
      integer(kind=c_int), value, intent(in)    :: n_vert_levels
      type(error_t_c),             intent(inout) :: error
      type(c_ptr)                                :: create_emissions_c
    end function create_emissions_c

    subroutine delete_emissions_c(emissions, error) bind(C, name="DeleteEmissions")
      import c_ptr, error_t_c
      type(c_ptr), value, intent(in)    :: emissions
      type(error_t_c),    intent(inout) :: error
    end subroutine delete_emissions_c

    subroutine emissions_run_c(emissions, epoch_seconds, dt_seconds, error) &
        bind(C, name="EmissionsRun")
      import c_ptr, c_double, error_t_c
      type(c_ptr),         value, intent(in)    :: emissions
      real(kind=c_double), value, intent(in)    :: epoch_seconds
      real(kind=c_double), value, intent(in)    :: dt_seconds
      type(error_t_c),             intent(inout) :: error
    end subroutine emissions_run_c

    integer(c_int) function get_num_species_c(emissions, error) &
        bind(C, name="GetNumSpecies")
      import c_ptr, c_int, error_t_c
      type(c_ptr), value, intent(in)    :: emissions
      type(error_t_c),    intent(inout) :: error
    end function get_num_species_c

    subroutine get_emissions_species_ordering_c(emissions, species_ordering, error) &
        bind(c, name="GetEmissionsSpeciesOrdering")
      use musica_util, only: error_t_c, mappings_t_c
      import c_ptr
      type(c_ptr),        value, intent(in)    :: emissions
      type(mappings_t_c),         intent(out)  :: species_ordering
      type(error_t_c),            intent(inout) :: error
    end subroutine get_emissions_species_ordering_c

    type(c_ptr) function get_surface_flux_pointer_c(emissions, array_size, error) &
        bind(C, name="GetSurfaceFluxPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr),        value, intent(in)    :: emissions
      integer(c_size_t),          intent(inout) :: array_size
      type(error_t_c),            intent(inout) :: error
    end function get_surface_flux_pointer_c

    subroutine get_surface_flux_strides_c(emissions, error, cell_stride, species_stride) &
        bind(C, name="GetSurfaceFluxStrides")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr),        value, intent(in)    :: emissions
      type(error_t_c),            intent(inout) :: error
      integer(c_size_t),          intent(inout) :: cell_stride
      integer(c_size_t),          intent(inout) :: species_stride
    end subroutine get_surface_flux_strides_c
  end interface

  !> Fortran wrapper around a parsed mechanism_configuration::Mechanism (opaque
  !> handle). Exists only so a Mechanism* can be built from a config path and
  !> handed to emissions_t's constructor, per CreateEmissions's literal C
  !> signature -- not a general-purpose mechanism API.
  type :: mechanism_t
    type(c_ptr), private :: ptr = c_null_ptr
  contains
    final :: mechanism_finalize
  end type mechanism_t

  interface mechanism_t
    procedure mechanism_constructor
  end interface mechanism_t

  type :: emissions_t
    type(c_ptr),        private :: ptr = c_null_ptr
    integer                     :: number_of_species = 0
    integer(c_size_t)           :: cell_stride = 0
    integer(c_size_t)           :: species_stride = 0
    real(kind=real64),  pointer :: surface_flux(:) => null()
    type(mappings_t),   pointer :: species_ordering => null()
  contains
    procedure :: run
    procedure :: flux
    final     :: emissions_finalize
  end type emissions_t

  interface emissions_t
    procedure emissions_constructor
  end interface emissions_t

contains

  !> Parse a Mechanism from a configuration file path
  function mechanism_constructor(config_path, error) result(this)
    character(len=*), intent(in)    :: config_path
    type(error_t),     intent(inout) :: error
    type(mechanism_t), pointer      :: this

    character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
    type(error_t_c)                :: error_c
    integer                        :: n, i

    allocate( this )

    n = len_trim(config_path)
    do i = 1, n
      c_config_path(i) = config_path(i:i)
    end do
    c_config_path(n+1) = c_null_char

    this%ptr = read_mechanism_c( c_config_path, error_c )
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
    end if
  end function mechanism_constructor

  !> Build an emissions_t from a mechanism_t's parsed Mechanism
  function emissions_constructor(mechanism, n_cells, n_vert_levels, error) result(this)
    use musica_util, only: mappings_t_c
    type(mechanism_t), intent(in)    :: mechanism
    integer,            intent(in)    :: n_cells, n_vert_levels
    type(error_t),      intent(inout) :: error
    type(emissions_t), pointer       :: this

    type(error_t_c)    :: error_c
    type(mappings_t_c) :: mapping

    allocate( this )

    this%ptr = create_emissions_c( mechanism%ptr, int(n_cells, kind=c_int), &
                                    int(n_vert_levels, kind=c_int), error_c )
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    this%number_of_species = int( get_num_species_c(this%ptr, error_c) )
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    call get_surface_flux_strides_c( this%ptr, error_c, this%cell_stride, this%species_stride )
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    call get_emissions_species_ordering_c( this%ptr, mapping, error_c )
    this%species_ordering => mappings_t( mapping )
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    ! surface_flux stays null() until the first run() -- EmissionsModel has no
    ! data before that
  end function emissions_constructor

  !> Advance one time step and refresh the surface flux buffer.
  !!
  !! EmissionsModel reassigns its internal flux buffer on every Run() call, so
  !! the pointer must be re-fetched after every run -- exactly the same hazard
  !! state_t%update_references guards against after every micm%solve().
  subroutine run(this, epoch_seconds, dt_seconds, error)
    class(emissions_t), intent(inout) :: this
    real(real64),        intent(in)    :: epoch_seconds, dt_seconds
    type(error_t),       intent(out)   :: error

    type(error_t_c)   :: error_c
    type(c_ptr)       :: temp_c_ptr
    integer(c_size_t) :: array_size_c

    call emissions_run_c( this%ptr, real(epoch_seconds, c_double), &
                          real(dt_seconds, c_double), error_c )
    error = error_t(error_c)
    if (.not. error%is_success()) return

    temp_c_ptr = get_surface_flux_pointer_c( this%ptr, array_size_c, error_c )
    call c_f_pointer( temp_c_ptr, this%surface_flux, [ array_size_c ] )
    error = error_t(error_c)
  end subroutine run

  !> Surface flux for one cell/species from the most recent run() [kg m-2 s-1].
  !! Species-major layout: cell_stride = 1, species_stride = n_cells. Only
  !! valid after at least one call to run().
  function flux(this, cell, species_name, error) result(value)
    class(emissions_t), intent(in)    :: this
    integer,             intent(in)    :: cell
    character(len=*),    intent(in)    :: species_name
    type(error_t),       intent(inout) :: error
    real(real64) :: value

    integer :: species_index

    value = 0.0_real64
    species_index = this%species_ordering%index(species_name, error)
    if (.not. error%is_success()) return
    value = this%surface_flux( 1 + (cell - 1) * int(this%cell_stride) &
                                  + (species_index - 1) * int(this%species_stride) )
  end function flux

  subroutine emissions_finalize(this)
    type(emissions_t), intent(inout) :: this

    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (associated(this%species_ordering)) then
      deallocate(this%species_ordering)
    end if

    if (c_associated(this%ptr)) then
      call delete_emissions_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if
  end subroutine emissions_finalize

  subroutine mechanism_finalize(this)
    type(mechanism_t), intent(inout) :: this

    type(error_t_c) :: error_c
    type(error_t)   :: error

    if (c_associated(this%ptr)) then
      call delete_mechanism_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if
  end subroutine mechanism_finalize

end module musica_emissions
