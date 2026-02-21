.. _LHS:

Initializing Larger Numbers of Grid Cells
==========================================
In the :ref:`2-grid-cell example <2grid>`, all grid cells had to be manually filled in with data.
This is impractical for simulations with larger numbers of grid cells. As a solution, we recommend the use
of `Latin Hyper Cube Sampling (LHS) <https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.qmc.LatinHypercube.html>`_
as a means of randomly generating grid cell conditions.

Defining a system
-----------------
The following is identical to the system and solver set up done in the previous :ref:`2-grid-cell <2-grid>`page::

    A = mc.Species(name="A")
    B = mc.Species(name="B")
    C = mc.Species(name="C")
    species = [A, B, C]
    gas = mc.Phase(name="gas", species=species)

    r1 = mc.Arrhenius(
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

    mechanism = mc.Mechanism(
        name="musica_micm_example",
        species=species,
        phases=[gas],
        reactions=[r1, r2]
    )

    solver = musica.MICM(mechanism = mechanism, solver_type = musica.SolverType.rosenbrock_standard_order)

Creating the state
-------------------
We will now use 100 grid cells for our MUSICA simulation as opposed to the previous 2::

    num_grid_cells = 100
    state = solver.create_state(num_grid_cells)

Note that there is technically no upper limit to the number of grid cells you can create, but computational expense will increase with grid cell count.

Creating and scaling LHS
-------------------------
The LHS initialized here uses the same 5 dimensions as `box_model_values` entries in the previous :ref:`2-grid-cell example <2grid>`:
* temperature (Kelvin),
* pressure (Pascals), and
* the concentrations of each of the 3 species (mol/m<sup>3</sup>).

The LHS is created with these 5 dimensions and a randomized sample that will be scaled by the sampler.
The desired upper and lower bounds for temperature, pressure, and each concentrations are then set, and the sample is scaled with those bounds by the LHS::

    ndim = 5
    nsamples = num_grid_cells

    # Create a Latin Hypercube sampler in the unit hypercube
    sampler = qmc.LatinHypercube(d=ndim)

    # Generate samples
    sample = sampler.random(n=nsamples)

    # Define bounds for each dimension
    l_bounds = [275, 100753.3, 0, 0, 0] # Lower bounds
    u_bounds = [325, 101753.3, 10, 10, 10] # Upper bounds

    # Scale the samples to the defined bounds
    sample_scaled = qmc.scale(sample, l_bounds, u_bounds)

Spliting up array output
------------------------
As done in the previous :ref:`2-grid-cell example <2grid>` example, values from the LHS-generated conditions are unpacked into variables representing the five model inputs::

    temperatures = sample_scaled[:, 0]
    pressures = sample_scaled[:, 1]
    concentrations = {
        "A": [],
        "B": [],
        "C": []
    }
    concentrations["A"] = sample_scaled[:, 2]
    concentrations["B"] = sample_scaled[:, 3]
    concentrations["C"] = sample_scaled[:, 4]

    state.set_conditions(temperatures, pressures)
    state.set_concentrations(concentrations)
    concentrations_solved = []
    time_step_length = 1
    sim_length = 60
    curr_time = 0

Running the solver
------------------
From this point, the `solver` can be solved over each time step of interest as done previously::

    while curr_time <= sim_length:
    solver.solve(state, curr_time)
    concentrations_solved.append(state.get_concentrations())
    curr_time += time_step_length

Preparing and Visualizing Results
-----------------------------------
When running simulations with multiple grid cells, you’ll need to track larger numbers of concentrations and results, which can make data visualization more complex.
For guidance on handling and visualizing outputs from multi-grid-cell simulations, see the `Latin Hypercube Sampling in MUSICA <../../../tutorials/2.%20hypercube.ipynb/>`_
notebook on the :ref:`Interactive Tutorials <tutorials page>` page.