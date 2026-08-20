MUSICA Julia API
================

The Julia API provides access to MUSICA's MICM chemical kinetics solver through the ``Musica.jl`` package,
which uses `CxxWrap.jl <https://github.com/JuliaInterop/CxxWrap.jl>`_ to interface with the underlying C++ library.

Installation
------------

Prerequisites
^^^^^^^^^^^^^

- Julia 1.10 or 1.11
- CMake 3.24 or later
- A C++ compiler with C++20 support

Building from Source
^^^^^^^^^^^^^^^^^^^^

1. Clone and build MUSICA with Julia support:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica
   cmake -S . -B build -D MUSICA_ENABLE_JULIA=ON -D CMAKE_BUILD_TYPE=Release
   cmake --build build

2. Install the Julia package dependencies:

.. code-block:: bash

   cd julia
   julia --project=. -e 'using Pkg; Pkg.instantiate()'

Quick Start
-----------

The following example runs a single-cell chemical solve using a Rosenbrock solver:

.. code-block:: julia

   using Musica

   println("MUSICA version: ", Musica.get_version())

   micm = MICM(config_path = "path/to/config")
   state = create_state(micm)

   set_conditions!(state, temperatures = 298.0, pressures = 101325.0)
   set_concentrations!(state, Dict{String,Any}("A" => 1.0, "B" => 0.0))
   set_user_defined_rate_parameters!(state, Dict{String,Any}("USER.k1" => 0.001))

   result = solve!(micm, state, 60.0)  # integrate 60 seconds
   println("Solver state: ", result.state)   # Converged
   println("Steps taken: ", result.stats.number_of_steps)

   concs = get_concentrations(state)
   println("Final [A]: ", concs["A"][1])

API Reference
-------------

Core
^^^^

.. function:: get_version() -> String

   Returns the version string of the MUSICA library.

   .. code-block:: julia

      println(Musica.get_version())  # e.g. "0.16.0"

Constants
^^^^^^^^^

The following physical constants are exported from the ``Musica`` module:

.. list-table::
   :header-rows: 1
   :widths: 25 20 55

   * - Name
     - Value
     - Description
   * - ``AVOGADRO``
     - 6.02214076 × 10²³ mol⁻¹
     - Avogadro's number
   * - ``BOLTZMANN``
     - 1.380649 × 10⁻²³ J K⁻¹
     - Boltzmann constant
   * - ``GAS_CONSTANT``
     - ``AVOGADRO * BOLTZMANN``
     - Universal gas constant (J K⁻¹ mol⁻¹)

Types
-----

SolverType
^^^^^^^^^^

.. type:: SolverType

   Enum controlling which MICM solver backend is used. Pass to the ``MICM`` constructor.

   .. list-table::
      :header-rows: 1
      :widths: 35 10 55

      * - Value
        - Int
        - Description
      * - ``Rosenbrock``
        - 1
        - Vector-ordered Rosenbrock solver
      * - ``RosenbrockStandardOrder``
        - 2
        - Standard-ordered Rosenbrock solver (default)
      * - ``BackwardEuler``
        - 3
        - Vector-ordered Backward Euler solver
      * - ``BackwardEulerStandardOrder``
        - 4
        - Standard-ordered Backward Euler solver
      * - ``CudaRosenbrock``
        - 5
        - GPU Rosenbrock solver (requires CUDA build)

Conditions
^^^^^^^^^^

.. type:: Conditions

   Environmental conditions for a single grid cell.

   **Fields**

   - ``temperature::Float64`` — Temperature in Kelvin
   - ``pressure::Float64`` — Pressure in Pascals
   - ``air_density::Float64`` — Air number density in mol m⁻³

   **Constructor**

   .. code-block:: julia

      Conditions(; temperature=0.0, pressure=0.0, air_density=nothing)

   If ``air_density`` is not provided and both ``temperature`` and ``pressure`` are positive,
   air density is calculated from the Ideal Gas Law: ``p / (R * T)``.

   .. code-block:: julia

      c = Conditions(temperature = 298.0, pressure = 101325.0)
      # air_density is computed automatically

      c2 = Conditions(temperature = 300.0, pressure = 100000.0, air_density = 42.0)

RosenbrockSolverParameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. type:: RosenbrockSolverParameters

   Tuning parameters for the Rosenbrock solver family.

   **Fields**

   - ``relative_tolerance::Float64`` — Relative tolerance (default: ``1e-6``)
   - ``absolute_tolerances::Union{Vector{Float64}, Nothing}`` — Per-species absolute tolerances; ``nothing`` uses solver defaults
   - ``h_min::Float64`` — Minimum step size in seconds (default: ``0.0``)
   - ``h_max::Float64`` — Maximum step size in seconds (default: ``0.0``)
   - ``h_start::Float64`` — Initial step size in seconds (default: ``0.0``)
   - ``max_number_of_steps::Int`` — Maximum number of internal steps (default: ``1000``)

   .. code-block:: julia

      params = RosenbrockSolverParameters(
          relative_tolerance = 1e-8,
          h_start = 1e-5,
          max_number_of_steps = 500,
      )
      micm = MICM(config_path = path, solver_parameters = params)

BackwardEulerSolverParameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. type:: BackwardEulerSolverParameters

   Tuning parameters for the Backward Euler solver family.

   **Fields**

   - ``relative_tolerance::Float64`` — Relative tolerance (default: ``1e-6``)
   - ``absolute_tolerances::Union{Vector{Float64}, Nothing}`` — Per-species absolute tolerances; ``nothing`` uses solver defaults
   - ``max_number_of_steps::Int`` — Maximum number of internal steps (default: ``11``)
   - ``time_step_reductions::Vector{Float64}`` — Five reduction factors applied after failed solves (default: ``[0.5, 0.5, 0.5, 0.5, 0.1]``)

   .. code-block:: julia

      params = BackwardEulerSolverParameters(relative_tolerance = 1e-4, max_number_of_steps = 20)
      micm = MICM(config_path = path, solver_type = BackwardEulerStandardOrder,
                  solver_parameters = params)

SolverState
^^^^^^^^^^^

.. type:: SolverState

   Enum representing the outcome of a ``solve!`` call.

   .. list-table::
      :header-rows: 1
      :widths: 35 10 55

      * - Value
        - Int
        - Meaning
      * - ``NotYetCalled``
        - 0
        - ``solve!`` has not been called yet
      * - ``Running``
        - 1
        - Solver is in progress (internal use)
      * - ``Converged``
        - 2
        - Solution accepted within tolerances
      * - ``ConvergenceExceededMaxSteps``
        - 3
        - Maximum internal steps reached before convergence
      * - ``StepSizeTooSmall``
        - 4
        - Step size fell below numerical limit
      * - ``RepeatedlySingularMatrix``
        - 5
        - Jacobian factorisation failed repeatedly
      * - ``NaNDetected``
        - 6
        - NaN appeared in the solution
      * - ``InfDetected``
        - 7
        - Inf appeared in the solution
      * - ``AcceptingUnconvergedIntegration``
        - 8
        - Solution accepted despite not fully converging

SolverStats
^^^^^^^^^^^

.. type:: SolverStats

   Performance counters returned by ``solve!``.

   **Fields**

   - ``function_calls::Int`` — Number of right-hand-side evaluations
   - ``jacobian_updates::Int`` — Number of Jacobian computations
   - ``number_of_steps::Int`` — Total internal steps taken
   - ``accepted::Int`` — Number of accepted steps
   - ``rejected::Int`` — Number of rejected steps
   - ``decompositions::Int`` — Number of LU decompositions
   - ``solves::Int`` — Number of linear system solves
   - ``final_time::Float64`` — Simulated time reached (seconds)

SolverResult
^^^^^^^^^^^^

.. type:: SolverResult

   Combined outcome of a ``solve!`` call.

   **Fields**

   - ``state::SolverState`` — Convergence status
   - ``stats::SolverStats`` — Performance counters

   .. code-block:: julia

      result = solve!(micm, state, 60.0)
      if result.state == Converged
          println("Took ", result.stats.number_of_steps, " steps")
      end

MICM
^^^^

.. type:: MICM

   Wrapper around the C++ MICM chemical kinetics solver.

   **Constructor**

   .. code-block:: julia

      MICM(; config_path, solver_type=RosenbrockStandardOrder, solver_parameters=nothing)

   - ``config_path::String`` — Path to the mechanism configuration file or directory
   - ``solver_type::SolverType`` — Solver backend to use (see :type:`SolverType`)
   - ``solver_parameters`` — Optional ``RosenbrockSolverParameters`` or ``BackwardEulerSolverParameters``

   .. code-block:: julia

      micm = MICM(config_path = "configs/v0/analytical")
      micm_be = MICM(config_path = "configs/v0/analytical",
                     solver_type = BackwardEulerStandardOrder)

