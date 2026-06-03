.. _chapter4:

Chapter 4
=========

Multiple Mechanisms in the Same Fortran Host Application
---------------------------------------------------------

In real atmospheric models, different parts of the model may require different
chemical mechanisms. For example, an urban chemistry scheme might co-exist with
a remote-troposphere scheme, or a gas-phase solver might run alongside an aerosol
chemistry solver. MUSICA supports this naturally: you can create as many
independent ``micm_t`` solver instances as you need, each with its own mechanism,
and advance all of them inside the same Fortran host application.

In this example, we create **two independent solvers**:

- **Solver 1** loads the six-species analytical mechanism from ``configs/v0/analytical``.
  This mechanism has six species (``A``, ``B``, ``C``, ``D``, ``E``, ``F``), two
  Arrhenius reactions, and two user-defined rate-constant reactions.

- **Solver 2** loads the three-species analytical mechanism from
  ``configs/v1/analytical/config.json``.
  This mechanism has three species (``A``, ``B``, ``C``) and two Arrhenius reactions.

Both solvers share the same physical conditions (temperature and pressure) but
maintain completely independent state vectors. Both are advanced through the same
time step in the same simulation loop.

Save the following to ``test_multiple_mechanisms.F90``:

  .. literalinclude:: ../../../../fortran/test/tutorial/test_multiple_mechanisms.F90
    :language: f90

Key concepts illustrated by this example:

**Independent solver instances**

Each ``micm_t`` pointer is a self-contained solver. Creating a second solver does
not affect the first:

.. code-block:: fortran

  micm_1 => micm_t(config_path_1, RosenbrockStandardOrder, error)
  micm_2 => micm_t(config_path_2, RosenbrockStandardOrder, error)

**Independent state objects**

Each solver creates its own ``state_t`` object that holds concentrations and
environmental conditions for that mechanism only:

.. code-block:: fortran

  state_1 => micm_1%get_state(num_grid_cells, error)
  state_2 => micm_2%get_state(num_grid_cells, error)

Because ``state_1`` and ``state_2`` were created from different solvers, they may
have different numbers of species, different internal strides, and different
user-defined rate-parameter arrays.

**Simultaneous time-stepping**

Within the same simulation loop, both mechanisms are advanced by calling each
solver's ``solve`` method:

.. code-block:: fortran

  call micm_1%solve(time_step, state_1, solver_state_str, solver_stats, error)
  call micm_2%solve(time_step, state_2, solver_state_str, solver_stats, error)

The two calls are completely independent: they can even be re-ordered or
parallelized without affecting the results of either mechanism.

**Memory management**

Each pointer must be deallocated independently when it is no longer needed:

.. code-block:: fortran

  deallocate(state_1)
  deallocate(micm_1)
  deallocate(state_2)
  deallocate(micm_2)

To build and run the example, follow the same build instructions as in Chapter 1.
The compilation command would be:

.. code-block:: bash

  gfortran -o test_multiple_mechanisms test_multiple_mechanisms.F90 \
    -I<MUSICA_DIR>/include -L<MUSICA_DIR>/lib64 -lmusica-fortran -lmusica -lstdc++ -lyaml-cpp

Run it with:

.. code-block:: bash

  $ ./test_multiple_mechanisms
   Creating Solver 1 from: configs/v0/analytical
     Mechanism 1 has 6 species:
       F  index=6
       E  index=4
       B  index=2
       C  index=5
       D  index=3
       A  index=1
   Creating Solver 2 from: configs/v1/analytical/config.json
     Mechanism 2 has 3 species:
       B  index=2
       C  index=3
       A  index=1

   Advancing both mechanisms by one time step...
     Solver 1 result: Converged
     Solver 2 result: Converged

   Final concentrations – Mechanism 1:
       F =     0.382238
       E =     1.467040
       B =     0.818730
       C =     0.818730
       D =     1.150722
       A =     1.362541

   Final concentrations – Mechanism 2:
       B =     0.449202
       C =     0.809056
       A =     1.741742

   Both mechanisms solved successfully in the same host application!

The final concentrations differ because the two mechanisms have different species,
reactions, and rate expressions.
