Enabling GPU Solving
====================
MUSICA can be built for utilization with a GPU on a Linux-based, GPU-ready environment.

Creating a GPU Virtual environment
-----------------------------------
Running MUSICA on a GPU requires as different installation protocol when setting up a virtual environment.
To utilize this example code, run the following commands in your terminal to build MUSICA for GPUs:

.. code-block:: console
    
    conda create --name musica_gpu python=3.9
    conda activate musica_gpu
    pip install --upgrade setuptools pip wheel
    pip install nvidia-pyindex
    pip install musica[gpu]
    conda install ipykernel seaborn scipy pandas

Running a Basic solver on a GPU
--------------------------------
This example code primarily follows the page on :ref:`Initializing Larger Numbers of Grid Cells <LHS>`
for its chemical system and `solver` definitions; however, an if statement has been added to verify that you
are running on a GPU::

    if is_cuda_available():
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

        solver = musica.MICM(mechanism = mechanism, solver_type = musica.SolverType.cuda_rosenbrock)

        num_grid_cells = 100
        state = solver.create_state(num_grid_cells)

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

        state.set_conditions(temperatures, pressures)
        state.set_concentrations(concentrations)
        concentrations_solved = []
        time_step_length = 1
        sim_length = 60
        curr_time = 0

        while curr_time <= sim_length:
            solver.solve(state, curr_time)
            concentrations_solved.append(state.get_concentrations())
            curr_time += time_step_length

        def convert_results_all_cells():
            concentrations_solved_pd = []
            time = []
            for i in range(0, sim_length + 1, time_step_length):
                for j in range(0, num_grid_cells):
                    concentrations_solved_pd.append({species: concentration[j] for species, concentration in concentrations_solved[int(i/time_step_length)].items()})
                    time.append(i)
            df = pd.DataFrame(concentrations_solved_pd)
            df = df.rename(columns = {'A' : 'CONC.A.mol m-3', 'B' : 'CONC.B.mol m-3', 'C' : 'CONC.C.mol m-3'})
            df['time.s'] = time
            df['ENV.temperature.K'] = np.repeat(temperatures[0], (sim_length/time_step_length + 1.0) * num_grid_cells)
            df['ENV.pressure.Pa'] = np.repeat(pressures[0], (sim_length/time_step_length + 1.0) * num_grid_cells)
            df['ENV.air number density.mol m-3'] = np.repeat(state.get_conditions()['air_density'][0], (sim_length/time_step_length + 1.0) * num_grid_cells)
            df = df[['time.s', 'ENV.temperature.K', 'ENV.pressure.Pa', 'ENV.air number density.mol m-3', 'CONC.A.mol m-3', 'CONC.B.mol m-3', 'CONC.C.mol m-3']]
            return concentrations_solved_pd, df

        concentrations_solved_pd, df = convert_results_all_cells()

        sns.lineplot(data=df, x='time.s', y='CONC.A.mol m-3', errorbar=('ci', 95), err_kws={'alpha' : 0.4}, label='CONC.A.mol m-3')
        sns.lineplot(data=df, x='time.s', y='CONC.B.mol m-3', errorbar=('ci', 95), err_kws={'alpha' : 0.4}, label='CONC.B.mol m-3')
        sns.lineplot(data=df, x='time.s', y='CONC.C.mol m-3', errorbar=('ci', 95), err_kws={'alpha' : 0.4}, label='CONC.C.mol m-3')
        plt.title('Average concentration with CI over time')
        plt.ylabel('Concentration (mol m-3)')
        plt.xlabel('Time (s)')
        plt.legend(loc='center right')
        plt.show()

        min_y = []
        max_y = []
        for i in range(0, sim_length + 1, time_step_length):
            min_y.append({species: np.min(concentration) for species, concentration in concentrations_solved[int(i/time_step_length)].items()})
            max_y.append({species: np.max(concentration) for species, concentration in concentrations_solved[int(i/time_step_length)].items()})
        time_x = list(map(float, range(0, sim_length + 1, time_step_length)))

        plt.fill_between(time_x, [y['A'] for y in min_y], [y['A'] for y in max_y], alpha = 0.4, label='CONC.A.mol m-3')
        plt.fill_between(time_x, [y['B'] for y in min_y], [y['B'] for y in max_y], alpha = 0.4, label='CONC.B.mol m-3')
        plt.fill_between(time_x, [y['C'] for y in min_y], [y['C'] for y in max_y], alpha = 0.4, label='CONC.C.mol m-3')
        plt.title('Concentration range over time')
        plt.ylabel('Concentration (mol m-3)')
        plt.xlabel('Time (s)')
        plt.legend()
        plt.show()
    else:
        print("Error: No GPU Available")