State
^^^^^

.. type:: State

   Chemical state for one or more grid cells. Created via ``create_state``;
   do not construct directly.

   Holds species concentrations, environmental conditions, and user-defined rate
   parameters. Internally uses the same vector-ordering layout as the C++ and Python APIs.

Functions
---------

MICM Functions
^^^^^^^^^^^^^^

.. function:: create_state(micm::MICM; number_of_grid_cells=1) -> State

   Create a new ``State`` object for the given solver.

   - ``number_of_grid_cells`` — Number of independent atmospheric columns (default: ``1``)

   .. code-block:: julia

      state = create_state(micm)
      state3 = create_state(micm, number_of_grid_cells = 3)

.. function:: solve!(micm::MICM, state::State, time_step::Real) -> SolverResult

   Integrate the chemical system forward by ``time_step`` seconds.
   Species concentrations in ``state`` are updated in-place.

   .. code-block:: julia

      result = solve!(micm, state, 60.0)
      @assert result.state == Converged

.. function:: solver_type(micm::MICM) -> SolverType

   Return the solver type this ``MICM`` was created with.

.. function:: set_solver_parameters!(micm::MICM, params)

   Update solver tuning parameters. ``params`` must match the solver type
   (``RosenbrockSolverParameters`` for Rosenbrock solvers, ``BackwardEulerSolverParameters``
   for Backward Euler solvers).

   .. code-block:: julia

      set_solver_parameters!(micm, RosenbrockSolverParameters(relative_tolerance = 1e-8))

.. function:: get_solver_parameters(micm::MICM)

   Return the current solver parameters as ``RosenbrockSolverParameters`` or
   ``BackwardEulerSolverParameters``, depending on the solver type.

State Functions
^^^^^^^^^^^^^^^

.. function:: set_concentrations!(state::State, concentrations::Dict{String})

   Set species concentrations. For a single grid cell, values may be scalars.
   For multiple grid cells, provide a ``Vector`` of length ``number_of_grid_cells``.

   .. code-block:: julia

      # Single grid cell
      set_concentrations!(state, Dict{String,Any}("A" => 1.0, "B" => 0.0))

      # Multiple grid cells
      set_concentrations!(state, Dict{String,Any}("A" => [1.0, 2.0, 3.0]))

.. function:: get_concentrations(state::State) -> Dict{String, Vector{Float64}}

   Return species concentrations for all grid cells.
   Keys are species names; each value is a vector of length ``number_of_grid_cells``.

.. function:: set_conditions!(state::State; temperatures, pressures, air_densities)

   Set environmental conditions. All keyword arguments accept scalars (single grid cell)
   or vectors (multiple grid cells). If ``air_densities`` is omitted, it is computed
   from the Ideal Gas Law using the provided temperature and pressure.

   .. code-block:: julia

      # Single cell, auto air_density
      set_conditions!(state, temperatures = 298.0, pressures = 101325.0)

      # Multiple cells
      set_conditions!(state,
          temperatures = [298.0, 310.0, 280.0],
          pressures    = [101325.0, 95000.0, 105000.0])

.. function:: get_conditions(state::State) -> Dict{String, Vector{Float64}}

   Return environmental conditions for all grid cells.
   Keys are ``"temperature"`` (K), ``"pressure"`` (Pa), and ``"air_density"`` (mol m⁻³).

.. function:: set_user_defined_rate_parameters!(state::State, params::Dict{String})

   Set user-defined rate parameters (e.g. photolysis rates or emission fluxes).
   Values may be scalars or vectors matching the number of grid cells.

   .. code-block:: julia

      set_user_defined_rate_parameters!(state,
          Dict{String,Any}("USER.reaction 1" => 0.001, "USER.reaction 2" => 0.002))

.. function:: get_user_defined_rate_parameters(state::State) -> Dict{String, Vector{Float64}}

   Return user-defined rate parameters for all grid cells.

.. function:: get_species_ordering(state::State) -> Dict{String, Int}

   Return the mapping of species names to their 0-based indices in the internal
   concentration array.

.. function:: get_user_defined_rate_parameters_ordering(state::State) -> Dict{String, Int}

   Return the mapping of user-defined rate parameter names to their 0-based indices.

