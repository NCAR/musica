# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

# Tests for the in-code mechanism configuration interface
# (`Musica.MechanismConfiguration`).
#
# Builds the "ABBA" mechanism entirely in code, checks JSON/YAML serialization, then
# runs a box model from the serialized string and uses a species' molecular weight to
# convert a mass mixing ratio to a molar concentration.

using Test
using Musica
using Musica.MechanismConfiguration
using JSON
using YAML

# ── Build the ABBA mechanism in code ────────────────────────────────────────
# Molecular weights use the standard `molecular weight [kg mol-1]` field.
A = Species(
    name = "A",
    molecular_weight = 0.029,
    other_properties = Dict(
        "absolute tolerance" => 1e-4,
        "long name" => "atom_A",
        "do advect" => true,
        "initial concentration" => 0.0,
    ),
)
B = Species(
    name = "B",
    molecular_weight = 0.029,
    other_properties = Dict("absolute tolerance" => 1e-4, "long name" => "atom_B"),
)
AB = Species(
    name = "AB",
    molecular_weight = 0.058,
    other_properties = Dict("long name" => "molecule_AB"),
)

gas = Phase(name = "gas", species = [A, B, AB])

forward = UserDefined(
    name = "forward_AB_to_A_B",
    gas_phase = "gas",
    scaling_factor = 2.0e-3,
    reactants = [ReactionComponent(species_name = "AB")],
    products = [
        ReactionComponent(species_name = "A"),
        ReactionComponent(species_name = "B"),
    ],
)
reverse_rxn = UserDefined(
    name = "reverse_A_B_to_AB",
    gas_phase = "gas",
    scaling_factor = 1.0e-3,
    reactants = [
        ReactionComponent(species_name = "A"),
        ReactionComponent(species_name = "B"),
    ],
    products = [ReactionComponent(species_name = "AB")],
)
photo = Photolysis(
    name = "jNO2",
    gas_phase = "gas",
    reactants = [ReactionComponent(species_name = "AB")],
    products = [
        ReactionComponent(species_name = "A"),
        ReactionComponent(species_name = "B"),
    ],
)

abba = Mechanism(
    name = "ABBA",
    version = "1.0.0",
    species = [A, B, AB],
    phases = [gas],
    reactions = [forward, reverse_rxn, photo],
)

