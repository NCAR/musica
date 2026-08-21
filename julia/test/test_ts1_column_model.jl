# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# A column (multi grid cell) model that drives the TS1/TSMLT chemical
# mechanism with TUV-x TS1 photolysis rates, run two ways: once with TUV-x's
# own delta-eddington radiative transfer, and once by feeding TUV-x a
# host-supplied radiation field instead. The two scenarios use the exact same
# radiation field -- the second run's field is read straight out of the
# first run's own output -- so their photolysis rates, and the concentrations
# the column model produces from them, are expected to match.

using Test
using Musica
using Musica.VTS1
using JSON

const _TS1_CONFIG_ROOT = joinpath(@__DIR__, "..", "..", "configs")
const _TS1_TUVX_CONFIG = joinpath(_TS1_CONFIG_ROOT, "tuvx", "ts1_tsmlt.json")
const _TS1_HOST_RADIATION_FIELD_CONFIG =
    joinpath(_TS1_CONFIG_ROOT, "tuvx", "ts1_tsmlt_host_radiation_field.json")
const _TS1_MICM_CONFIG = joinpath(_TS1_CONFIG_ROOT, "v1", "ts1", "ts1.json")
const _TS1_INITIAL_CONDITIONS =
    joinpath(_TS1_CONFIG_ROOT, "v1", "ts1", "initial_conditions.csv")

# ts1_tsmlt_host_radiation_field.json is ts1_tsmlt.json with its
# "radiative transfer" section replaced by a "from host" solver -- same
# grids, profiles, and 73 photolysis reactions, so a TUVX built from it can
# be driven from the same GridMap/ProfileMap/RadiatorMap setup.
function _ts1_maps()
    grids = GridMap()
    grids["height", "km"] = VTS1.height_grid()
    grids["wavelength", "nm"] = VTS1.wavelength_grid()

    profiles = ProfileMap()
    profiles["air", "molecule cm-3"] = VTS1.profile("air", grids["height", "km"])
    profiles["O3", "molecule cm-3"] = VTS1.profile("O3", grids["height", "km"])
    profiles["O2", "molecule cm-3"] = VTS1.profile("O2", grids["height", "km"])
    profiles["temperature", "K"] = VTS1.profile("temperature", grids["height", "km"])
    profiles["surface albedo", "none"] =
        VTS1.profile("surface albedo", grids["wavelength", "nm"])
    profiles["extraterrestrial flux", "photon cm-2 s-1"] =
        VTS1.profile("extraterrestrial flux", grids["wavelength", "nm"])

    radiators = RadiatorMap()
    radiators["aerosol"] =
        VTS1.radiator("aerosol", grids["height", "km"], grids["wavelength", "nm"])
    return grids, profiles, radiators
end

# initial_conditions.csv has one "parameter,value1[,value2]" row per line, no
# header. ENV.* rows are not used here: conditions instead come from TUV-x's
# own temperature/air profiles, so both models see the same atmosphere.
function _parse_ts1_conditions(path)
    concentrations = Dict{String,Float64}()
    user_defined = Dict{String,Float64}()
    surface_reactions = Dict{String,Tuple{Float64,Float64}}()
    for line in readlines(path)
        isempty(strip(line)) && continue
        parts = split(line, ',')
        name = parts[1]
        value1 = parse(Float64, parts[2])
        if startswith(name, "CONC.")
            concentrations[name[6:end]] = value1
        elseif startswith(name, "SURF.")
            surface_reactions[name] = (value1, parse(Float64, parts[3]))
        elseif startswith(name, "USER.") || startswith(name, "PHOTO.")
            user_defined[name] = value1
        end
    end
    return concentrations, user_defined, surface_reactions
end

_ts1_alias_pairs() = JSON.parsefile(_TS1_TUVX_CONFIG)["__CAM options"]["aliasing"]["pairs"]

# Maps TUV-x reaction rates onto the mechanism's "PHOTO.<label>" user-defined
# rate parameters via the config's alias table, restricted to `cell_range`.
function _photolysis_params(rate_constants, reaction_names, cell_range, pairs)
    params = Dict{String,Vector{Float64}}()
    for pair in pairs
        from = pair["from"]
        haskey(reaction_names, from) || continue
        scale = get(pair, "scale by", 1.0)
        params["PHOTO.$(pair["to"])"] =
            rate_constants[reaction_names[from]+1, cell_range] .* scale
    end
    return params
end

