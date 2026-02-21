Examples and tutorials
===============================

Running premade examples
-------------------------
MUSICA provides a selection of pre-made example configurations for the user to work with:

* `Analytical <https://github.com/NCAR/musica/tree/main/configs/v0/analytical>`_
* `CarbonBond5 <https://github.com/NCAR/musica/tree/main/configs/v0/carbon_bond_5>`_
* `Chapman <https://github.com/NCAR/musica/tree/main/configs/v0/chapman>`_
* `TS1 <https://github.com/NCAR/musica/tree/main/configs/v0/robertson>`_

Each example (found in `configs/v0`) includes an associated set of JSON files acccessible through the a MICM `solver`::

    import musica

    solver = musica.MICM(
        config_path="/configs/v0/analytical",
        solver_type=musica.SolverType.rosenbrock_standard_order)
    state = solver.create_state()

    #set conditions
    temperature = 272.5
    pressure = 101253.3
    concentrations = {
        "A": 0.75,
        "B": 0,
        "C": 0.4,
        "D": 0.8,
        "E": 0,
        "F": 0.1
    }

    state.set_conditions(temperature, pressure)
    state.set_concentrations(concentrations)

    #solve
    sim_length =  100
    time_step = 1
    curr_time = time_step
    while curr_time <= sim_length:
        solver.solve(state, time_step)
        concentrations = state.get_concentrations()
        print(concentrations)
        curr_time += time_step

Interactive tutorial notebooks
-------------------------------
Looking for hands on examples of the concepts covered in this guide? Explore our :ref:`Tutorials <tutorials page>` page for detailed, interactive walkthroughs.





