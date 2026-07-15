MUSICA JavaScript API
=====================

The JavaScript API exposes MUSICA's MICM chemical kinetics solver through a
WebAssembly (WASM) module. For installation and usage examples, see the
:ref:`JavaScript User Guide <javascript-user-guide>`.

API Reference
-------------

Module Initialization
^^^^^^^^^^^^^^^^^^^^^

.. js:autofunction:: initModule

.. js:autofunction:: getBackend

.. js:autofunction:: getVersion

.. js:autofunction:: getMicmVersion

Constants
^^^^^^^^^

.. js:autoattribute:: AVOGADRO

.. js:autoattribute:: BOLTZMANN

.. js:autoattribute:: GAS_CONSTANT

Enumerations
^^^^^^^^^^^^

.. js:autoattribute:: SolverType

   Solver backend selection. Pass to :js:class:`MICM` factory methods.

   .. list-table::
      :header-rows: 1
      :widths: 40 10 50

      * - Key
        - Value
        - Description
      * - ``rosenbrock``
        - 1
        - Vector-ordered Rosenbrock solver
      * - ``rosenbrock_standard_order``
        - 2
        - Standard-ordered Rosenbrock solver (default)
      * - ``backward_euler``
        - 3
        - Vector-ordered Backward Euler solver
      * - ``backward_euler_standard_order``
        - 4
        - Standard-ordered Backward Euler solver

.. js:autoattribute:: SolverState

   Outcome of a :js:func:`MICM.solve` call.

   .. list-table::
      :header-rows: 1
      :widths: 40 10 50

      * - Key
        - Value
        - Meaning
      * - ``NotYetCalled``
        - 0
        - ``solve`` has not been called yet
      * - ``Running``
        - 1
        - Solver is executing (internal use)
      * - ``Converged``
        - 2
        - Solution accepted within tolerances
      * - ``ConvergenceExceededMaxSteps``
        - 3
        - Maximum internal steps reached
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

Classes
^^^^^^^

.. js:autoclass:: Conditions
   :members:

.. js:autoclass:: RosenbrockSolverParameters
   :members:

.. js:autoclass:: BackwardEulerSolverParameters
   :members:

.. js:autoclass:: SolverStats
   :members:

.. js:autoclass:: SolverResult
   :members:

.. js:autoclass:: MICM
   :members:

.. js:autoclass:: State
   :members:

Mechanism Configuration
^^^^^^^^^^^^^^^^^^^^^^^^

Classes for building a mechanism configuration in JavaScript and serializing it
to MUSICA's JSON format via ``getJSON()`` / ``getString()``. Accessible under
``mechanismConfiguration`` (``types``, ``reactionTypes``, and ``Mechanism``).

.. js:autoclass:: Mechanism
   :members:

Core types
""""""""""

.. js:autoclass:: Species
   :members:

.. js:autoclass:: PhaseSpecies
   :members:

.. js:autoclass:: Phase
   :members:

.. js:autoclass:: ReactionComponent
   :members:

Reaction types
""""""""""""""

Every reaction class exposes its mechanism-configuration type string as both a
static property (e.g. ``Arrhenius.type``) and an instance property
(``new Arrhenius(...).type``).

.. js:autoclass:: Arrhenius
   :members:

.. js:autoclass:: Branched
   :members:

.. js:autoclass:: Emission
   :members:

.. js:autoclass:: FirstOrderLoss
   :members:

.. js:autoclass:: Photolysis
   :members:

.. js:autoclass:: Surface
   :members:

.. js:autoclass:: TaylorSeries
   :members:

.. js:autoclass:: Troe
   :members:

.. js:autoclass:: TernaryChemicalActivation
   :members:

.. js:autoclass:: Tunneling
   :members:

.. js:autoclass:: UserDefined
   :members:

.. js:autoclass:: LambdaRateConstant
   :members:

Further Reading
---------------

- :ref:`JavaScript User Guide <javascript-user-guide>`
- `MUSICA npm package <https://www.npmjs.com/package/@ncar/musica>`_
- :doc:`MICM Documentation <micm:index>`