TUV-x
-----

TUV-x is an optional Fortran component. Its types and functions exist only when
the library was built with TUV-x.

.. function:: tuvx_available() -> Bool

   Report whether the bindings were built with TUV-x support.

.. function:: get_tuvx_version() -> String

   Return the version of the TUV-x library.

Grid
^^^^

.. type:: Grid

   A grid on which TUV-x profiles are defined, such as a height grid or a
   wavelength grid.

   **Constructor**

   .. code-block:: julia

      Grid(; name, units, num_sections=nothing, edges=nothing, midpoints=nothing)

   Give at least one of ``num_sections``, ``edges``, or ``midpoints``.

   - ``name::AbstractString`` — Name of the grid
   - ``units::AbstractString`` — Units of the grid values
   - ``num_sections::Integer`` — Number of grid sections
   - ``edges::AbstractVector{<:Real}`` — Edge values, of length ``num_sections + 1``
   - ``midpoints::AbstractVector{<:Real}`` — Midpoint values, of length ``num_sections``

   .. code-block:: julia

      grid = Grid(name = "height", units = "km", edges = [0.0, 2.0, 4.0, 6.0])

.. function:: get_name(grid::Grid) -> String

   Return the name of the grid.

.. function:: get_units(grid::Grid) -> String

   Return the units of the grid values.

.. function:: num_sections(grid::Grid) -> Int

   Return the number of sections in the grid. ``length(grid)`` returns the same value.

.. function:: edges(grid::Grid) -> GridView

   Return a zero-copy view of the grid edges, of length ``num_sections + 1``.
   A write to the view changes the grid.

   .. code-block:: julia

      edges(grid) .= [0.0, 2.0, 4.0, 6.0]
      first_edge = edges(grid)[1]

.. function:: midpoints(grid::Grid) -> GridView

   Return a zero-copy view of the grid midpoints, of length ``num_sections``.
   A write to the view changes the grid.

.. function:: set_edges!(grid::Grid, values::AbstractVector{<:Real}) -> Grid

   Copy ``values`` into the grid edges. The length must equal ``num_sections + 1``.

.. function:: set_midpoints!(grid::Grid, values::AbstractVector{<:Real}) -> Grid

   Copy ``values`` into the grid midpoints. The length must equal ``num_sections``.

GridMap
^^^^^^^

.. type:: GridMap

   A collection of ``Grid`` objects, keyed by name and units. The map supports
   both named methods and dictionary-style access.

   **Constructor**

   .. code-block:: julia

      grids = GridMap()

   Index access with an integer is 1-based. Iteration and ``values`` return
   ``Grid`` objects. ``keys`` returns ``(name, units)`` tuples.

   .. code-block:: julia

      grids = GridMap()
      grids["height", "km"] = Grid(name = "height", units = "km", num_sections = 5)
      length(grids)                     # 1
      grid = grids["height", "km"]
      haskey(grids, ("height", "km"))   # true
      for a_grid in grids
          println(get_name(a_grid))
      end

.. function:: add_grid!(map::GridMap, grid::Grid) -> GridMap

   Add a grid to the map.

   The map takes over the memory of the TUV-x grid. The ``grid`` object stays
   usable and reads through the map. Get a new view from ``edges`` or
   ``midpoints`` after this call, because an older view points to memory that
   the map has released.

.. function:: get_grid(map::GridMap, name::AbstractString, units::AbstractString) -> Grid
              get_grid(map::GridMap, index::Integer) -> Grid

   Return a grid from the map by name and units, or by 1-based index.

.. function:: remove_grid!(map::GridMap, name::AbstractString, units::AbstractString) -> GridMap
              remove_grid!(map::GridMap, index::Integer) -> GridMap

   Remove a grid from the map by name and units, or by 1-based index.

.. function:: get_number_of_grids(map::GridMap) -> Int

   Return the number of grids in the map. ``length(map)`` returns the same value.

Profile
^^^^^^^

