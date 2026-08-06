! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module musica_emissions
#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
  use iso_c_binding
  use iso_fortran_env, only: real64
  use musica_util, only: assert, error_t, error_t_c, mapping_t, mappings_t, &
    mappings_t_c, string_t_c, to_f_string, delete_string_c

  implicit none

  public :: mechanism_t, emissions_t, emissions_flux_buffer_t, &
    emissions_grid_metadata_t, &
    MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN, &
    MUSICA_EMISSIONS_GRID_GEOMETRY_PLANAR, &
    MUSICA_EMISSIONS_GRID_GEOMETRY_SPHERICAL, &
    MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL, &
    MUSICA_EMISSIONS_GRID_FIELD_LAT_CELL, &
    MUSICA_EMISSIONS_GRID_FIELD_LON_CELL, &
    MUSICA_EMISSIONS_GRID_FIELD_X_CELL, &
    MUSICA_EMISSIONS_GRID_FIELD_Y_CELL, &
    MUSICA_EMISSIONS_GRID_FIELD_Z_CELL
  private

  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN = 0_c_int
  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_GEOMETRY_PLANAR = 1_c_int
  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_GEOMETRY_SPHERICAL = 2_c_int

  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL = 0_c_int
  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_FIELD_LAT_CELL = 1_c_int
  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_FIELD_LON_CELL = 2_c_int
  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_FIELD_X_CELL = 3_c_int
  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_FIELD_Y_CELL = 4_c_int
  integer(c_int), parameter :: MUSICA_EMISSIONS_GRID_FIELD_Z_CELL = 5_c_int

  !> C-interoperable exact-grid metadata descriptor.
  type, bind(c) :: emissions_grid_metadata_t_c
    integer(c_int) :: available_ = 0_c_int
    integer(c_int) :: exact_grid_ = 0_c_int
    integer(c_int) :: global_n_cells_ = 0_c_int
    integer(c_int) :: geometry_ = MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN
    integer(c_int) :: has_sphere_radius_ = 0_c_int
    real(c_double) :: sphere_radius_ = 0.0_c_double
    integer(c_int) :: field_mask_ = 0_c_int
    type(string_t_c) :: on_a_sphere_
    type(string_t_c) :: is_periodic_
    type(string_t_c) :: fingerprint_algorithm_
    type(string_t_c) :: fingerprint_
    type(string_t_c) :: field_manifest_
    type(string_t_c) :: index_to_cell_id_units_
  end type emissions_grid_metadata_t_c

  !> Fortran view of immutable inventory metadata. Array pointers remain
  !! owned by the emissions object and are refreshed after every run().
  type :: emissions_grid_metadata_t
    logical :: available = .false.
    logical :: exact_grid = .false.
    integer :: global_number_of_cells = 0
    integer :: geometry = MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN
    logical :: has_sphere_radius = .false.
    real(real64) :: sphere_radius = 0.0_real64
    integer :: field_mask = 0
    character(len=:), allocatable :: on_a_sphere
    character(len=:), allocatable :: is_periodic
    character(len=:), allocatable :: fingerprint_algorithm
    character(len=:), allocatable :: fingerprint
    character(len=:), allocatable :: field_manifest
    character(len=:), allocatable :: index_to_cell_id_units
    integer(c_int64_t), pointer :: index_to_cell_id(:) => null()
    real(real64), pointer :: area_cell(:) => null()
    real(real64), pointer :: lat_cell(:) => null()
    real(real64), pointer :: lon_cell(:) => null()
    real(real64), pointer :: x_cell(:) => null()
    real(real64), pointer :: y_cell(:) => null()
    real(real64), pointer :: z_cell(:) => null()
    character(len=:), allocatable :: area_cell_units
    character(len=:), allocatable :: lat_cell_units
    character(len=:), allocatable :: lon_cell_units
    character(len=:), allocatable :: x_cell_units
    character(len=:), allocatable :: y_cell_units
    character(len=:), allocatable :: z_cell_units
  end type emissions_grid_metadata_t

  !> One C-owned diagnostic array exposed as a Fortran pointer.
  type :: emissions_flux_buffer_t
    real(real64), pointer :: values(:) => null()
  end type emissions_flux_buffer_t

  !> Fortran wrappers for the C interface to the Mechanism* handle and the
  !> EmissionsModel object.
  interface
    function read_mechanism_c(config_path, error) &
        bind(C, name="ReadMechanismC")
      import c_ptr, error_t_c, c_char
      character(kind=c_char), intent(in) :: config_path(*)
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: read_mechanism_c
    end function read_mechanism_c

    subroutine delete_mechanism_c(mechanism, error) bind(C, name="DeleteMechanism")
      import c_ptr, error_t_c
      type(c_ptr), value, intent(in) :: mechanism
      type(error_t_c), intent(inout) :: error
    end subroutine delete_mechanism_c

    function create_emissions_c(mechanism, n_cells, n_vert_levels, error) &
        bind(C, name="CreateEmissions")
      import c_ptr, error_t_c, c_int
      type(c_ptr), value, intent(in) :: mechanism
      integer(c_int), value, intent(in) :: n_cells
      integer(c_int), value, intent(in) :: n_vert_levels
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: create_emissions_c
    end function create_emissions_c

    function create_emissions_selected_c(mechanism, global_n_cells, n_vert_levels, &
        selected_global_cell_ids, n_selected_global_cell_ids, diagnostic_sectors, &
        diagnostic_category_ids, n_diagnostic_categories, layered_diagnostics, &
        max_diagnostic_fields, error) bind(C, name="CreateEmissionsSelected")
      import c_ptr, c_int, c_size_t, error_t_c, mappings_t_c
      type(c_ptr), value, intent(in) :: mechanism
      integer(c_int), value, intent(in) :: global_n_cells
      integer(c_int), value, intent(in) :: n_vert_levels
      integer(c_int), intent(in) :: selected_global_cell_ids(*)
      integer(c_size_t), value, intent(in) :: n_selected_global_cell_ids
      type(mappings_t_c), intent(in) :: diagnostic_sectors
      integer(c_int), intent(in) :: diagnostic_category_ids(*)
      integer(c_size_t), value, intent(in) :: n_diagnostic_categories
      integer(c_int), value, intent(in) :: layered_diagnostics
      integer(c_size_t), value, intent(in) :: max_diagnostic_fields
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: create_emissions_selected_c
    end function create_emissions_selected_c

    subroutine delete_emissions_c(emissions, error) bind(C, name="DeleteEmissions")
      import c_ptr, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(error_t_c), intent(inout) :: error
    end subroutine delete_emissions_c

    subroutine emissions_run_c(emissions, epoch_seconds, dt_seconds, error) &
        bind(C, name="EmissionsRun")
      import c_ptr, c_double, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      real(c_double), value, intent(in) :: epoch_seconds
      real(c_double), value, intent(in) :: dt_seconds
      type(error_t_c), intent(inout) :: error
    end subroutine emissions_run_c

    integer(c_int) function get_num_species_c(emissions, error) &
        bind(C, name="GetNumSpecies")
      import c_ptr, c_int, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(error_t_c), intent(inout) :: error
    end function get_num_species_c

    integer(c_int) function get_emissions_num_global_cells_c(emissions, error) &
        bind(C, name="GetEmissionsNumGlobalCells")
      import c_ptr, c_int, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(error_t_c), intent(inout) :: error
    end function get_emissions_num_global_cells_c

    integer(c_int) function get_emissions_num_cells_c(emissions, error) &
        bind(C, name="GetEmissionsNumCells")
      import c_ptr, c_int, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(error_t_c), intent(inout) :: error
    end function get_emissions_num_cells_c

    integer(c_int) function get_emissions_num_vert_levels_c(emissions, error) &
        bind(C, name="GetEmissionsNumVertLevels")
      import c_ptr, c_int, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(error_t_c), intent(inout) :: error
    end function get_emissions_num_vert_levels_c

    function get_emissions_selected_global_cell_ids_pointer_c(emissions, array_size, error) result(pointer) &
        bind(C, name="GetEmissionsSelectedGlobalCellIdsPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_emissions_selected_global_cell_ids_pointer_c

    subroutine get_emissions_species_ordering_c(emissions, species_ordering, error) &
        bind(C, name="GetEmissionsSpeciesOrdering")
      import c_ptr, error_t_c, mappings_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(mappings_t_c), intent(out) :: species_ordering
      type(error_t_c), intent(inout) :: error
    end subroutine get_emissions_species_ordering_c

    subroutine get_emissions_sector_ordering_c(emissions, sector_ordering, error) &
        bind(C, name="GetEmissionsSectorOrdering")
      import c_ptr, error_t_c, mappings_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(mappings_t_c), intent(out) :: sector_ordering
      type(error_t_c), intent(inout) :: error
    end subroutine get_emissions_sector_ordering_c

    function get_emissions_category_ids_pointer_c(emissions, array_size, error) result(pointer) &
        bind(C, name="GetEmissionsCategoryIdsPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_emissions_category_ids_pointer_c

    function get_surface_flux_pointer_c(emissions, array_size, error) result(pointer) &
        bind(C, name="GetSurfaceFluxPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_surface_flux_pointer_c

    function get_layer_flux_pointer_c(emissions, array_size, error) result(pointer) &
        bind(C, name="GetLayerFluxPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_layer_flux_pointer_c

    subroutine get_surface_flux_strides_c(emissions, cell_stride, species_stride, error) &
        bind(C, name="GetSurfaceFluxStrides")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), intent(out) :: cell_stride
      integer(c_size_t), intent(out) :: species_stride
      type(error_t_c), intent(inout) :: error
    end subroutine get_surface_flux_strides_c

    subroutine get_layer_flux_strides_c(emissions, cell_stride, level_stride, species_stride, error) &
        bind(C, name="GetLayerFluxStrides")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), intent(out) :: cell_stride
      integer(c_size_t), intent(out) :: level_stride
      integer(c_size_t), intent(out) :: species_stride
      type(error_t_c), intent(inout) :: error
    end subroutine get_layer_flux_strides_c

    function get_sector_flux_pointer_c(emissions, group_index, array_size, error) result(pointer) &
        bind(C, name="GetSectorFluxPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), value, intent(in) :: group_index
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_sector_flux_pointer_c

    function get_category_flux_pointer_c(emissions, group_index, array_size, error) result(pointer) &
        bind(C, name="GetCategoryFluxPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), value, intent(in) :: group_index
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_category_flux_pointer_c

    function get_sector_layer_flux_pointer_c(emissions, group_index, array_size, error) result(pointer) &
        bind(C, name="GetSectorLayerFluxPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), value, intent(in) :: group_index
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_sector_layer_flux_pointer_c

    function get_category_layer_flux_pointer_c(emissions, group_index, array_size, error) result(pointer) &
        bind(C, name="GetCategoryLayerFluxPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), value, intent(in) :: group_index
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_category_layer_flux_pointer_c

    subroutine get_emissions_grid_metadata_c(emissions, metadata, error) &
        bind(C, name="GetEmissionsGridMetadata")
      import c_ptr, error_t_c, emissions_grid_metadata_t_c
      type(c_ptr), value, intent(in) :: emissions
      type(emissions_grid_metadata_t_c), intent(inout) :: metadata
      type(error_t_c), intent(inout) :: error
    end subroutine get_emissions_grid_metadata_c

    subroutine delete_emissions_grid_metadata_c(metadata) &
        bind(C, name="DeleteEmissionsGridMetadata")
      import emissions_grid_metadata_t_c
      type(emissions_grid_metadata_t_c), intent(inout) :: metadata
    end subroutine delete_emissions_grid_metadata_c

    function get_emissions_grid_index_to_cell_id_pointer_c(emissions, array_size, error) result(pointer) &
        bind(C, name="GetEmissionsGridIndexToCellIdPointer")
      import c_ptr, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_emissions_grid_index_to_cell_id_pointer_c

    function get_emissions_grid_field_pointer_c(emissions, field, array_size, error) result(pointer) &
        bind(C, name="GetEmissionsGridFieldPointer")
      import c_ptr, c_int, c_size_t, error_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_int), value, intent(in) :: field
      integer(c_size_t), intent(out) :: array_size
      type(error_t_c), intent(inout) :: error
      type(c_ptr) :: pointer
    end function get_emissions_grid_field_pointer_c

    subroutine get_emissions_grid_field_units_c(emissions, field, units, error) &
        bind(C, name="GetEmissionsGridFieldUnits")
      import c_ptr, c_int, error_t_c, string_t_c
      type(c_ptr), value, intent(in) :: emissions
      integer(c_int), value, intent(in) :: field
      type(string_t_c), intent(inout) :: units
      type(error_t_c), intent(inout) :: error
    end subroutine get_emissions_grid_field_units_c
  end interface

  !> Opaque parsed mechanism_configuration::Mechanism handle.
  type :: mechanism_t
    type(c_ptr), private :: ptr = c_null_ptr
  contains
    final :: mechanism_finalize
  end type mechanism_t

  interface mechanism_t
    procedure mechanism_constructor
  end interface mechanism_t

  !> Fortran wrapper around musica::EmissionsModel.
  type :: emissions_t
    type(c_ptr), private :: ptr = c_null_ptr
    integer :: number_of_species = 0
    integer :: global_number_of_cells = 0
    integer :: number_of_cells = 0
    integer :: number_of_vertical_levels = 0
    integer(c_size_t) :: cell_stride = 0_c_size_t
    integer(c_size_t) :: species_stride = 0_c_size_t
    integer(c_size_t) :: layer_cell_stride = 0_c_size_t
    integer(c_size_t) :: layer_level_stride = 0_c_size_t
    integer(c_size_t) :: layer_species_stride = 0_c_size_t
    logical :: layered_diagnostics = .false.
    integer(c_int), pointer :: selected_global_cell_ids(:) => null()
    integer(c_int), pointer :: category_ids(:) => null()
    real(real64), pointer :: surface_flux(:) => null()
    real(real64), pointer :: layer_flux(:) => null()
    type(mappings_t), pointer :: species_ordering => null()
    type(mappings_t), pointer :: sector_ordering => null()
    type(emissions_flux_buffer_t), allocatable :: sector_flux(:)
    type(emissions_flux_buffer_t), allocatable :: category_flux(:)
    type(emissions_flux_buffer_t), allocatable :: sector_layer_flux(:)
    type(emissions_flux_buffer_t), allocatable :: category_layer_flux(:)
    type(emissions_grid_metadata_t) :: grid_metadata
  contains
    procedure :: run
    procedure :: flux
    procedure :: layer_flux_at
    final :: emissions_finalize
  end type emissions_t

  interface emissions_t
    procedure emissions_constructor
    procedure emissions_selected_constructor
  end interface emissions_t

contains

  !> Parse a Mechanism from a configuration file path.
  function mechanism_constructor(config_path, error) result(this)
    character(len=*), intent(in) :: config_path
    type(error_t), intent(inout) :: error
    type(mechanism_t), pointer :: this

    character(len=1, kind=c_char) :: c_config_path(len_trim(config_path)+1)
    type(error_t_c) :: error_c
    integer :: n, i

    allocate(this)

    n = len_trim(config_path)
    do i = 1, n
      c_config_path(i) = config_path(i:i)
    end do
    c_config_path(n+1) = c_null_char

    this%ptr = read_mechanism_c(c_config_path, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
    end if
  end function mechanism_constructor

  !> Backward-compatible full-grid emissions constructor.
  function emissions_constructor(mechanism, n_cells, n_vert_levels, error) result(this)
    type(mechanism_t), intent(in) :: mechanism
    integer, intent(in) :: n_cells, n_vert_levels
    type(error_t), intent(inout) :: error
    type(emissions_t), pointer :: this

    type(error_t_c) :: error_c

    allocate(this)
    this%ptr = create_emissions_c(mechanism%ptr, int(n_cells, c_int), &
                                  int(n_vert_levels, c_int), error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    call initialize_emissions(this, error)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
    end if
  end function emissions_constructor

  !> Rank-local constructor. selected_global_cell_ids are one-based inventory
  !! slots in the exact output order required by the host.
  function emissions_selected_constructor(mechanism, global_n_cells, n_vert_levels, &
      selected_global_cell_ids, error, diagnostic_sectors, diagnostic_categories, &
      layered_diagnostics, max_diagnostic_fields) result(this)
    type(mechanism_t), intent(in) :: mechanism
    integer, intent(in) :: global_n_cells, n_vert_levels
    integer, intent(in) :: selected_global_cell_ids(:)
    type(error_t), intent(inout) :: error
    character(len=*), intent(in), optional :: diagnostic_sectors(:)
    integer, intent(in), optional :: diagnostic_categories(:)
    logical, intent(in), optional :: layered_diagnostics
    integer, intent(in), optional :: max_diagnostic_fields
    type(emissions_t), pointer :: this

    type(error_t_c) :: error_c
    integer(c_int), allocatable :: selected_ids_c(:), categories_c(:)
    type(mapping_t), allocatable :: sector_values(:)
    type(mapping_t), pointer :: sector_value
    type(mappings_t), pointer :: sectors
    integer(c_int) :: layered_c
    integer(c_size_t) :: field_cap_c
    integer :: i, n_categories, n_sectors

    allocate(this)

    allocate(selected_ids_c(size(selected_global_cell_ids)))
    selected_ids_c = int(selected_global_cell_ids, c_int)

    n_sectors = 0
    if (present(diagnostic_sectors)) n_sectors = size(diagnostic_sectors)
    allocate(sector_values(n_sectors))
    do i = 1, n_sectors
      sector_value => mapping_t(trim(diagnostic_sectors(i)), i)
      sector_values(i) = sector_value
      deallocate(sector_value)
    end do
    sectors => mappings_t(sector_values)

    n_categories = 0
    if (present(diagnostic_categories)) n_categories = size(diagnostic_categories)
    allocate(categories_c(max(1, n_categories)))
    categories_c = 0_c_int
    if (n_categories > 0) categories_c(1:n_categories) = int(diagnostic_categories, c_int)

    layered_c = 0_c_int
    if (present(layered_diagnostics)) then
      this%layered_diagnostics = layered_diagnostics
      if (layered_diagnostics) layered_c = 1_c_int
    end if
    field_cap_c = 0_c_size_t
    if (present(max_diagnostic_fields)) field_cap_c = int(max_diagnostic_fields, c_size_t)

    this%ptr = create_emissions_selected_c(mechanism%ptr, int(global_n_cells, c_int), &
      int(n_vert_levels, c_int), selected_ids_c, int(size(selected_ids_c), c_size_t), &
      sectors%mappings_c_, categories_c, int(n_categories, c_size_t), layered_c, &
      field_cap_c, error_c)
    deallocate(sectors)

    error = error_t(error_c)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
      return
    end if

    call initialize_emissions(this, error)
    if (.not. error%is_success()) then
      deallocate(this)
      nullify(this)
    end if
  end function emissions_selected_constructor

  !> Query immutable construction-time dimensions, strides, and ordering.
  subroutine initialize_emissions(this, error)
    type(emissions_t), intent(inout) :: this
    type(error_t), intent(out) :: error

    type(error_t_c) :: error_c
    type(mappings_t_c) :: mappings_c
    type(c_ptr) :: temp_c_ptr
    integer(c_size_t) :: array_size_c

    this%number_of_species = int(get_num_species_c(this%ptr, error_c))
    error = error_t(error_c)
    if (.not. error%is_success()) return

    this%global_number_of_cells = int(get_emissions_num_global_cells_c(this%ptr, error_c))
    error = error_t(error_c)
    if (.not. error%is_success()) return

    this%number_of_cells = int(get_emissions_num_cells_c(this%ptr, error_c))
    error = error_t(error_c)
    if (.not. error%is_success()) return

    this%number_of_vertical_levels = int(get_emissions_num_vert_levels_c(this%ptr, error_c))
    error = error_t(error_c)
    if (.not. error%is_success()) return

    call get_surface_flux_strides_c(this%ptr, this%cell_stride, this%species_stride, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return

    call get_layer_flux_strides_c(this%ptr, this%layer_cell_stride, &
      this%layer_level_stride, this%layer_species_stride, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return

    call get_emissions_species_ordering_c(this%ptr, mappings_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    this%species_ordering => mappings_t(mappings_c)

    call get_emissions_sector_ordering_c(this%ptr, mappings_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    this%sector_ordering => mappings_t(mappings_c)

    temp_c_ptr = get_emissions_selected_global_cell_ids_pointer_c(this%ptr, array_size_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    if (array_size_c > 0_c_size_t) then
      call c_f_pointer(temp_c_ptr, this%selected_global_cell_ids, [ int(array_size_c) ])
    else
      nullify(this%selected_global_cell_ids)
    end if

    temp_c_ptr = get_emissions_category_ids_pointer_c(this%ptr, array_size_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    if (array_size_c > 0_c_size_t) then
      call c_f_pointer(temp_c_ptr, this%category_ids, [ int(array_size_c) ])
    else
      nullify(this%category_ids)
    end if

    allocate(this%sector_flux(this%sector_ordering%size()))
    allocate(this%category_flux(int(array_size_c)))
    allocate(this%sector_layer_flux(this%sector_ordering%size()))
    allocate(this%category_layer_flux(int(array_size_c)))
  end subroutine initialize_emissions

  !> Advance one time step and refresh every C-owned output pointer.
  subroutine run(this, epoch_seconds, dt_seconds, error)
    class(emissions_t), intent(inout) :: this
    real(real64), intent(in) :: epoch_seconds, dt_seconds
    type(error_t), intent(out) :: error

    type(error_t_c) :: error_c
    type(c_ptr) :: temp_c_ptr
    integer(c_size_t) :: array_size_c
    integer :: i

    call emissions_run_c(this%ptr, real(epoch_seconds, c_double), &
                         real(dt_seconds, c_double), error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return

    temp_c_ptr = get_surface_flux_pointer_c(this%ptr, array_size_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    call c_f_pointer(temp_c_ptr, this%surface_flux, [ int(array_size_c) ])

    temp_c_ptr = get_layer_flux_pointer_c(this%ptr, array_size_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    call c_f_pointer(temp_c_ptr, this%layer_flux, [ int(array_size_c) ])

    do i = 1, size(this%sector_flux)
      temp_c_ptr = get_sector_flux_pointer_c(this%ptr, int(i - 1, c_size_t), array_size_c, error_c)
      error = error_t(error_c)
      if (.not. error%is_success()) return
      call c_f_pointer(temp_c_ptr, this%sector_flux(i)%values, [ int(array_size_c) ])
    end do

    do i = 1, size(this%category_flux)
      temp_c_ptr = get_category_flux_pointer_c(this%ptr, int(i - 1, c_size_t), array_size_c, error_c)
      error = error_t(error_c)
      if (.not. error%is_success()) return
      call c_f_pointer(temp_c_ptr, this%category_flux(i)%values, [ int(array_size_c) ])
    end do

    if (this%layered_diagnostics) then
      do i = 1, size(this%sector_layer_flux)
        temp_c_ptr = get_sector_layer_flux_pointer_c(this%ptr, int(i - 1, c_size_t), array_size_c, error_c)
        error = error_t(error_c)
        if (.not. error%is_success()) return
        call c_f_pointer(temp_c_ptr, this%sector_layer_flux(i)%values, [ int(array_size_c) ])
      end do
      do i = 1, size(this%category_layer_flux)
        temp_c_ptr = get_category_layer_flux_pointer_c(this%ptr, int(i - 1, c_size_t), array_size_c, error_c)
        error = error_t(error_c)
        if (.not. error%is_success()) return
        call c_f_pointer(temp_c_ptr, this%category_layer_flux(i)%values, [ int(array_size_c) ])
      end do
    end if

    if (associated(this%selected_global_cell_ids)) then
      call refresh_grid_metadata(this, error)
    end if
  end subroutine run

  !> Refresh the exact-grid metadata and C-owned selected field pointers.
  subroutine refresh_grid_metadata(this, error)
    class(emissions_t), intent(inout) :: this
    type(error_t), intent(out) :: error

    type(emissions_grid_metadata_t_c) :: metadata_c
    type(error_t_c) :: error_c
    type(c_ptr) :: temp_c_ptr
    integer(c_size_t) :: array_size_c

    call get_emissions_grid_metadata_c(this%ptr, metadata_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
      call delete_emissions_grid_metadata_c(metadata_c)
      return
    end if

    this%grid_metadata%available = metadata_c%available_ /= 0_c_int
    this%grid_metadata%exact_grid = metadata_c%exact_grid_ /= 0_c_int
    this%grid_metadata%global_number_of_cells = int(metadata_c%global_n_cells_)
    this%grid_metadata%geometry = int(metadata_c%geometry_)
    this%grid_metadata%has_sphere_radius = metadata_c%has_sphere_radius_ /= 0_c_int
    this%grid_metadata%sphere_radius = real(metadata_c%sphere_radius_, real64)
    this%grid_metadata%field_mask = int(metadata_c%field_mask_)
    this%grid_metadata%on_a_sphere = to_f_string(metadata_c%on_a_sphere_)
    this%grid_metadata%is_periodic = to_f_string(metadata_c%is_periodic_)
    this%grid_metadata%fingerprint_algorithm = to_f_string(metadata_c%fingerprint_algorithm_)
    this%grid_metadata%fingerprint = to_f_string(metadata_c%fingerprint_)
    this%grid_metadata%field_manifest = to_f_string(metadata_c%field_manifest_)
    this%grid_metadata%index_to_cell_id_units = to_f_string(metadata_c%index_to_cell_id_units_)
    call delete_emissions_grid_metadata_c(metadata_c)

    temp_c_ptr = get_emissions_grid_index_to_cell_id_pointer_c(this%ptr, array_size_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    call c_f_pointer(temp_c_ptr, this%grid_metadata%index_to_cell_id, [ int(array_size_c) ])

    call reset_grid_field_pointers(this%grid_metadata)
    if (btest(this%grid_metadata%field_mask, MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL)) then
      call refresh_grid_field(this%ptr, MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL, &
        this%grid_metadata%area_cell, this%grid_metadata%area_cell_units, error)
      if (.not. error%is_success()) return
    end if
    if (btest(this%grid_metadata%field_mask, MUSICA_EMISSIONS_GRID_FIELD_LAT_CELL)) then
      call refresh_grid_field(this%ptr, MUSICA_EMISSIONS_GRID_FIELD_LAT_CELL, &
        this%grid_metadata%lat_cell, this%grid_metadata%lat_cell_units, error)
      if (.not. error%is_success()) return
    end if
    if (btest(this%grid_metadata%field_mask, MUSICA_EMISSIONS_GRID_FIELD_LON_CELL)) then
      call refresh_grid_field(this%ptr, MUSICA_EMISSIONS_GRID_FIELD_LON_CELL, &
        this%grid_metadata%lon_cell, this%grid_metadata%lon_cell_units, error)
      if (.not. error%is_success()) return
    end if
    if (btest(this%grid_metadata%field_mask, MUSICA_EMISSIONS_GRID_FIELD_X_CELL)) then
      call refresh_grid_field(this%ptr, MUSICA_EMISSIONS_GRID_FIELD_X_CELL, &
        this%grid_metadata%x_cell, this%grid_metadata%x_cell_units, error)
      if (.not. error%is_success()) return
    end if
    if (btest(this%grid_metadata%field_mask, MUSICA_EMISSIONS_GRID_FIELD_Y_CELL)) then
      call refresh_grid_field(this%ptr, MUSICA_EMISSIONS_GRID_FIELD_Y_CELL, &
        this%grid_metadata%y_cell, this%grid_metadata%y_cell_units, error)
      if (.not. error%is_success()) return
    end if
    if (btest(this%grid_metadata%field_mask, MUSICA_EMISSIONS_GRID_FIELD_Z_CELL)) then
      call refresh_grid_field(this%ptr, MUSICA_EMISSIONS_GRID_FIELD_Z_CELL, &
        this%grid_metadata%z_cell, this%grid_metadata%z_cell_units, error)
    end if
  end subroutine refresh_grid_metadata

  subroutine reset_grid_field_pointers(metadata)
    type(emissions_grid_metadata_t), intent(inout) :: metadata

    nullify(metadata%area_cell, metadata%lat_cell, metadata%lon_cell)
    nullify(metadata%x_cell, metadata%y_cell, metadata%z_cell)
    if (allocated(metadata%area_cell_units)) deallocate(metadata%area_cell_units)
    if (allocated(metadata%lat_cell_units)) deallocate(metadata%lat_cell_units)
    if (allocated(metadata%lon_cell_units)) deallocate(metadata%lon_cell_units)
    if (allocated(metadata%x_cell_units)) deallocate(metadata%x_cell_units)
    if (allocated(metadata%y_cell_units)) deallocate(metadata%y_cell_units)
    if (allocated(metadata%z_cell_units)) deallocate(metadata%z_cell_units)
  end subroutine reset_grid_field_pointers

  subroutine refresh_grid_field(emissions_ptr, field, values, units, error)
    type(c_ptr), intent(in) :: emissions_ptr
    integer(c_int), intent(in) :: field
    real(real64), pointer, intent(inout) :: values(:)
    character(len=:), allocatable, intent(inout) :: units
    type(error_t), intent(out) :: error

    type(error_t_c) :: error_c
    type(string_t_c) :: units_c
    type(c_ptr) :: temp_c_ptr
    integer(c_size_t) :: array_size_c

    temp_c_ptr = get_emissions_grid_field_pointer_c(emissions_ptr, field, array_size_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) return
    call c_f_pointer(temp_c_ptr, values, [ int(array_size_c) ])

    call get_emissions_grid_field_units_c(emissions_ptr, field, units_c, error_c)
    error = error_t(error_c)
    if (.not. error%is_success()) then
      call delete_string_c(units_c)
      return
    end if
    units = to_f_string(units_c)
    call delete_string_c(units_c)
  end subroutine refresh_grid_field

  !> Surface flux for one selected cell/species from the most recent run().
  function flux(this, cell, species_name, error) result(value)
    class(emissions_t), intent(in) :: this
    integer, intent(in) :: cell
    character(len=*), intent(in) :: species_name
    type(error_t), intent(inout) :: error
    real(real64) :: value

    integer :: species_index

    value = 0.0_real64
    species_index = this%species_ordering%index(species_name, error)
    if (.not. error%is_success()) return
    value = this%surface_flux(1 + (cell - 1) * int(this%cell_stride) + &
      (species_index - 1) * int(this%species_stride))
  end function flux

  !> Layer flux for one selected cell/level/species from the most recent run().
  function layer_flux_at(this, cell, level, species_name, error) result(value)
    class(emissions_t), intent(in) :: this
    integer, intent(in) :: cell, level
    character(len=*), intent(in) :: species_name
    type(error_t), intent(inout) :: error
    real(real64) :: value

    integer :: species_index

    value = 0.0_real64
    species_index = this%species_ordering%index(species_name, error)
    if (.not. error%is_success()) return
    value = this%layer_flux(1 + (cell - 1) * int(this%layer_cell_stride) + &
      (level - 1) * int(this%layer_level_stride) + &
      (species_index - 1) * int(this%layer_species_stride))
  end function layer_flux_at

  subroutine emissions_finalize(this)
    type(emissions_t), intent(inout) :: this

    type(error_t_c) :: error_c
    type(error_t) :: error
    integer :: i

    nullify(this%selected_global_cell_ids, this%category_ids)
    nullify(this%surface_flux, this%layer_flux)
    if (allocated(this%sector_flux)) then
      do i = 1, size(this%sector_flux)
        nullify(this%sector_flux(i)%values)
      end do
      deallocate(this%sector_flux)
    end if
    if (allocated(this%category_flux)) then
      do i = 1, size(this%category_flux)
        nullify(this%category_flux(i)%values)
      end do
      deallocate(this%category_flux)
    end if
    if (allocated(this%sector_layer_flux)) then
      do i = 1, size(this%sector_layer_flux)
        nullify(this%sector_layer_flux(i)%values)
      end do
      deallocate(this%sector_layer_flux)
    end if
    if (allocated(this%category_layer_flux)) then
      do i = 1, size(this%category_layer_flux)
        nullify(this%category_layer_flux(i)%values)
      end do
      deallocate(this%category_layer_flux)
    end if
    nullify(this%grid_metadata%index_to_cell_id)
    call reset_grid_field_pointers(this%grid_metadata)

    if (associated(this%sector_ordering)) deallocate(this%sector_ordering)
    if (associated(this%species_ordering)) deallocate(this%species_ordering)

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
    type(error_t) :: error

    if (c_associated(this%ptr)) then
      call delete_mechanism_c(this%ptr, error_c)
      this%ptr = c_null_ptr
      error = error_t(error_c)
      ASSERT(error%is_success())
    end if
  end subroutine mechanism_finalize

end module musica_emissions
