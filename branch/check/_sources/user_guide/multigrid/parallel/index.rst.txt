Parallelizing Larger Numbers of Grid Cells
===========================================
As users grow more familiar with MUSICA, they may wish to increase the complexity of their simulations by
significantly increasing the number of grid cells used. By default, MUSICA executes on a single CPU core.
Here, we will demonstrate how to use `Dask <https://docs.dask.org/en/stable/>`_ to paralellize multiple grid cell
simulations across multiple CPUs.

.. note::
    The workflow provided here uses `NCAR's Casper HPC <https://ncar-hpc-docs.readthedocs.io/en/latest/compute-systems/casper/>`_.
    Users may find it helpful to first practice parallization locally, even without performance improvement. A proof-of-concept local
    parallelization notebook is provided with the :ref:`Interactive Tutorials <tutorials page>`.

Dask cluster set up
--------------------
In order to use Dask, we first need to set up a Dask Cluster object. Here, we use the `Dask PBSCluster <https://jobqueue.dask.org/en/latest/generated/dask_jobqueue.PBSCluster.html>`_
as our HPC system uses a PBS-based scheduled. However, Dask also provides an equivalent `SLURMCluster class <https://jobqueue.dask.org/en/latest/generated/dask_jobqueue.SLURMCluster.html>`_.
When initializing the cluster, you can specify arguments similar to those in your job scripts—such as memory, cores, and walltime. Adjust these to match your workload and system. As a guideline,
we find simulations with ~10,000 grid cells typically run well with about 8 GB of memory, while larger simulations (100,000–1,000,000 cells) may need around 15 GB.
It’s often useful to request a bit more memory, as shown below (10 GB)::
    
    cluster = PBSCluster(
    job_name = 'dask-test',
    cores = 1,
    memory = '10GiB',
    processes = 1,
    local_directory = '/local_scratch/pbs.$PBS_JOBID/dask/spill',
    resource_spec = 'select=1:ncpus=1:mem=10GB', #memory and resource especially memory should match
    queue = 'casper',
    walltime = '50:00',
    interface = 'ext'
)

For further scaling test results, please refer to the HPC parallelization notebook.

Checking your Dask configuration
---------------------------------
Before launcing Dask `workers` on your HPC system, it can be helpful to double-check any existing job configurations
along with the job scrip that would be used for each `worker`::

    from dask import config
    config.refresh()
    config.get('jobqueue.pbs')

    print(cluster.job_script())

Dask dashboard and launching workers
-------------------------------------
A convenient feature of Dask is its interactive dashboard which allows users to visualize the distribution and progress
of their parallelized work. This dashboard can be accessed via the `Client`::
    
    client = Client(cluster)
    client

With the `Cluster` and dashboard set up, you can now scale to your desired number of workers::

    cluster.scale(2)
    client.wait_for_workers(2) # wait to launch jobs until all workers needed are available

Setting up multiple grid cells
-------------------------------
As done in the previous Latin Hypercube Sampling page, here we initialize our desired number of grid
cells with their temperatures, pressures, and `Species` concentrations::

    num_grid_cells = 10000

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

    concentrations_solved = []
    time_step_length = 1
    sim_length = 60
    curr_time = 0

Note that, with Dask, this step happens prior to our `Mechanism` definitions.

Creating a delayed Dask function
---------------------------------
With Dask, the chemistry system is defined inside a delayed function that runs only when triggered to compute.
This approach is necessary because Dask requires pickleable objects, and our chemistry objects are not currently pickleable by default.
Defining them within the delayed function ensures compatibility::

    @delayed
    def solve_one_cell(cell_index,temperatures,pressures,concentrations, sim_length, time_step):

        # Define the system

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


        #create the solver
        solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)

        #create the state
        state = solver.create_state(1)
        state.set_conditions(temperatures[cell_index],pressures[cell_index])
        cur_concentrations = {key: value[cell_index] for key, value in concentrations.items()}
        state.set_concentrations(cur_concentrations)

        time = 0.0
        result = []
        track_time = []
        while time <= sim_length:
            solver.solve(state, time)
            result.append(state.get_concentrations().copy())
            track_time.append(time)
            time += time_step

        return {
            "times": np.array(track_time),
            "concentrations": np.stack(result)
        }


Batch solving with Dask
------------------------
Since MUSICA already computes individual grid cells efficiently, the main performance gain from Dask comes from batching - or grouping multiple grid cells so each CPU core processes several at once.
This approach maximizes core usage, reduces overhead, and improves throughput for large simulations.
Batching can be implemented as follows::

    @delayed
    def solve_batch(start_idx, end_idx, temperatures, pressures, concentrations, sim_length, time_step):
        batch_size = end_idx - start_idx

        # define the system
        A = mc.Species(name="A")
        B = mc.Species(name="B")
        C = mc.Species(name="C")
        species = [A, B, C]
        gas = mc.Phase(name="gas", species=species)

        r1 = mc.Arrhenius(name="A_to_B", A=4.0e-3, C=50, reactants=[A], products=[B], gas_phase=gas)
        r2 = mc.Arrhenius(name="B_to_C", A=4.0e-3, C=50, reactants=[B], products=[C], gas_phase=gas)

        mechanism = mc.Mechanism(
            name="musica_micm_example",
            species=species,
            phases=[gas],
            reactions=[r1, r2]
        )

        # create state for all cells in this batch
        solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)
        state = solver.create_state(batch_size)

        # set conditions and concentrations
        batch_temps = temperatures[start_idx:end_idx]
        batch_pressures = pressures[start_idx:end_idx]
        batch_air_densities = []

        state.set_conditions(batch_temps, batch_pressures)
        air_density = state.get_conditions()['air_density'] # track for later visualization
        batch_air_densities.append(air_density)

        # set concentrations: need shape (batch_size,) for each species
        for species_name, all_values in concentrations.items():
            # slice this batch
            state.set_concentrations({species_name: all_values[start_idx:end_idx]})

        # time stepping
        time = 0.0
        batch_results = []

        while time <= sim_length:
            solver.solve(state, time)
            # get concentrations for all cells at this timestep
            concs = state.get_concentrations().copy() 
            batch_results.append(concs)
            time += time_step

        return np.stack(batch_results), np.array(batch_air_densities)

    batch_size = 100 #number of grid cells solved on a single worker

    tasks = []
    for start_idx in range(0, num_grid_cells, batch_size):
        end_idx = min(start_idx + batch_size, num_grid_cells)
        task = solve_batch(
            start_idx, end_idx,
            temperatures, pressures, concentrations,
            sim_length, time_step_length
        )
        tasks.append(task)
    results_and_densities = compute(*tasks)

Preparing and visualizing results
----------------------------------
As previously mentioned throughout this guide, multiple grid cells calculations - particularly at this scale - tracks significantly
larger numbers of concentrations and results than a box model. For guidance on handling and visualizing outputs from multi-grid-cell simulations,
see the `Parallelizing Multiple Grid Cells on a High-Performance Computing Cluster <../../../tutorials/5.%20hpc_parallelization.ipynb/>`_ notebook on the
:ref:`Interactive Tutorials <tutorials page>` page.