.. type:: Profile

   A physical quantity defined on a TUV-x ``Grid``, such as temperature or a
   species concentration.

   **Constructor**

   .. code-block:: julia

      Profile(; name, units, grid, edge_values=nothing, midpoint_values=nothing,
                layer_densities=nothing, calculate_layer_densities=false,
                exo_layer_density=0.0)

   Give at most one of ``edge_values`` or ``midpoint_values``; the other is
   derived from it by linear interpolation/extrapolation. Give at most one of
   ``layer_densities`` or ``calculate_layer_densities=true``.

   - ``name::AbstractString`` — Name of the profile
   - ``units::AbstractString`` — Units of the profile values
   - ``grid::Grid`` — Grid on which the profile is defined
   - ``edge_values::AbstractVector{<:Real}`` — Values at grid edges, of length ``num_sections(grid) + 1``
   - ``midpoint_values::AbstractVector{<:Real}`` — Values at grid midpoints, of length ``num_sections(grid)``
   - ``layer_densities::AbstractVector{<:Real}`` — Layer densities, of length ``num_sections(grid)``
   - ``calculate_layer_densities::Bool`` — If ``true``, calculate layer densities from midpoint values
   - ``exo_layer_density::Real`` — Layer density above the top of the grid

   .. code-block:: julia

      grid = Grid(name = "height", units = "km", edges = [0.0, 2.0, 4.0])
      profile = Profile(name = "temperature", units = "K", grid = grid,
                         midpoint_values = [270.0, 260.0])

.. function:: get_name(profile::Profile) -> String

   Return the name of the profile.

.. function:: get_units(profile::Profile) -> String

   Return the units of the profile values.

.. function:: num_sections(profile::Profile) -> Int

   Return the number of sections in the profile's grid. ``length(profile)`` returns the same value.

.. function:: edge_values(profile::Profile) -> ProfileView

   Return a zero-copy view of the profile values at grid edges, of length
   ``num_sections + 1``. A write to the view changes the profile.

.. function:: midpoint_values(profile::Profile) -> ProfileView

   Return a zero-copy view of the profile values at grid midpoints, of length
   ``num_sections``. A write to the view changes the profile.

.. function:: layer_densities(profile::Profile) -> ProfileView

   Return a zero-copy view of the profile's layer densities, of length
   ``num_sections``. A write to the view changes the profile.

.. function:: set_edge_values!(profile::Profile, values::AbstractVector{<:Real}) -> Profile
              set_midpoint_values!(profile::Profile, values::AbstractVector{<:Real}) -> Profile
              set_layer_densities!(profile::Profile, values::AbstractVector{<:Real}) -> Profile

   Copy ``values`` into the profile edge values, midpoint values, or layer
   densities. The length must equal the length of the target array.

.. function:: exo_layer_density(profile::Profile) -> Float64

   Return the layer density above the top of the profile's grid.

.. function:: set_exo_layer_density!(profile::Profile, value::Real) -> Profile

   Set the layer density above the top of the profile's grid.

.. function:: calculate_exo_layer_density!(profile::Profile, scale_height::Real) -> Profile

   Calculate the layer density above the top of the profile's grid from the
   given scale height.

.. function:: calculate_layer_densities!(profile::Profile, grid::Grid; conv=nothing) -> Profile

   Calculate layer densities from midpoint values and grid spacing. ``conv``
   defaults to ``1.0``, except when the grid is named ``"height"`` with units
   ``"km"`` and the profile units are ``"molecule cm-3"``, where it defaults to ``1.0e5``.

ProfileMap
^^^^^^^^^^

.. type:: ProfileMap

   A collection of ``Profile`` objects, keyed by name and units. The map
   supports both named methods and dictionary-style access.

   **Constructor**

   .. code-block:: julia

      profiles = ProfileMap()

   Index access with an integer is 1-based. Iteration and ``values`` return
   ``Profile`` objects. ``keys`` returns ``(name, units)`` tuples.

   .. code-block:: julia

      profiles = ProfileMap()
      grid = Grid(name = "height", units = "km", num_sections = 5)
      profiles["temperature", "K"] = Profile(name = "temperature", units = "K", grid = grid)
      length(profiles)                          # 1
      profile = profiles["temperature", "K"]
      haskey(profiles, ("temperature", "K"))    # true
      for a_profile in profiles
          println(get_name(a_profile))
      end

.. function:: add_profile!(map::ProfileMap, profile::Profile) -> ProfileMap

   Add a profile to the map.

   The map takes over the memory of the TUV-x profile. The ``profile`` object
   stays usable and reads through the map. Get a new view from
   ``edge_values``, ``midpoint_values``, or ``layer_densities`` after this
   call, because an older view points to memory that the map has released.

