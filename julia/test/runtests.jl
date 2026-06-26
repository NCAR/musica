using Test
using Musica

@testset "Musica.jl Tests" begin
    @testset "Version" begin
        musica_version = Musica.get_musica_version()
        micm_version = Musica.get_micm_version()
        @test musica_version isa AbstractString
        @test micm_version isa AbstractString
        @test !isempty(musica_version)
        @test !isempty(micm_version)
        # Test version format (semantic versioning: X.Y.Z or X.Y.Z.W)
        @test occursin(r"^\d+\.\d+\.\d+", musica_version)
        @test occursin(r"^\d+\.\d+\.\d+", micm_version)
        println("MUSICA version: ", musica_version)
        println("MICM version: ", micm_version)
    end

    @testset "CUDA availability" begin
        @test Musica.cpp_is_cuda_available() isa Bool
    end

    @testset "Constants" begin
        @test AVOGADRO ≈ 6.02214076e23
        @test BOLTZMANN ≈ 1.380649e-23
        @test GAS_CONSTANT ≈ AVOGADRO * BOLTZMANN
    end

    @testset "SolverType enum" begin
        @test Int(Rosenbrock) == 1
        @test Int(RosenbrockStandardOrder) == 2
        @test Int(BackwardEuler) == 3
        @test Int(BackwardEulerStandardOrder) == 4
        @test Int(CudaRosenbrock) == 5
    end

    @testset "SolverState enum" begin
        @test Int(NotYetCalled) == 0
        @test Int(Converged) == 2
        @test Int(NaNDetected) == 6
    end

    @testset "Conditions" begin
        c = Conditions(temperature = 298.0, pressure = 101325.0)
        @test c.temperature ≈ 298.0
        @test c.pressure ≈ 101325.0
        @test c.air_density ≈ 101325.0 / (GAS_CONSTANT * 298.0) atol=1e-6

        c2 = Conditions(temperature = 300.0, pressure = 100000.0, air_density = 42.0)
        @test c2.air_density ≈ 42.0
    end

    @testset "RosenbrockSolverParameters" begin
        params = RosenbrockSolverParameters()
        @test params.relative_tolerance ≈ 1e-6
        @test params.absolute_tolerances === nothing
        @test params.h_min ≈ 0.0
        @test params.h_max ≈ 0.0
        @test params.h_start ≈ 0.0
        @test params.max_number_of_steps == 1000

        params2 = RosenbrockSolverParameters(
            relative_tolerance = 1e-8,
            h_start = 1e-5,
            max_number_of_steps = 500,
        )
        @test params2.relative_tolerance ≈ 1e-8
        @test params2.h_start ≈ 1e-5
        @test params2.max_number_of_steps == 500
    end

    @testset "BackwardEulerSolverParameters" begin
        params = BackwardEulerSolverParameters()
        @test params.relative_tolerance ≈ 1e-6
        @test params.max_number_of_steps == 11
        @test length(params.time_step_reductions) == 5

        @test_throws ErrorException BackwardEulerSolverParameters(
            time_step_reductions = [0.5, 0.5, 0.5],
        )
    end

    # Config path for the analytical test mechanism
    config_path = joinpath(@__DIR__, "..", "..", "configs", "v0", "analytical")

    @testset "MICM creation" begin
        micm = MICM(config_path = config_path)
        @test solver_type(micm) == RosenbrockStandardOrder

        micm2 = MICM(config_path = config_path, solver_type = BackwardEulerStandardOrder)
        @test solver_type(micm2) == BackwardEulerStandardOrder
    end

    @testset "State management" begin
        micm = MICM(config_path = config_path)
        state = create_state(micm)

        # Check species ordering
        ordering = get_species_ordering(state)
        @test haskey(ordering, "A")
        @test haskey(ordering, "B")
        @test haskey(ordering, "C")
        @test haskey(ordering, "D")
        @test haskey(ordering, "E")
        @test haskey(ordering, "F")

        # Set and get concentrations
        set_concentrations!(
            state,
            Dict{String,Any}(
                "A" => 1.0,
                "B" => 0.0,
                "C" => 0.0,
                "D" => 1.0,
                "E" => 0.0,
                "F" => 0.0,
            ),
        )
        concs = get_concentrations(state)
        @test concs["A"][1] ≈ 1.0
        @test concs["B"][1] ≈ 0.0
        @test concs["D"][1] ≈ 1.0

        # Set and get conditions
        set_conditions!(state, temperatures = 298.0, pressures = 101325.0)
        conds = get_conditions(state)
        @test conds["temperature"][1] ≈ 298.0
        @test conds["pressure"][1] ≈ 101325.0
        @test conds["air_density"][1] ≈ 101325.0 / (GAS_CONSTANT * 298.0) atol=1e-6

        # Check rate parameter ordering
        rate_ordering = get_user_defined_rate_parameters_ordering(state)
        @test haskey(rate_ordering, "USER.reaction 1")
        @test haskey(rate_ordering, "USER.reaction 2")

        # Set and get user-defined rate parameters
        set_user_defined_rate_parameters!(
            state,
            Dict{String,Any}("USER.reaction 1" => 0.001, "USER.reaction 2" => 0.002),
        )
        rate_params = get_user_defined_rate_parameters(state)
        @test rate_params["USER.reaction 1"][1] ≈ 0.001
        @test rate_params["USER.reaction 2"][1] ≈ 0.002
    end

    @testset "Solve" begin
        micm = MICM(config_path = config_path)
        state = create_state(micm)

        set_concentrations!(
            state,
            Dict{String,Any}(
                "A" => 1.0,
                "B" => 0.0,
                "C" => 0.0,
                "D" => 1.0,
                "E" => 0.0,
                "F" => 0.0,
            ),
        )
        set_conditions!(state, temperatures = 298.0, pressures = 101325.0)
        set_user_defined_rate_parameters!(
            state,
            Dict{String,Any}("USER.reaction 1" => 0.001, "USER.reaction 2" => 0.002),
        )

        result = solve!(micm, state, 60.0)
        @test result.state == Converged
        @test result.stats.number_of_steps > 0
        @test result.stats.final_time ≈ 60.0

        # Check that concentrations changed
        concs = get_concentrations(state)
        @test concs["A"][1] < 1.0  # A should decrease
    end

    @testset "Solver parameters round-trip" begin
        micm = MICM(config_path = config_path)

        params = get_solver_parameters(micm)
        @test params isa RosenbrockSolverParameters

        new_params =
            RosenbrockSolverParameters(relative_tolerance = 1e-8, max_number_of_steps = 500)
        set_solver_parameters!(micm, new_params)

        retrieved = get_solver_parameters(micm)
        @test retrieved.relative_tolerance ≈ 1e-8
        @test retrieved.max_number_of_steps == 500
    end

    @testset "Multi-grid-cell state" begin
        micm = MICM(config_path = config_path)
        n_cells = 3
        state = create_state(micm, number_of_grid_cells = n_cells)

        ordering = get_species_ordering(state)
        @test length(ordering) > 0

        # Set concentrations with per-cell vectors
        set_concentrations!(
            state,
            Dict{String,Any}(
                "A" => [1.0, 2.0, 3.0],
                "B" => [0.0, 0.0, 0.0],
                "C" => [0.0, 0.0, 0.0],
                "D" => [1.0, 1.0, 1.0],
                "E" => [0.0, 0.0, 0.0],
                "F" => [0.0, 0.0, 0.0],
            ),
        )
        concs = get_concentrations(state)
        @test concs["A"] ≈ [1.0, 2.0, 3.0]
        @test concs["D"] ≈ [1.0, 1.0, 1.0]

        # Set conditions with per-cell vectors
        set_conditions!(
            state,
            temperatures = [298.0, 310.0, 280.0],
            pressures = [101325.0, 95000.0, 105000.0],
        )
        conds = get_conditions(state)
        @test length(conds["temperature"]) == n_cells
        @test conds["temperature"] ≈ [298.0, 310.0, 280.0]
        @test conds["pressure"] ≈ [101325.0, 95000.0, 105000.0]
        for i = 1:n_cells
            @test conds["air_density"][i] ≈
                  conds["pressure"][i] / (GAS_CONSTANT * conds["temperature"][i]) atol=1e-6
        end

        # Set user-defined rate parameters with per-cell vectors
        set_user_defined_rate_parameters!(
            state,
            Dict{String,Any}(
                "USER.reaction 1" => [0.001, 0.002, 0.003],
                "USER.reaction 2" => [0.004, 0.005, 0.006],
            ),
        )
        rate_params = get_user_defined_rate_parameters(state)
        @test rate_params["USER.reaction 1"] ≈ [0.001, 0.002, 0.003]
        @test rate_params["USER.reaction 2"] ≈ [0.004, 0.005, 0.006]

        # Solve with multi-grid-cell state
        result = solve!(micm, state, 60.0)
        @test result.state == Converged

        # Check that concentrations changed per cell
        concs_after = get_concentrations(state)
        for i = 1:n_cells
            @test concs_after["A"][i] < concs["A"][i]
        end
    end

    @testset "Error handling" begin
        micm = MICM(config_path = config_path)
        state = create_state(micm)

        # Unknown species name
        @test_throws ErrorException set_concentrations!(
            state,
            Dict{String,Any}("NONEXISTENT_SPECIES" => 1.0),
        )

        # Unknown rate parameter name
        @test_throws ErrorException set_user_defined_rate_parameters!(
            state,
            Dict{String,Any}("NONEXISTENT_PARAM" => 1.0),
        )

        # Wrong vector length for concentrations
        @test_throws ErrorException set_concentrations!(
            state,
            Dict{String,Any}(
                "A" => [1.0, 2.0],  # state has 1 grid cell, providing 2 values
            ),
        )

        # Wrong vector length for rate parameters
        @test_throws ErrorException set_user_defined_rate_parameters!(
            state,
            Dict{String,Any}("USER.reaction 1" => [1.0, 2.0]),
        )

        # Wrong vector length for conditions
        @test_throws ErrorException set_conditions!(state, temperatures = [298.0, 300.0])

        # Invalid config path
        @test_throws Exception MICM(config_path = "/nonexistent/path/to/config")

        # Division-by-zero guard: zero temperature should yield air_density = 0.0, not Inf
        set_conditions!(state, temperatures = 0.0, pressures = 101325.0)
        conds = get_conditions(state)
        @test conds["air_density"][1] == 0.0

        # Division-by-zero guard: zero pressure should also yield air_density = 0.0
        set_conditions!(state, temperatures = 298.0, pressures = 0.0)
        conds = get_conditions(state)
        @test conds["air_density"][1] == 0.0
    end

    @testset "Solver parameters round-trip (Backward Euler)" begin
        micm = MICM(config_path = config_path, solver_type = BackwardEulerStandardOrder)

        params = get_solver_parameters(micm)
        @test params isa BackwardEulerSolverParameters

        new_params = BackwardEulerSolverParameters(
            relative_tolerance = 1e-4,
            max_number_of_steps = 20,
        )
        set_solver_parameters!(micm, new_params)

        retrieved = get_solver_parameters(micm)
        @test retrieved.relative_tolerance ≈ 1e-4
        @test retrieved.max_number_of_steps == 20
    end

    @testset "Species properties" begin
        # Mirrors the C++ GetSpeciesProperty test (src/test/unit/micm/micm_c_api.cpp),
        # using the v1 chapman example mechanism. O3 defines one property per type.
        v1_config_path =
            joinpath(@__DIR__, "..", "..", "configs", "v1", "chapman", "config.json")
        micm = MICM(config_path = v1_config_path)

        @test get_species_property(micm, "O3", "__long name", String) == "ozone"
        @test get_species_property(micm, "O3", "molecular weight [kg mol-1]", Float64) ≈
              0.048
        @test get_species_property(micm, "O3", "__atoms", Int) == 3
        @test get_species_property(micm, "O3", "__do advect", Bool) == true

        # Unknown species and unknown property should raise.
        @test_throws Exception get_species_property(micm, "bad species", "__atoms", Int)
        @test_throws Exception get_species_property(micm, "O3", "bad property", Float64)
    end

    @testset "MICM from config string" begin
        # Configure a solver from an in-memory mechanism string (mirrors the
        # JavaScript MICM.fromConfigString path). A simple A -> B mechanism driven
        # by a user-defined rate, written as both JSON and YAML.
        json_mechanism = """
        {
            "name": "A to B",
            "version": "1.0.0",
            "species": [ { "name": "A" }, { "name": "B" } ],
            "phases": [
                { "name": "gas", "species": [ { "name": "A" }, { "name": "B" } ] }
            ],
            "reactions": [
                {
                    "type": "USER_DEFINED",
                    "name": "loss",
                    "gas phase": "gas",
                    "reactants": [ { "species name": "A", "coefficient": 1.0 } ],
                    "products": [ { "species name": "B", "coefficient": 1.0 } ]
                }
            ]
        }
        """

        yaml_mechanism = """
        name: A to B
        version: "1.0.0"
        species:
          - name: A
          - name: B
        phases:
          - name: gas
            species:
              - name: A
              - name: B
        reactions:
          - type: USER_DEFINED
            name: loss
            gas phase: gas
            reactants:
              - species name: A
                coefficient: 1.0
            products:
              - species name: B
                coefficient: 1.0
        """

        for config_string in (json_mechanism, yaml_mechanism)
            micm = MICM(config_string = config_string)
            @test solver_type(micm) == RosenbrockStandardOrder

            state = create_state(micm)
            ordering = get_species_ordering(state)
            @test haskey(ordering, "A")
            @test haskey(ordering, "B")

            rate_ordering = get_user_defined_rate_parameters_ordering(state)
            @test haskey(rate_ordering, "USER.loss")

            set_concentrations!(state, Dict{String,Any}("A" => 1.0, "B" => 0.0))
            set_conditions!(state, temperatures = 298.0, pressures = 101325.0)
            set_user_defined_rate_parameters!(state, Dict{String,Any}("USER.loss" => 0.01))

            result = solve!(micm, state, 60.0)
            @test result.state == Converged

            concs = get_concentrations(state)
            @test concs["A"][1] < 1.0   # A is consumed
            @test concs["B"][1] > 0.0   # B is produced
        end

        # Exactly one of config_path / config_string must be provided.
        @test_throws ErrorException MICM()
        @test_throws ErrorException MICM(
            config_path = config_path,
            config_string = json_mechanism,
        )

        # An invalid mechanism string should raise.
        @test_throws Exception MICM(config_string = "not a valid mechanism")
    end
end
