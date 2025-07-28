.. _chapter3:

Chapter 3
=========

MICM Box Model with Multiple Grid Cells in Fortran
--------------------------------------------------

In this third MUSICA Fortran example,
we will extend the MICM box model from Chapter 2 to handle multiple grid cells
in a single call to the MICM solver.
This demonstrates how MUSICA can efficiently solve chemical mechanisms
for multiple independent air masses simultaneously.

Each grid cell represents an independent set of well-mixed air masses,
allowing us to model different atmospheric conditions (temperature, pressure, initial concentrations)
and observe how the same chemical mechanism evolves differently under various conditions.

We will use the same analytical configuration files from Chapter 2
(``config.json``, ``species.json``, and ``reactions.json``)
saved in the subdirectory ``configs/analytical`` or ``configs/v0/analytical``.

For reference, these files define:

- A system of six chemical species: A, B, C, D, E, and F
- Four reactions including two Arrhenius-type reactions and two user-defined rate reactions
- The same mechanism will be solved simultaneously across multiple grid cells

To create a multiple grid cells box model, save the following Fortran code to a file named ``micm_multiple_grid_cells.F90``: 

  .. literalinclude:: ../../../fortran/test/fetch_content_integration/test_micm_multiple_grid_cells.F90
    :language: f90

Key differences from the single grid cell example in Chapter 2:

**Grid Cell Configuration:**

- ``num_grid_cells`` is set to 3 instead of 1
- Each grid cell can have different environmental conditions (temperature, pressure)
- Each grid cell can start with different initial concentrations

**State Management:**

The ``state`` object now manages data for multiple grid cells using strides:

- ``state%species_strides%variable`` provides the stride for accessing species data across cells
- ``state%rate_parameters_strides%variable`` provides the stride for rate parameter data
- Concentrations are stored in a flat array with stride-based indexing

**Setting Conditions and Initial Values:**

.. code-block:: fortran

  ! Different temperatures for each cell
  do cell_id = 1, num_grid_cells
    state%conditions(cell_id)%temperature = 273.0 + (cell_id - 1) * 10.0
    state%conditions(cell_id)%pressure    = 1.0e5
    state%conditions(cell_id)%air_density = state%conditions(cell_id)%pressure / &
                                           (GAS_CONSTANT * state%conditions(cell_id)%temperature)
  end do

**Accessing Multi-Cell Data:**

To access concentration data for a specific species in a specific grid cell:

.. code-block:: fortran

  var_stride = state%species_strides%variable
  cell_concentration = state%concentrations((cell_id - 1) + species_index * var_stride)

**Simultaneous Solving:**

The key advantage is that a single call to ``micm%solve`` processes all grid cells:

.. code-block:: fortran

  call micm%solve(time_step, state, solver_state, solver_stats, error)

This is much more efficient than calling the solver separately for each grid cell,
especially when dealing with large numbers of atmospheric grid cells in climate or weather models.

To build and run the example, follow the same build instructions as in Chapter 1 and 2.
The compilation command would be:

.. code-block:: bash

  gfortran -o micm_multiple_grid_cells micm_multiple_grid_cells.F90 -I<MUSICA_DIR>/include -L<MUSICA_DIR>/lib64 -lmusica-fortran -lmusica -lstdc++ -lyaml-cpp

Assuming you name the executable ``micm_multiple_grid_cells``, you can run the program as follows:

.. code-block:: bash

  $ ./micm_multiple_grid_cells
 Creating MICM solver with           3 grid cells...
 Creating State for multiple grid cells...
 Species in the mechanism:
 Species Name:A, Index:           1
 Species Name:B, Index:           2
 Species Name:C, Index:           5
 Species Name:D, Index:           3
 Species Name:E, Index:           4
 Species Name:F, Index:           6

 Initial concentrations by grid cell:
 Grid Cell 1 (T= 273.0K):
    1.000   1.000   1.000   1.000   1.000   1.000
 Grid Cell 2 (T= 283.0K):
    2.000   2.000   2.000   2.000   2.000   2.000
 Grid Cell 3 (T= 293.0K):
    0.500   0.500   0.500   0.500   0.500   0.500

 Solving for all grid cells simultaneously...

 Final concentrations by grid cell:
 Grid Cell 1 (T= 273.0K):
    0.382   1.468   0.670   1.116   1.150   1.214
 Grid Cell 2 (T= 283.0K):
    0.751   2.462   1.234   2.105   2.201   2.347
 Grid Cell 3 (T= 293.0K):
    0.195   0.831   0.381   0.602   0.625   0.666

 Solver completed successfully for all           3 grid cells!
  $

**Analysis of Results:**

Notice how each grid cell evolves differently:

- **Grid Cell 1** (273K): Starting with concentrations of 1.0, the cooler temperature leads to slower reaction rates
- **Grid Cell 2** (283K): Starting with higher concentrations (2.0), the moderate temperature produces intermediate reaction rates  
- **Grid Cell 3** (293K): Starting with lower concentrations (0.5), the warmer temperature leads to faster reaction rates

The chemical mechanism responds to both the initial concentrations and the temperature conditions,
demonstrating how MUSICA can handle realistic atmospheric variability across multiple air masses
in a single, efficient computation.

This multiple grid cell approach is essential for atmospheric modeling applications
where hundreds or thousands of grid cells need to be processed simultaneously
while maintaining computational efficiency.