.. function:: get_profile(map::ProfileMap, name::AbstractString, units::AbstractString) -> Profile
              get_profile(map::ProfileMap, index::Integer) -> Profile

   Return a profile from the map by name and units, or by 1-based index.

.. function:: remove_profile!(map::ProfileMap, name::AbstractString, units::AbstractString) -> ProfileMap
              remove_profile!(map::ProfileMap, index::Integer) -> ProfileMap

   Remove a profile from the map by name and units, or by 1-based index.

.. function:: get_number_of_profiles(map::ProfileMap) -> Int

   Return the number of profiles in the map. ``length(map)`` returns the same value.

Radiator
^^^^^^^^

.. type:: Radiator

   An optically active species for TUV-x radiative transfer calculations,
   such as an aerosol layer.

   **Constructor**

   .. code-block:: julia

      Radiator(; name, height_grid, wavelength_grid, optical_depths=nothing,
                 single_scattering_albedos=nothing, asymmetry_factors=nothing)

   - ``name::AbstractString`` — Name of the radiator
   - ``height_grid::Grid`` — Height grid on which the radiator is defined
   - ``wavelength_grid::Grid`` — Wavelength grid on which the radiator is defined
   - ``optical_depths::AbstractMatrix{<:Real}`` — Optical depths, shape ``(num_height_sections, num_wavelength_sections)``
   - ``single_scattering_albedos::AbstractMatrix{<:Real}`` — Single scattering albedos, same shape as ``optical_depths``
   - ``asymmetry_factors::AbstractMatrix{<:Real}`` — Asymmetry factors, same shape as ``optical_depths``

   The number of streams is currently fixed at 1 in TUV-x, so asymmetry
   factors are exposed as a 2D array.

   .. code-block:: julia

      height_grid = Grid(name = "height", units = "km", edges = [0.0, 2.0, 4.0])
      wavelength_grid = Grid(name = "wavelength", units = "nm", edges = [200.0, 300.0])
      radiator = Radiator(name = "aerosol", height_grid = height_grid, wavelength_grid = wavelength_grid)

.. function:: get_name(radiator::Radiator) -> String

   Return the name of the radiator.

.. function:: num_height_sections(radiator::Radiator) -> Int

   Return the number of sections in the radiator's height grid.

.. function:: num_wavelength_sections(radiator::Radiator) -> Int

   Return the number of sections in the radiator's wavelength grid.

.. function:: optical_depths(radiator::Radiator) -> RadiatorView
              single_scattering_albedos(radiator::Radiator) -> RadiatorView
              asymmetry_factors(radiator::Radiator) -> RadiatorView

   Return a zero-copy view of the corresponding radiator array, of shape
   ``(num_height_sections, num_wavelength_sections)``. A write to the view
   changes the radiator.

.. function:: set_optical_depths!(radiator::Radiator, values::AbstractMatrix{<:Real}) -> Radiator
              set_single_scattering_albedos!(radiator::Radiator, values::AbstractMatrix{<:Real}) -> Radiator
              set_asymmetry_factors!(radiator::Radiator, values::AbstractMatrix{<:Real}) -> Radiator

   Copy ``values`` into the corresponding radiator array. The shape must
   equal ``(num_height_sections, num_wavelength_sections)``.

RadiatorMap
^^^^^^^^^^^

.. type:: RadiatorMap

   A collection of ``Radiator`` objects, keyed by name. Unlike ``GridMap``
   and ``ProfileMap``, there is no units key. The map supports both named
   methods and dictionary-style access.

   **Constructor**

   .. code-block:: julia

      radiators = RadiatorMap()

   Index access with an integer is 1-based. Iteration and ``values`` return
   ``Radiator`` objects. ``keys`` returns radiator names.

   .. code-block:: julia

      radiators = RadiatorMap()
      height_grid = Grid(name = "height", units = "km", num_sections = 5)
      wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
      radiators["aerosol"] = Radiator(name = "aerosol", height_grid = height_grid,
                                       wavelength_grid = wavelength_grid)
      length(radiators)          # 1
      radiator = radiators["aerosol"]
      haskey(radiators, "aerosol")   # true
      for a_radiator in radiators
          println(get_name(a_radiator))
      end

.. function:: add_radiator!(map::RadiatorMap, radiator::Radiator) -> RadiatorMap

   Add a radiator to the map.

   The map takes over the memory of the TUV-x radiator. The ``radiator``
   object stays usable and reads through the map. Get a new view from
   ``optical_depths``, ``single_scattering_albedos``, or
   ``asymmetry_factors`` after this call, because an older view points to
   memory that the map has released.

