.. _cpp-user-guide:

Overview
========

The C++ library is the foundation that all other language bindings are built
on, and gives direct access to MICM, TUV-x, and CARMA. For installation, see
:doc:`../../getting_started/cpp`.

Defining a Mechanism
--------------------

Mechanisms are loaded from a :doc:`mechanism configuration file <mc:index>`
directory. Use the bundled ``configs/v0/analytical`` example or provide your own:

.. code-block:: cpp

   #include <musica/micm.hpp>

   auto micm = musica::MICM("configs/v0/analytical",
                             musica::MICMSolver::RosenbrockStandardOrder);

See :doc:`../cmake_options/index` for build options including TUV-x, CARMA, and
GPU support.

Creating a Solver
-----------------

The solver type is selected at construction time. Available options in
``musica::MICMSolver``:

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - Solver
     - Description
   * - ``RosenbrockStandardOrder``
     - Rosenbrock solver (standard species ordering)
   * - ``RosenbrockArbitraryOrder``
     - Rosenbrock solver (arbitrary species ordering)
   * - ``BackwardEulerStandardOrder``
     - Backward Euler solver (standard species ordering)
   * - ``BackwardEulerArbitraryOrder``
     - Backward Euler solver (arbitrary species ordering)
   * - ``CudaRosenbrock``
     - GPU Rosenbrock solver (requires CUDA build)

Setting Conditions
------------------

Create a state and set concentrations and environmental conditions:

.. code-block:: cpp

   auto state = micm.CreateState();

   state.SetConcentrations({ {"A", {1.0}}, {"B", {3.0}}, {"C", {5.0}} });
   state.SetConditions(300.0 /*K*/, 101000.0 /*Pa*/);

Solving
-------

Call ``Solve`` at each time step:

.. code-block:: cpp

   double time_step  = 4.0;   // s
   double sim_length = 20.0;  // s

   for (double t = time_step; t <= sim_length; t += time_step) {
     micm.Solve(state, time_step);
   }

Accessing Results
-----------------

Retrieve updated concentrations from the state after each solve:

.. code-block:: cpp

   auto conc = state.GetConcentrations();
   std::cout << "t=" << t << "s  A=" << conc["A"][0] << "\n";

Using MUSICA in a CMake Project
--------------------------------

Link MUSICA into your project with ``find_package``:

.. code-block:: cmake

   find_package(musica REQUIRED)
   target_link_libraries(my_target musica::musica)

If installed to a non-standard prefix:

.. code-block:: bash

   cmake -D CMAKE_PREFIX_PATH=<INSTALL_DIR> ..

Further Reading
---------------

- :doc:`C++ API Reference <../../api/C++>`
- :doc:`CMake Options <../cmake_options/index>`
- :doc:`MICM Documentation <micm:index>`
