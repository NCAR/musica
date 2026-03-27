MUSICA JavaScript API
=====================

The JavaScript API exposes MUSICA's MICM chemical kinetics solver through a
WebAssembly (WASM) module, making it usable in both browser and Node.js
environments. The package is available as ``@ncar/musica`` on npm.

Installation
------------

.. code-block:: bash

   npm install @ncar/musica

Or, to build from source with WASM support:

.. code-block:: bash

   git clone https://github.com/NCAR/musica.git
   cd musica
   npm run build

Initialization
--------------

The WASM module must be initialized before any API calls. Call
:js:func:`initModule` once at startup and ``await`` it before proceeding.

.. code-block:: javascript

   import { initModule, MICM, SolverType } from '@ncar/musica';

   await initModule();

   const micm = MICM.fromConfigPath('path/to/config');
   const state = micm.createState();
   // ...

Quick Start
-----------

.. code-block:: javascript

   import { initModule, MICM, SolverType, SolverState } from '@ncar/musica';

   await initModule();

   const micm = MICM.fromConfigPath('configs/v0/analytical');
   const state = micm.createState();

   state.setConditions({ temperatures: 298.0, pressures: 101325.0 });
   state.setConcentrations({ A: 1.0, B: 0.0, C: 0.0 });
   state.setUserDefinedRateParameters({ 'USER.reaction 1': 0.001 });

   const result = micm.solve(state, 60.0);
   console.log(result.state === SolverState.Converged);  // true
   console.log('Steps:', result.stats.number_of_steps);

   const concs = state.getConcentrations();
   console.log('Final [A]:', concs.A[0]);

   state.delete();
   micm.delete();

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

Multi-Grid-Cell Example
-----------------------

.. code-block:: javascript

   import { initModule, MICM, SolverState } from '@ncar/musica';

   await initModule();

   const micm = MICM.fromConfigPath('configs/v0/analytical');
   const state = micm.createState(3);  // 3 independent grid cells

   state.setConcentrations({
     A: [1.0, 2.0, 3.0],
     B: [0.0, 0.0, 0.0],
   });

   state.setConditions({
     temperatures: [298.0, 310.0, 280.0],
     pressures:    [101325.0, 95000.0, 105000.0],
   });

   state.setUserDefinedRateParameters({
     'USER.reaction 1': [0.001, 0.002, 0.003],
     'USER.reaction 2': [0.004, 0.005, 0.006],
   });

   const result = micm.solve(state, 60.0);
   console.assert(result.state === SolverState.Converged);

   const concs = state.getConcentrations();
   console.log('Final [A] per cell:', concs.A);

   state.delete();
   micm.delete();

Building from a Mechanism Object
---------------------------------

Mechanisms can also be constructed in code rather than loaded from a config file:

.. code-block:: javascript

   import { initModule, MICM, mechanismConfiguration } from '@ncar/musica';

   await initModule();

   const { Mechanism, types } = mechanismConfiguration;
   // ... build mechanism object ...

   const micm = MICM.fromMechanism(mechanism);

Additional Resources
--------------------

- `MUSICA npm package <https://www.npmjs.com/package/@ncar/musica>`_
- `MUSICA GitHub Repository <https://github.com/NCAR/musica>`_
- :doc:`MICM Documentation <micm:index>`