.. function:: get_radiator(map::RadiatorMap, name::AbstractString) -> Radiator
              get_radiator(map::RadiatorMap, index::Integer) -> Radiator

   Return a radiator from the map by name, or by 1-based index.

.. function:: remove_radiator!(map::RadiatorMap, name::AbstractString) -> RadiatorMap
              remove_radiator!(map::RadiatorMap, index::Integer) -> RadiatorMap

   Remove a radiator from the map by name, or by 1-based index.

.. function:: get_number_of_radiators(map::RadiatorMap) -> Int

   Return the number of radiators in the map. ``length(map)`` returns the same value.

TUVX
^^^^

.. type:: TUVX

   The TUV-x photolysis calculator.

   **Constructor**

   .. code-block:: julia

      TUVX(; grid_map, profile_map, radiator_map, config_path=nothing, config_string=nothing)

   Provide exactly one of ``config_path`` or ``config_string``.

   - ``grid_map::GridMap`` — Grid definitions (height, wavelength) for the calculation
   - ``profile_map::ProfileMap`` — Atmospheric profiles (temperature, species concentrations, surface albedo, ET flux)
   - ``radiator_map::RadiatorMap`` — Optically active species
   - ``config_path::AbstractString`` — Path to a JSON/YAML configuration file
   - ``config_string::AbstractString`` — A JSON/YAML configuration as a string

   TUV-x opens relative data-file paths named inside the configuration
   against the current working directory, not the configuration file's own
   location.

   .. code-block:: julia

      grids = GridMap()
      profiles = ProfileMap()
      radiators = RadiatorMap()
      # ... populate grids, profiles, radiators to match the configuration ...
      tuvx = TUVX(grid_map = grids, profile_map = profiles, radiator_map = radiators,
                  config_path = "path/to/config.json")
      result = run!(tuvx, deg2rad(30.0), 1.0)
      result.photolysis_rate_constants  # (num_reactions, num_vertical_edges)

.. function:: get_grid_map(tuvx::TUVX) -> GridMap
              get_profile_map(tuvx::TUVX) -> ProfileMap
              get_radiator_map(tuvx::TUVX) -> RadiatorMap

   Return the grid, profile, or radiator map used by this TUV-x instance.

.. function:: photolysis_rate_constant_count(tuvx::TUVX) -> Int
              heating_rate_count(tuvx::TUVX) -> Int
              dose_rate_count(tuvx::TUVX) -> Int
              num_height_midpoints(tuvx::TUVX) -> Int
              num_wavelength_midpoints(tuvx::TUVX) -> Int

   Return the number of photolysis reactions, heating rate types, dose rate
   types, vertical layers, or wavelength bins.

.. function:: photolysis_rate_names(tuvx::TUVX) -> Dict{String, Int}
              heating_rate_names(tuvx::TUVX) -> Dict{String, Int}
              dose_rate_names(tuvx::TUVX) -> Dict{String, Int}

   Return the mapping of photolysis reaction, heating rate, or dose rate
   names to their 0-based index in the corresponding output array from
   :func:`run!`.

.. function:: run!(tuvx::TUVX, solar_zenith_angle::Real, earth_sun_distance::Real) -> NamedTuple

   Run the TUV-x photolysis calculator.

   - ``solar_zenith_angle`` — Solar zenith angle in radians
   - ``earth_sun_distance`` — Earth-Sun distance in astronomical units (AU)

   Returns a ``NamedTuple`` with:

   - ``photolysis_rate_constants`` — ``(num_reactions, num_vertical_edges)`` [s⁻¹]
   - ``heating_rates`` — ``(num_heating_rates, num_vertical_edges)`` [K s⁻¹]
   - ``dose_rates`` — ``(num_dose_rates, num_vertical_edges)`` [W m⁻²]
   - ``actinic_flux`` — ``(num_wavelengths, num_vertical_edges, 3)`` [photons cm⁻² s⁻¹ nm⁻¹]
   - ``spectral_irradiance`` — ``(num_wavelengths, num_vertical_edges, 3)`` [W m⁻² nm⁻¹]

   The trailing dimension of ``actinic_flux`` and ``spectral_irradiance`` indexes the direct,
   upwelling, and downwelling components, in that order.

