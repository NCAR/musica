Chapter 2
=========

An MICM Box Model Fortran Example
---------------------------------

In this next MUSICA Fortran example,
we will setup a MICM solver, starting with a set of MICM configuration files,
and run the solver for a single integration time step.

The MICM configuration is specified in a top-level ``config.json`` file,
which simply lists the chemical species configuration file followed by
the reactions configuration file.

  .. literalinclude:: ../../../configs/analytical/config.json
    :language: json

For this example, we will have a system of three chemical species
`A`, `B`, and `C`, defined in the JSON file ``species.json`` as follows:

  .. literalinclude:: ../../../configs/analytical/species.json
    :language: json

The ``reactions.json`` specifies a mechanism, or a set of reactions for the system.
Here, we will introduce two Arrhenius type reactions, the first
with `B` evolving to `C`, and specifying all five reaction parameters,
and the second reaction with `A` evolving to `B` and using only two reaction parameters. 
The mechanism configuration might then be set up as:

  .. literalinclude:: ../../../configs/analytical/reactions.json
    :language: json

More information on MICM configurations and reactions can be found in the MICM documentation
at `https://ncar.github.io/micm/user_guide/`_

The Fortran example code is shown below in full: 

  .. literalinclude:: ../../../fortran/test/fetch_content_integration/test_micm_box_model.F90
    :language: f90

From the ``musica_util`` module we need the Fortran types
``error_t``, ``string_t``, and ``mapping_t``.
A pointer to a ``musica_micm::micm_t`` will serve as the interface to the MICM solver
(in the example the pointer name is ``micm``).
Note that the ``config_path`` in the code sample has been set to ``configs/analytical``,
so that subdir should be created relative to the main program and contain
the MICM JSON configuration files,
or otherwise the ``config_path`` should be modified appropriately.
The initial species concentrations are initialized in the ``concentrations`` array,
which is an argument to the MICM solver.

Finally, a single time step solution is obtained through a call to ``micm%solve``,
after which the updated concentrations may be displayed.

.. code-block:: bash

  $ ./test_micm_box_model
    Creating MICM solver...
    Species Name:A, Index:           1
    Species Name:B, Index:           2
    Species Name:C, Index:           3
    Solving starts...
    After solving, concentrations  0.38  1.61E-009  2.62
  $

