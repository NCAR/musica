.. _julia-user-guide:

Overview
========

The Julia API exposes MICM gas-phase chemistry and is built on top of the C++
library using `CxxWrap.jl <https://github.com/JuliaInterop/CxxWrap.jl>`_. For
installation, see :doc:`../../getting_started/julia`.

Defining a Mechanism
--------------------

Mechanisms are loaded from a :doc:`mechanism configuration file <mc:index>`
directory. Use the bundled ``configs/v0/analytical`` example or provide your own:

.. code-block:: julia

   using MUSICA

   solver = MICM("configs/v0/analytical", SolverType.rosenbrock_standard_order)

Building a Mechanism in Code
----------------------------

Instead of loading a configuration file, you can assemble a version 1 mechanism
configuration directly in Julia using the ``Musica.MechanismConfiguration``
submodule, then serialize it to a JSON or YAML string and hand it to ``MICM``. This
mirrors the Python and JavaScript interfaces.

.. code-block:: julia

   using Musica
   using Musica.MechanismConfiguration

   # Species (molecular weight in kg mol-1). Arbitrary extra properties may be
   # supplied via `other_properties`; they are serialized with a `__` prefix.
   A  = Species(name = "A",  molecular_weight = 0.029,
                other_properties = Dict("long name" => "atom_A"))
   B  = Species(name = "B",  molecular_weight = 0.029)
   AB = Species(name = "AB", molecular_weight = 0.058)

   # A phase groups species. Members may be a `Species`, a `PhaseSpecies`
   # (carrying a diffusion coefficient), or a bare species-name string.
   gas = Phase(name = "gas", species = [A, B, AB])

   # Reactions reference species by name through `ReactionComponent`s.
   forward = UserDefined(
       name = "AB_loss",
       gas_phase = "gas",
       scaling_factor = 2.0e-3,
       reactants = [ReactionComponent(species_name = "AB")],
       products  = [ReactionComponent(species_name = "A"),
                    ReactionComponent(species_name = "B")],
   )
   photo = Photolysis(
       name = "jAB",
       gas_phase = "gas",
       reactants = [ReactionComponent(species_name = "AB")],
       products  = [ReactionComponent(species_name = "A"),
                    ReactionComponent(species_name = "B")],
   )

   mech = Mechanism(
       name = "ABBA", version = "1.0.0",
       species = [A, B, AB], phases = [gas], reactions = [forward, photo],
   )

   # Serialize to a string (JSON or YAML) ...
   json_string = to_json_string(mech)
   yaml_string = to_yaml_string(mech)
   # ... or `to_string(mech; format = :json)` / `:yaml`.

   # ... and build a solver directly from it.
   micm  = MICM(config_string = json_string)
   state = create_state(micm)

The available reaction types are ``Arrhenius``, ``Branched``, ``Emission``,
``FirstOrderLoss``, ``Photolysis``, ``Surface``, ``TaylorSeries``, ``Troe``,
``TernaryChemicalActivation``, ``Tunneling``, and ``UserDefined``.

Species properties stored in the mechanism can be read back from the solver. For
example, to convert a mass mixing ratio (kg kg⁻¹) to a molar concentration
(mol m⁻³) using a species' molecular weight and an ideal-gas air density:

.. code-block:: julia

   temperature = 298.0      # K
   pressure    = 101325.0   # Pa
   M_air       = 0.0289647  # kg mol-1 (dry air)

   # Air density from the ideal gas law: molar [mol m-3] -> mass [kg m-3]
   molar_density    = pressure / (GAS_CONSTANT * temperature)  # mol m-3
   air_mass_density = molar_density * M_air                    # kg m-3

   mw_AB             = get_species_property(micm, "AB", "molecular weight [kg mol-1]", Float64)
   mass_mixing_ratio = 0.6  # kg AB per kg air
   conc_AB           = mass_mixing_ratio * air_mass_density / mw_AB  # mol m-3

Creating a Solver
-----------------

The solver type is the second argument to ``MICM``. Available options via the
``SolverType`` enum:

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - Solver
     - Description
   * - ``SolverType.rosenbrock_standard_order``
     - Rosenbrock solver (standard species ordering)
   * - ``SolverType.rosenbrock_arbitrary_order``
     - Rosenbrock solver (arbitrary species ordering)
   * - ``SolverType.backward_euler_standard_order``
     - Backward Euler solver (standard species ordering)
   * - ``SolverType.backward_euler_arbitrary_order``
     - Backward Euler solver (arbitrary species ordering)

Setting Conditions
------------------

Create a state and set concentrations and environmental conditions:

.. code-block:: julia

   state = create_state(solver)

   set_concentrations!(state, Dict("A" => 1.0, "B" => 3.0, "C" => 5.0))
   set_conditions!(state, 300.0, 101000.0)   # temperature (K), pressure (Pa)

Solving
-------

Call ``solve!`` at each time step:

.. code-block:: julia

   time_step  = 4.0    # s
   sim_length = 20.0   # s
   curr_time  = time_step

   while curr_time <= sim_length
       solve!(solver, state, time_step)
       curr_time += time_step
   end

Accessing Results
-----------------

Retrieve updated concentrations from the state after each solve:

.. code-block:: julia

   conc = get_concentrations(state)
   println("A=$(conc["A"][1])  B=$(conc["B"][1])  C=$(conc["C"][1])")

.. note::

   TUV-x photolysis and CARMA aerosol support are not yet available in the
   Julia API.

Further Reading
---------------

- :doc:`Julia API Reference <../../api/julia>`
- :doc:`MICM Documentation <micm:index>`
- `CxxWrap.jl Documentation <https://github.com/JuliaInterop/CxxWrap.jl>`_