# Runs the TS1/TSMLT mechanism for `n_steps` of `time_step` seconds over
# `cell_range` and returns the final concentrations.
function _run_ts1_column(
    photolysis_params,
    cell_range,
    concentrations,
    user_defined,
    surface_reactions,
    profiles,
)
    n_cells = length(cell_range)
    micm = MICM(config_path = _TS1_MICM_CONFIG)
    state = create_state(micm; number_of_grid_cells = n_cells)

    rate_ordering = get_user_defined_rate_parameters_ordering(state)
    species_ordering = get_species_ordering(state)

    concentration_dict = Dict{String,Any}(
        name => fill(value, n_cells) for
        (name, value) in concentrations if haskey(species_ordering, name)
    )

    rate_param_dict = Dict{String,Any}()
    for (name, value) in user_defined
        haskey(rate_ordering, name) && (rate_param_dict[name] = fill(value, n_cells))
    end
    for (name, (radius, number_concentration)) in surface_reactions
        radius_key = "$name.effective radius [m]"
        number_key = "$name.particle number concentration [# m-3]"
        haskey(rate_ordering, radius_key) &&
            (rate_param_dict[radius_key] = fill(radius, n_cells))
        haskey(rate_ordering, number_key) &&
            (rate_param_dict[number_key] = fill(number_concentration, n_cells))
    end
    for (name, value) in photolysis_params
        haskey(rate_ordering, name) && (rate_param_dict[name] = value)
    end

    # Same atmosphere for both scenarios: TUV-x's own temperature and air
    # number density profiles, converted from molecule cm^-3 to mol m^-3.
    avogadro = 6.02214076e23
    temperatures = midpoint_values(profiles["temperature", "K"])[cell_range .- 1]
    air_densities =
        midpoint_values(profiles["air", "molecule cm-3"])[cell_range .- 1] .* 1.0e6 ./
        avogadro

    set_conditions!(state; temperatures = temperatures, air_densities = air_densities)
    set_concentrations!(state, concentration_dict)
    set_user_defined_rate_parameters!(state, rate_param_dict)

    time_step = 30.0
    n_steps = 20
    for _ = 1:n_steps
        elapsed = 0.0
        while elapsed < time_step
            result = solve!(micm, state, time_step - elapsed)
            elapsed += result.stats.final_time
        end
    end

    return get_concentrations(state)
end

@testset "TS1 column model" begin
    # Vertical edges 2:10 (1-based), skipping the surface edge, matching the
    # approach in python/musica/examples/ts1_box_model.py.
    cell_range = 2:10
    sza = deg2rad(20.0)
    earth_sun_distance = 1.0
    pairs = _ts1_alias_pairs()
    concentrations, user_defined, surface_reactions =
        _parse_ts1_conditions(_TS1_INITIAL_CONDITIONS)

    grids1, profiles1, radiators1 = _ts1_maps()
    tuvx1 = cd(dirname(_TS1_TUVX_CONFIG)) do
        TUVX(
            grid_map = grids1,
            profile_map = profiles1,
            radiator_map = radiators1,
            config_path = _TS1_TUVX_CONFIG,
        )
    end
    @test get_radiation_field_updater(tuvx1) === nothing  # delta eddington, not "from host"

    result1 = run!(tuvx1, sza, earth_sun_distance)
    reaction_names1 = photolysis_rate_names(tuvx1)
    baseline_concentrations = nothing

    @testset "TUV-x radiative transfer drives photolysis" begin
        photolysis_params = _photolysis_params(
            result1.photolysis_rate_constants,
            reaction_names1,
            cell_range,
            pairs,
        )
        @test length(photolysis_params) > 0

        final_concentrations = _run_ts1_column(
            photolysis_params,
            cell_range,
            concentrations,
            user_defined,
            surface_reactions,
            profiles1,
        )
        for (species, values) in final_concentrations
            @test all(isfinite, values)
            @test all(>=(0.0), values)
        end
        @test all(>(0.0), final_concentrations["O3"])  # photolysis is active at this SZA

        baseline_concentrations = final_concentrations
    end

    @testset "Host-supplied radiation field drives the same photolysis" begin
        grids2, profiles2, radiators2 = _ts1_maps()
        tuvx2 = cd(dirname(_TS1_TUVX_CONFIG)) do
            TUVX(
                grid_map = grids2,
                profile_map = profiles2,
                radiator_map = radiators2,
                config_path = _TS1_HOST_RADIATION_FIELD_CONFIG,
            )
        end
        updater = get_radiation_field_updater(tuvx2)
        @test updater isa RadiationFieldUpdater

        # run!'s actinic_flux/spectral_irradiance output is already the
        # normalized quantity update! expects -- confirmed empirically: it
        # reproduces the baseline photolysis rates exactly. Only the axis
        # order needs to change: run! returns (wavelength, interface,
        # component); update! wants (interface, wavelength).
        update!(
            updater,
            permutedims(result1.actinic_flux[:, :, 1], (2, 1)),
            permutedims(result1.actinic_flux[:, :, 2], (2, 1)),
            permutedims(result1.actinic_flux[:, :, 3], (2, 1));
            direct_irradiance = permutedims(result1.spectral_irradiance[:, :, 1], (2, 1)),
            upward_irradiance = permutedims(result1.spectral_irradiance[:, :, 2], (2, 1)),
            downward_irradiance = permutedims(result1.spectral_irradiance[:, :, 3], (2, 1)),
        )

        result2 = run!(tuvx2, sza, earth_sun_distance)
        reaction_names2 = photolysis_rate_names(tuvx2)

        @test result2.photolysis_rate_constants ≈ result1.photolysis_rate_constants rtol =
            1.0e-8

        photolysis_params = _photolysis_params(
            result2.photolysis_rate_constants,
            reaction_names2,
            cell_range,
            pairs,
        )
        final_concentrations = _run_ts1_column(
            photolysis_params,
            cell_range,
            concentrations,
            user_defined,
            surface_reactions,
            profiles2,
        )
        @test Set(keys(final_concentrations)) == Set(keys(baseline_concentrations))
        for (species, values) in final_concentrations
            @test all(isfinite, values)
            @test all(>=(0.0), values)
            @test values ≈ baseline_concentrations[species] rtol = 1.0e-8 atol = 1.0e-30
        end
    end
end
