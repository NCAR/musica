.. _julia-user-guide:

Overview
========

The Julia API exposes MICM gas-phase chemistry and is built on top of the C++
library using `CxxWrap.jl <https://github.com/JuliaInterop/CxxWrap.jl>`_. For
installation, see :doc:`../../getting_started/julia`.

Defining a Mechanism
--------------------

Mechanisms are loaded from a :doc:`mechanism configuration file <mc:index>`
directory. Use the bundled ``configs/v0/analytical`` example or provide your own:

.. code-block:: julia

   using MUSICA

   solver = MICM("configs/v0/analytical", SolverType.rosenbrock_standard_order)

Creating a Solver
-----------------

The solver type is the second argument to ``MICM``. Available options via the
``SolverType`` enum:

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

Setting Conditions
------------------

Create a state and set concentrations and environmental conditions:

.. code-block:: julia

   state = create_state(solver)

   set_concentrations!(state, Dict("A" => 1.0, "B" => 3.0, "C" => 5.0))
   set_conditions!(state, 300.0, 101000.0)   # temperature (K), pressure (Pa)

Solving
-------

Call ``solve!`` at each time step:

.. code-block:: julia

   time_step  = 4.0    # s
   sim_length = 20.0   # s
   curr_time  = time_step

   while curr_time <= sim_length
       solve!(solver, state, time_step)
       curr_time += time_step
   end

Accessing Results
-----------------

Retrieve updated concentrations from the state after each solve:

.. code-block:: julia

   conc = get_concentrations(state)
   println("A=$(conc["A"][1])  B=$(conc["B"][1])  C=$(conc["C"][1])")

.. note::

   TUV-x photolysis and CARMA aerosol support are not yet available in the
   Julia API.

Further Reading
---------------

- :doc:`Julia API Reference <../../api/julia>`
- :doc:`MICM Documentation <micm:index>`
- `CxxWrap.jl Documentation <https://github.com/JuliaInterop/CxxWrap.jl>`_
