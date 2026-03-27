.. _javascript-user-guide:

Overview
========

The JavaScript API exposes MUSICA's MICM chemical kinetics solver through a
WebAssembly (WASM) module, usable in both browser and Node.js environments.
It is designed primarily to support the MUSICA box model web application.

For installation, see :doc:`../../getting_started/javascript`.

Initialization
--------------

The WASM module must be initialized once before any API calls:

.. code-block:: javascript

   import { initModule, MICM, SolverType } from '@ncar/musica';

   await initModule();

Defining a Mechanism
--------------------

Mechanisms are loaded from a :doc:`mechanism configuration file <mc:index>`
directory on the file system (or bundled with the application):

.. code-block:: javascript

   const micm = MICM.fromConfigPath('configs/v0/analytical');

Mechanisms can also be constructed programmatically from a mechanism object:

.. code-block:: javascript

   import { initModule, MICM, mechanismConfiguration } from '@ncar/musica';

   await initModule();

   const { Mechanism, types } = mechanismConfiguration;
   // ... build mechanism object ...
   const micm = MICM.fromMechanism(mechanism);

Creating a Solver
-----------------

The solver type is set when creating a ``MICM`` instance. The default is
``rosenbrock_standard_order``:

.. code-block:: javascript

   import { MICM, SolverType } from '@ncar/musica';

   const micm = MICM.fromConfigPath('configs/v0/analytical');
   // Solver type can be set via fromMechanism; fromConfigPath uses the default.

See the :doc:`../../api/javascript` for all available ``SolverType`` values.

Setting Conditions
------------------

Create a state and set temperature, pressure, concentrations, and any
user-defined rate parameters:

.. code-block:: javascript

   const state = micm.createState();

   state.setConditions({ temperatures: 298.0, pressures: 101325.0 });
   state.setConcentrations({ A: 1.0, B: 0.0, C: 0.0 });
   state.setUserDefinedRateParameters({ 'USER.reaction 1': 0.001 });

Solving
-------

Call ``solve`` with the state and a time step (seconds). Check the returned
``SolverResult`` to confirm convergence:

.. code-block:: javascript

   import { SolverState } from '@ncar/musica';

   const result = micm.solve(state, 60.0);

   if (result.state !== SolverState.Converged) {
     console.warn('Solver did not converge:', result.state);
   }

Accessing Results
-----------------

Retrieve updated concentrations from the state after solving:

.. code-block:: javascript

   const concs = state.getConcentrations();
   console.log('Final [A]:', concs.A[0]);

Always release WASM objects when done to avoid memory leaks:

.. code-block:: javascript

   state.delete();
   micm.delete();

Multiple Grid Cells
-------------------

Pass the number of grid cells to ``createState`` and provide per-cell arrays
for conditions and concentrations:

.. code-block:: javascript

   const state = micm.createState(3);   // 3 independent grid cells

   state.setConcentrations({ A: [1.0, 2.0, 3.0], B: [0.0, 0.0, 0.0] });
   state.setConditions({
     temperatures: [298.0, 310.0, 280.0],
     pressures:    [101325.0, 95000.0, 105000.0],
   });
   state.setUserDefinedRateParameters({
     'USER.reaction 1': [0.001, 0.002, 0.003],
   });

   const result = micm.solve(state, 60.0);
   const concs = state.getConcentrations();
   console.log('Final [A] per cell:', concs.A);

   state.delete();
   micm.delete();

.. note::

   TUV-x photolysis and CARMA aerosol support are not available in the
   JavaScript API.

Further Reading
---------------

- :doc:`JavaScript API Reference <../../api/javascript>`
- `MUSICA npm package <https://www.npmjs.com/package/@ncar/musica>`_
- :doc:`MICM Documentation <micm:index>`