@testset "MechanismConfiguration" begin
    @testset "Serialization" begin
        d = JSON.parse(to_json_string(abba))

        @test d["name"] == "ABBA"
        @test d["version"] == "1.0.0"
        @test length(d["species"]) == 3
        @test length(d["phases"]) == 1
        @test length(d["reactions"]) == 3

        # Species: standard field + custom properties get the `__` prefix
        sA = only(filter(s -> s["name"] == "A", d["species"]))
        @test sA["molecular weight [kg mol-1]"] ≈ 0.029
        @test sA["__absolute tolerance"] ≈ 1e-4
        @test sA["__long name"] == "atom_A"
        @test sA["__do advect"] == true

        # Phase species are emitted as `{name: ...}` objects
        @test d["phases"][1]["species"] ==
              [Dict("name" => "A"), Dict("name" => "B"), Dict("name" => "AB")]

        # Reactions: types, spaced keys, default coefficient
        rf = only(filter(r -> r["name"] == "forward_AB_to_A_B", d["reactions"]))
        @test rf["type"] == "USER_DEFINED"
        @test rf["gas phase"] == "gas"
        @test rf["scaling factor"] ≈ 2.0e-3
        @test rf["reactants"][1]["species name"] == "AB"
        @test rf["reactants"][1]["coefficient"] ≈ 1.0
        @test only(filter(r -> r["name"] == "jNO2", d["reactions"]))["type"] == "PHOTOLYSIS"

        # YAML parses back to the same structure as JSON
        @test YAML.load(to_yaml_string(abba)) == d

        # to_string dispatches on format
        @test to_string(abba; format = :json) == to_json_string(abba)
        @test to_string(abba; format = :yaml) == to_yaml_string(abba)
        @test_throws ErrorException to_string(abba; format = :toml)
    end

    @testset "FirstOrderLoss products" begin
        # products are optional for first-order loss and omitted when empty
        fol_no_products = FirstOrderLoss(
            name = "loss",
            gas_phase = "gas",
            reactants = [ReactionComponent(species_name = "A")],
        )
        d_no = to_dict(fol_no_products)
        @test d_no["type"] == "FIRST_ORDER_LOSS"
        @test d_no["reactants"][1]["species name"] == "A"
        @test !haskey(d_no, "products")

        # products (with coefficients) are emitted when provided
        fol_with_products = FirstOrderLoss(
            name = "loss",
            gas_phase = "gas",
            reactants = [ReactionComponent(species_name = "A")],
            products = [
                ReactionComponent(species_name = "B"),
                ReactionComponent(species_name = "C", coefficient = 2.0),
            ],
        )
        d_with = to_dict(fol_with_products)
        @test length(d_with["products"]) == 2
        @test d_with["products"][1]["species name"] == "B"
        @test d_with["products"][1]["coefficient"] ≈ 1.0
        @test d_with["products"][2]["species name"] == "C"
        @test d_with["products"][2]["coefficient"] ≈ 2.0
    end

    # Recreates the single-cell ABBA box model from the CheMPAS-MUSICA dev test
    # (Fillmore, 2026-01-22): start from a mass mixing ratio, convert to mol m-3 with
    # the molecular weight and an ideal-gas air density, run A + B <-> AB to
    # equilibrium, and verify the equilibrium and mass conservation.
    @testset "ABBA box model reaches equilibrium" begin
        # Environmental conditions and physical constants.
        temperature = 298.0      # K
        pressure = 101325.0      # Pa
        molar_mass_air = 0.0289647  # kg mol-1 (dry air)

        # Rate constants from the mechanism (= the USER_DEFINED scaling factors):
        #   formation    A + B -> AB   kf = 1.0e-3
        #   dissociation AB -> A + B   kr = 2.0e-3
        kf = 1.0e-3
        kr = 2.0e-3
        q_AB0 = 0.6              # initial mass mixing ratio of AB [kg kg-1]

        micm = MICM(config_string = to_json_string(abba))
        state = create_state(micm)

        ordering = get_species_ordering(state)
        @test haskey(ordering, "A")
        @test haskey(ordering, "B")
        @test haskey(ordering, "AB")

        set_conditions!(state, temperatures = temperature, pressures = pressure)

        # ── Air density from the ideal gas law ──────────────────────────────
        # molar density [mol m-3] = P / (R T); mass density [kg m-3] = molar * M_air
        molar_density = pressure / (GAS_CONSTANT * temperature)   # mol m-3
        air_mass_density = molar_density * molar_mass_air         # kg m-3
        # Cross-check against the air density MICM computed internally.
        @test get_conditions(state)["air_density"][1] ≈ molar_density rtol = 1e-6

        # ── Read molecular weights and convert mixing ratio -> mol m-3 ───────
        M_AB = get_species_property(micm, "AB", "molecular weight [kg mol-1]", Float64)
        M_A = get_species_property(micm, "A", "molecular weight [kg mol-1]", Float64)
        M_B = get_species_property(micm, "B", "molecular weight [kg mol-1]", Float64)
        @test M_AB ≈ 0.058
        @test M_A ≈ 0.029

        # C [mol m-3] = q [kg kg-1] * air_mass_density [kg m-3] / M [kg mol-1]
        C_AB0 = q_AB0 * air_mass_density / M_AB
        @test C_AB0 > 0.0

        # Seed the box model with the converted concentration.
        set_concentrations!(state, Dict{String,Any}("A" => 0.0, "B" => 0.0, "AB" => C_AB0))

        # Drive the two A + B <-> AB reactions and turn the photolysis pathway off so
        # the equilibrium is governed purely by kf and kr. Match by reaction name so
        # the test is independent of the rate-parameter key prefix.
        rate_ordering = get_user_defined_rate_parameters_ordering(state)
        @test !isempty(rate_ordering)
        rates = Dict{String,Any}()
        for key in keys(rate_ordering)
            rates[key] =
                (occursin("forward_AB_to_A_B", key) || occursin("reverse_A_B_to_AB", key)) ?
                1.0 : 0.0
        end
        set_user_defined_rate_parameters!(state, rates)

        # Integrate to equilibrium (dissociation timescale 1/kr = 500 s; 90 min is
        # many e-folding times).
        for _ = 1:90
            result = solve!(micm, state, 60.0)
            @test result.state == Converged
        end

        concs = get_concentrations(state)
        A_eq = concs["A"][1]
        B_eq = concs["B"][1]
        AB_eq = concs["AB"][1]

        # AB is dissociated and A/B produced.
        @test AB_eq < C_AB0
        @test A_eq > 0.0

        # Stoichiometry: A and B produced one-for-one.
        @test A_eq ≈ B_eq rtol = 1e-6
        # Mole conservation: every AB lost yields one A.
        @test AB_eq + A_eq ≈ C_AB0 rtol = 1e-6

        # Chemical equilibrium: kf[A][B] == kr[AB]  =>  [A][B]/[AB] == kr/kf.
        @test (A_eq * B_eq) / AB_eq ≈ kr / kf rtol = 1e-2

        # Analytic equilibrium concentration: kf x^2 = kr (C_AB0 - x).
        x_eq = (-kr + sqrt(kr^2 + 4 * kf * kr * C_AB0)) / (2 * kf)
        @test A_eq ≈ x_eq rtol = 1e-2

        # Mass mixing ratio is conserved (total stays 0.6).
        q_total = (M_AB * AB_eq + M_A * A_eq + M_B * B_eq) / air_mass_density
        @test q_total ≈ q_AB0 rtol = 1e-6

        # Matches the documented surface equilibrium: qAB ~ 0.40, qA ~ 0.10.
        @test (M_AB * AB_eq) / air_mass_density ≈ 0.40 atol = 0.02
        @test (M_A * A_eq) / air_mass_density ≈ 0.10 atol = 0.02

        # Confirm we are truly at steady state: another step barely moves AB.
        solve!(micm, state, 60.0)
        @test get_concentrations(state)["AB"][1] ≈ AB_eq rtol = 1e-4
    end
end
