.. _2grid:

Simple 2-Grid-Cell Setup
========================
This page highlights the key differences in your MUSICA workflow set up for multi-grid cell simulations with a toy, 2-grid-cell example.

Defining a system
------------------
As done in the :ref:`Defining chemical systems <chemistry>` page, you first need to define the `Species`,
`Reactions`, and `Mechanisms` of interest::
    
    A = mc.Species(name="A") # Create each of the species with their respective names
    B = mc.Species(name="B")
    C = mc.Species(name="C")
    species = [A, B, C] # Bundle the species into a list
    gas = mc.Phase(name="gas", species=species) # Create a gas phase object containing the species

    r1 = mc.Arrhenius( # Create the reactions with their name, constants, reactants, products, and phase
        name="A_to_B",
        A=4.0e-3,  # Pre-exponential factor
        C=50,      # Activation energy (units assumed to be K)
        reactants=[A],
        products=[B],
        gas_phase=gas
    )

    r2 = mc.Arrhenius(
        name="B_to_C",
        A=4.0e-3,
        C=50,  
        reactants=[B],
        products=[C],
        gas_phase=gas
    )

    mechanism = mc.Mechanism( # Define the mechanism which contains a name, the species, the phases, and reactions
        name="musica_micm_example",
        species=species,
        phases=[gas],
        reactions=[r1, r2]
    )

Creating the solver
--------------------
As done in :ref:`Model solving and options <model>` page, a `solver` must be defined to integrate the chemical reactions
that determine how your mechanism of interest proceeds over time::
    
    solver = musica.MICM(mechanism = mechanism, solver_type = musica.SolverType.rosenbrock_standard_order)

Creating the state (new)
-------------------------
With multiple grid cells, the number of grid cells desired for your simulation must now be passed into the `state`::

    num_grid_cells = 2
    state = solver.create_state(num_grid_cells)

Populating the grid cells (new)
-------------------------------
As opposed to using the individual `condition` variables set up in the previous :ref:`Model solving and options <model>` page,
we recommend the use of arrays to store the `conditions` and `concentrations` desired per grid cell::

    box_model_values = np.array([[300, 101253.3, 5, 5, 5], [100, 11253.3, 20, 3, 7]])
    box_model_values = box_model_values.reshape(-1, 5)

The elements within each array item are ordered as follows:
* temperature (Kelvin),
* pressure (Pascals), and
* the concentrations of each of the 3 species (mol/m<sup>3</sup>).


Splitting up array output (new)
-----------------------------------
Next, values from the box_model_values array are unpacked into variables representing the five model inputs.
Each row corresponds to a grid cell, and each column to a specific variable. These variables are then passed into the `solver’s` state.
For concentrations, MUSICA requires bundling them into a dictionary before passing to `set_concentrations()`.

Finally, an empty array is initialized to store the solved concentrations over time, along with variables defining the time step length, total simulation time, and the current time step (all in seconds).::

    temperatures = box_model_values[:, 0]
    pressures = box_model_values[:, 1]
    concentrations = {
        "A": [],
        "B": [],
        "C": []
    }
    concentrations["A"] = box_model_values[:, 2]
    concentrations["B"] = box_model_values[:, 3]
    concentrations["C"] = box_model_values[:, 4]

    state.set_conditions(temperatures, pressures)
    state.set_concentrations(concentrations)
    concentrations_solved = []
    time_step_length = 1
    sim_length = 60
    curr_time = 0
    
Running the Solver
-------------------
With each grid cell `state` now properly initialized, the `solver` can be run as done in the previous :ref:`box model example <model>`::

    while curr_time <= sim_length:
    solver.solve(state, curr_time)
    concentrations_solved.append(state.get_concentrations())
    curr_time += time_step_length

Preparing and Visualizing Results (new)
---------------------------------------
When running simulations with multiple grid cells, you’ll need to track larger numbers of concentrations and results, which can make data visualization more complex.
For guidance on handling and visualizing outputs from multi-grid-cell simulations, see the `Multiple Grid Cells in MUSICA <../../../tutorials/1.%20multiple_grid_cells.ipynb/>`_
notebook on the :ref:`Interactive Tutorials <tutorials page>` page.