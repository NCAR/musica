Julia
=====

Installing MUSICA
-----------------

MUSICA is registered in the General Julia registry. Install it from the Julia REPL:

.. code-block:: julia

   using Pkg
   Pkg.add("MUSICA")

Verifying the installation
---------------------------

.. code-block:: julia

   using MUSICA
   println(MUSICA_VERSION)

This should print the installed version.

Next steps
----------

- :ref:`Julia User Guide <julia-user-guide>` — learn how to define mechanisms and run solvers
- :doc:`Julia API Reference <../api/julia>` — full API documentation
- :ref:`Development Setup <development-setup>` — set up for contributing to MUSICA