.. function:: get_photolysis_rate_constant(tuvx::TUVX, reaction_name::AbstractString, photolysis_rate_constants::AbstractMatrix{<:Real}) -> Vector{Float64}
              get_heating_rate(tuvx::TUVX, rate_name::AbstractString, heating_rates::AbstractMatrix{<:Real}) -> Vector{Float64}
              get_dose_rate(tuvx::TUVX, rate_name::AbstractString, dose_rates::AbstractMatrix{<:Real}) -> Vector{Float64}

   Extract one named reaction's or rate's row across all vertical edges from
   the corresponding output of :func:`run!`.

Mechanism Configuration
-----------------------

The ``Musica.MechanismConfiguration`` submodule builds version 1 mechanism
configurations in code and serializes them to a string for ``MICM(config_string = ...)``.
See the :ref:`user guide <julia-user-guide>` for a full example.

Data Types
^^^^^^^^^^

.. type:: Species

   A chemical species.

   .. code-block:: julia

      Species(; name, molecular_weight=nothing, constant_concentration=nothing,
                constant_mixing_ratio=nothing, is_third_body=nothing,
                other_properties=Dict())

   Optional fields left as ``nothing`` are omitted from the output. Keys in
   ``other_properties`` are serialized with a ``__`` prefix.

.. type:: PhaseSpecies

   A species reference within a phase, optionally carrying a diffusion coefficient.

   .. code-block:: julia

      PhaseSpecies(; name, diffusion_coefficient=nothing, other_properties=Dict())

.. type:: Phase

   A phase grouping species. Members may be a ``Species``, a ``PhaseSpecies``, or a
   bare species-name ``String``.

   .. code-block:: julia

      Phase(; name, species, other_properties=Dict())

.. type:: ReactionComponent

   A reactant or product entry.

   .. code-block:: julia

      ReactionComponent(; species_name, coefficient=1.0, other_properties=Dict())

Reaction Types
^^^^^^^^^^^^^^

Each reaction type is constructed with keyword arguments and supports ``name``,
``gas_phase``, and ``other_properties``:
``Arrhenius``, ``Branched``, ``Emission``, ``FirstOrderLoss``, ``Photolysis``,
``Surface``, ``TaylorSeries``, ``Troe``, ``TernaryChemicalActivation``,
``Tunneling``, and ``UserDefined``.

Mechanism and Serialization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. type:: Mechanism

   A complete mechanism configuration.

   .. code-block:: julia

      Mechanism(; name, version="1.0.0", species, phases, reactions)

.. function:: to_json_string(m::Mechanism) -> String

   Serialize the mechanism to a JSON string.

.. function:: to_yaml_string(m::Mechanism) -> String

   Serialize the mechanism to a YAML string.

.. function:: to_string(m::Mechanism; format=:json) -> String

   Serialize the mechanism to a string. ``format`` may be ``:json`` or ``:yaml``.

.. function:: to_dict(m::Mechanism) -> Dict{String,Any}

   Build the nested dictionary representation in the version 1 schema.

Multi-Grid-Cell Example
-----------------------

.. code-block:: julia

   using Musica

   micm = MICM(config_path = "configs/v0/analytical")
   state = create_state(micm, number_of_grid_cells = 3)

   set_concentrations!(state, Dict{String,Any}(
       "A" => [1.0, 2.0, 3.0],
       "B" => [0.0, 0.0, 0.0],
   ))

   set_conditions!(state,
       temperatures = [298.0, 310.0, 280.0],
       pressures    = [101325.0, 95000.0, 105000.0],
   )

   set_user_defined_rate_parameters!(state, Dict{String,Any}(
       "USER.reaction 1" => [0.001, 0.002, 0.003],
       "USER.reaction 2" => [0.004, 0.005, 0.006],
   ))

   result = solve!(micm, state, 60.0)
   @assert result.state == Converged

   concs = get_concentrations(state)
   println("Final [A] per cell: ", concs["A"])

Testing
-------

To run the Julia test suite:

.. code-block:: bash

   cd julia
   julia --project=. test/runtests.jl

Additional Resources
--------------------

- `CxxWrap.jl Documentation <https://github.com/JuliaInterop/CxxWrap.jl>`_
- `MUSICA GitHub Repository <https://github.com/NCAR/musica>`_
- :doc:`MICM Documentation <micm:index>`
