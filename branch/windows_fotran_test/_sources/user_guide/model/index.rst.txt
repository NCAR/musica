.. _model:

Model solving and options
==========================
.. note::
    
    MUSICA uses the Model-Independent Chemical Module (MICM) as its core chemistry solver. For more information about available reaction types,
    species configuration, and solver behavior, see the `MICM documentation <https://ncar.github.io/micm/index.html>`_.

To work with a MUSICA model and solve, please be sure to import the following::

    import musica

This section details the components of a MICM solver, conditions, and solutions. To initialize a MICM solver, 
the previously defined in-code mechanism (see :ref:`Defining chemical systems <chemistry>`) can be used as follows::
    
    solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)

While the Rosenbrock Standard Order solver was used here, several other types of solvers are made available :doc:`through MICM <micm:user_guide/solver_configurations>`.

Conditions
-----------
Use the `state` to set environmental parameters and species concentrations (:math:`\textsf{mol m}^{-3}`). Parameters such as 
temperature (K), pressure (Pa), and optionally air density define the environment that the mechanism takes place in at the start of the simulation.
Without an air density provided, the Ideal Gas Law is used to calculate this parameter. The initial concentrations of each
Species is also included::
    
    temperature=300.0
    pressure=101000.0
    state = solver.create_state()
    state.set_concentrations({"X": 1.0, "Y": 3.0, "Z": 5.0})
    state.set_conditions(temperature, pressure)

Time Parameters
---------------
Each MUSICA model should be run for a given amount of time (seconds) and that time should be looped through in a desired time step::

    time_step = 4  # stepping
    sim_length = 20  # total simulation time

For further descriptions of these MusicBox attributes, please see the :doc:`API Reference <mb:api/index>`.

Solving
--------
Once all components of the desired model have been defined, it can be solved by calling `solve` on the solver created at each time step::

    curr_time = time_step
    while curr_time <= sim_length:
        solver.solve(state, time_step)
        concentrations = state.get_concentrations()
        curr_time += time_step

To store and/or visualize the results of this solver, please see the :ref:`Output and Visualization <output>` section of this User Guide.
