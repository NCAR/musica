.. _julia-user-guide:

Overview
========

This section covers using MUSICA through the Julia API. The Julia interface
exposes MICM gas-phase chemistry and is built on top of the C++ library using
`CxxWrap.jl <https://github.com/JuliaInterop/CxxWrap.jl>`_.

Installation
------------

The MUSICA Julia package is registered in the General Julia registry. Install
it from the Julia REPL:

.. code-block:: julia

   using Pkg
   Pkg.add("MUSICA")

To verify the installation:

.. code-block:: julia

   using MUSICA
   println(MUSICA_VERSION)

Defining a Mechanism
--------------------

Mechanisms in the Julia API are loaded from a
:doc:`mechanism configuration file <mc:index>` in JSON format.
Place your configuration files in a directory (e.g., ``configs/analytical``)
and load the solver:

.. code-block:: julia

   using MUSICA

   solver = MICM("configs/analytical", SolverType.rosenbrock_standard_order)

Setting Conditions and Solving
-------------------------------

Create a state, set initial conditions, and solve through a time loop:

.. code-block:: julia

   using MUSICA

   solver = MICM("configs/analytical", SolverType.rosenbrock_standard_order)
   state  = create_state(solver)

   temperature = 300.0   # K
   pressure    = 101000.0  # Pa

   concentrations = Dict("A" => 1.0, "B" => 3.0, "C" => 5.0)
   set_concentrations!(state, concentrations)
   set_conditions!(state, temperature, pressure)

   time_step  = 4.0   # s
   sim_length = 20.0  # s
   curr_time  = time_step

   while curr_time <= sim_length
       solve!(solver, state, time_step)
       conc = get_concentrations(state)
       println("t=$(curr_time)s  A=$(conc["A"][1])  B=$(conc["B"][1])  C=$(conc["C"][1])")
       curr_time += time_step
   end

Solver Types
------------

Several solver types are available through the ``SolverType`` enum:

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - Solver
     - Description
   * - ``SolverType.rosenbrock_standard_order``
     - Rosenbrock solver (standard species ordering)
   * - ``SolverType.rosenbrock_arbitrary_order``
     - Rosenbrock solver (arbitrary species ordering)
   * - ``SolverType.backward_euler_standard_order``
     - Backward Euler solver (standard species ordering)
   * - ``SolverType.backward_euler_arbitrary_order``
     - Backward Euler solver (arbitrary species ordering)

Further Reading
---------------

- :doc:`Julia API Reference <../api/julia>`
- :doc:`MICM Documentation <micm:index>`
- `CxxWrap.jl Documentation <https://github.com/JuliaInterop/CxxWrap.jl>`_
