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

      println(Musica.get_version())  # e.g. "0.14.5"

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
