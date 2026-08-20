# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    Musica.V54

Convenience grids, profiles, radiators, and a ready-to-use [`TUVX`](@ref)
calculator for TUV-x's v5.4 configuration.

# Example

```julia
using Musica
using Musica.V54

tuvx = cd(dirname(V54.config_file_path())) do
    V54.get_tuvx_calculator()
end
result = run!(tuvx, deg2rad(30.0), 1.0)
```

The configuration file's data-file paths are relative to its own directory
([`TUVX`](@ref) does no directory handling), so `cd` there first as shown
above.
"""
module V54

using ..Musica: Grid, GridMap, Profile, ProfileMap, Radiator, RadiatorMap, TUVX
using ..Musica: set_edges!, set_midpoints!, edges, midpoints, num_sections

export config_file_path,
    height_grid, wavelength_grid, profile, radiator, get_tuvx_calculator
export profile_data_files, radiator_data_files, profile_from_map, radiator_from_map

const _CONFIG_ROOT = normpath(joinpath(@__DIR__, "..", "..", "..", "configs", "tuvx"))

"""
    config_file_path() -> String

Path to the TUV-x v5.4 configuration file.
"""
config_file_path() = joinpath(_CONFIG_ROOT, "tuv_5_4.json")

"""
    height_grid() -> Grid

The v5.4 height grid: 120 sections from 0 to 120 km.
"""
function height_grid()
    heights = Grid(name = "height", units = "km", num_sections = 120)
    set_edges!(heights, collect(range(0.0, 120.0, length = 121)))
    set_midpoints!(heights, 0.5 .* (edges(heights)[1:(end-1)] .+ edges(heights)[2:end]))
    return heights
end

"""
    wavelength_grid() -> Grid

The v5.4 wavelength grid: 156 sections from 120 to 735 nm.
"""
function wavelength_grid()
    wavelength_edges = [
        120.0,
        121.4,
        121.9,
        122.3,
        123.1,
        123.8,
        124.6,
        125.4,
        126.2,
        127.0,
        128.6,
        129.4,
        130.3,
        132.0,
        135.0,
        137.0,
        145.0,
        155.0,
        165.0,
        170.0,
        175.4,
        177.0,
        178.6,
        180.2,
        181.8,
        183.5,
        185.2,
        186.9,
        188.7,
        190.5,
        192.3,
        194.2,
        196.1,
        198.0,
        200.0,
        202.0,
        204.1,
        206.2,
        208.333,
        210.526,
        212.766,
        215.054,
        217.391,
        219.78,
        222.222,
        224.719,
        227.273,
        229.885,
        232.558,
        235.294,
        238.095,
        240.964,
        243.902,
        246.914,
        250.0,
        253.165,
        256.41,
        259.74,
        263.158,
        266.667,
        270.27,
        273.973,
        277.778,
        281.69,
        285.714,
        289.855,
        294.118,
        298.5,
        302.5,
        303.5,
        304.5,
        305.5,
        306.5,
        307.5,
        308.5,
        309.5,
        310.5,
        311.5,
        312.5,
        313.5,
        314.5,
        317.5,
        322.5,
        327.5,
        332.5,
        337.5,
        342.5,
        347.5,
        352.5,
        357.5,
        362.5,
        367.5,
        372.5,
        377.5,
        382.5,
        387.5,
        392.5,
        397.5,
        402.5,
        407.5,
        412.5,
        417.5,
        422.5,
        427.5,
        432.5,
        437.5,
        442.5,
        447.5,
        452.5,
        457.5,
        462.5,
        467.5,
        472.5,
        477.5,
        482.5,
        487.5,
        492.5,
        497.5,
        502.5,
        507.5,
        512.5,
        517.5,
        522.5,
        527.5,
        532.5,
        537.5,
        542.5,
        547.5,
        552.5,
        557.5,
        562.5,
        567.5,
        572.5,
        577.5,
        582.5,
        587.5,
        592.5,
        597.5,
        602.5,
        607.5,
        612.5,
        617.5,
        622.5,
        627.5,
        632.5,
        637.5,
        642.5,
        647.1,
        655.0,
        665.0,
        675.0,
        685.0,
        695.0,
        705.0,
        715.0,
        725.0,
        735.0,
    ]
    wavelengths =
        Grid(name = "wavelength", units = "nm", num_sections = length(wavelength_edges) - 1)
    set_edges!(wavelengths, wavelength_edges)
    set_midpoints!(
        wavelengths,
        0.5 .* (edges(wavelengths)[1:(end-1)] .+ edges(wavelengths)[2:end]),
    )
    return wavelengths
end

const profile_data_files = Dict(
    "O2" => joinpath("profiles", "atmosphere", "o2.v54.dat"),
    "O3" => joinpath("profiles", "atmosphere", "o3.v54.dat"),
    "air" => joinpath("profiles", "atmosphere", "air.v54.dat"),
    "temperature" => joinpath("profiles", "atmosphere", "temperature.v54.dat"),
    "surface albedo" => joinpath("profiles", "solar", "surface_albedo.v54.dat"),
    "extraterrestrial flux" =>
        joinpath("profiles", "solar", "extraterrestrial_flux.v54.dat"),
)

"""
    profile(name::AbstractString, grid::Grid) -> Profile

The standard v5.4 profile by name, interpolated onto `grid` if `grid` does
not match the data file's native grid. `name` is one of `keys(profile_data_files)`.
"""
profile(name::AbstractString, grid::Grid) = profile_from_map(profile_data_files, name, grid)

# Mimics numpy.interp: piecewise-linear, clamped to the end values outside
# the range of `xp`. `xp` must be sorted ascending.
function _interp(
    x::AbstractVector{<:Real},
    xp::AbstractVector{<:Real},
    fp::AbstractVector{<:Real},
)
    return map(x) do xi
        if xi <= xp[1]
            fp[1]
        elseif xi >= xp[end]
            fp[end]
        else
            i = searchsortedlast(xp, xi)
            t = (xi - xp[i]) / (xp[i+1] - xp[i])
            fp[i] + t * (fp[i+1] - fp[i])
        end
    end
end

"""
    profile_from_map(file_map::AbstractDict, name::AbstractString, grid::Grid) -> Profile

Build a `Profile` from a standard-atmosphere `.dat` data file keyed by `name`
in `file_map`, interpolating onto `grid` if the file's native grid does not
match `grid`. Shared by [`Musica.V54`](@ref) and [`Musica.VTS1`](@ref).
"""
function profile_from_map(file_map::AbstractDict, name::AbstractString, grid::Grid)
    haskey(file_map, name) ||
        error("Profile '$name' not found in this TUV-x configuration.")
    lines = readlines(joinpath(_CONFIG_ROOT, "data", file_map[name]))

    units = "unknown"
    for line in lines
        if startswith(line, " # Profile:") && occursin('(', line) && occursin(')', line)
            s = findfirst('(', line) + 1
            e = findnext(')', line, s)
            units = line[s:(e-1)]
            break
        end
    end

    grid_type = nothing
    for line in lines
        if occursin("height (km), mid-point", line) || occursin("height (km), edge", line)
            grid_type = :height
            break
        elseif occursin("wavelength (nm), mid-point", line) ||
               occursin("wavelength (nm), edge", line)
            grid_type = :wavelength
            break
        end
    end
    grid_type === nothing && error("Could not determine grid type from file headers.")

    # midpoint rows hold [height, midpoint value, layer density]; edge rows
    # hold [height, edge value]. A row with only the exo layer density column
    # populated marks the end of the midpoint section.
    midpoint_rows = Vector{Float64}[]
    edge_rows = Vector{Float64}[]
    exo_layer_density = 0.0
    section = nothing
    for raw in lines
        line = strip(raw)
        if isempty(line) || startswith(line, "#")
            if occursin("mid-point", line) &&
               (occursin("height (km)", line) || occursin("wavelength (nm)", line))
                section = :midpoint
            elseif occursin("edge", line) &&
                   (occursin("height (km)", line) || occursin("wavelength (nm)", line))
                section = :edge
            end
            continue
        end
        parts = split(line, ',')
        all(p -> strip(p) == "---", parts) && continue
        if section == :midpoint
            height = strip(parts[1]) == "---" ? nothing : parse(Float64, parts[1])
            mid = strip(parts[2]) == "---" ? nothing : parse(Float64, parts[2])
            layer_density =
                length(parts) > 3 && strip(parts[4]) != "---" ? parse(Float64, parts[4]) :
                nothing
            if height !== nothing && mid !== nothing
                push!(
                    midpoint_rows,
                    [height, mid, layer_density === nothing ? 0.0 : layer_density],
                )
            else
                exo_layer_density =
                    length(parts) > 4 && strip(parts[5]) != "---" ?
                    parse(Float64, parts[5]) : 0.0
            end
        elseif section == :edge
            height = strip(parts[1]) == "---" ? nothing : parse(Float64, parts[1])
            edge_value = strip(parts[2]) == "---" ? nothing : parse(Float64, parts[2])
            if height !== nothing && edge_value !== nothing
                push!(edge_rows, [height, edge_value])
            end
        end
    end

    file_midpoints = [r[1] for r in midpoint_rows]
    file_midpoint_values = [r[2] for r in midpoint_rows]
    file_layer_densities = [r[3] for r in midpoint_rows]
    file_edges = [r[1] for r in edge_rows]
    file_edge_values = [r[2] for r in edge_rows]

    grid_midpoints = collect(midpoints(grid))
    grid_edges = collect(edges(grid))
    grids_match =
        length(grid_midpoints) == length(file_midpoints) &&
        isapprox(grid_midpoints, file_midpoints) &&
        length(grid_edges) == length(file_edges) &&
        isapprox(grid_edges, file_edges)

    if grids_match
        interpolated_midpoints = file_midpoint_values
        interpolated_edges = file_edge_values
        interpolated_layer_densities = file_layer_densities
    else
        interpolated_midpoints =
            _interp(grid_midpoints, file_midpoints, file_midpoint_values)
        interpolated_edges = _interp(grid_edges, file_edges, file_edge_values)
        interpolated_layer_densities =
            _interp(grid_midpoints, file_midpoints, file_layer_densities)
    end

    # The exo layer density is added back on top of the uppermost layer
    # density by the Profile constructor, so remove it here first.
    if exo_layer_density > 0.0
        interpolated_layer_densities[end] -= exo_layer_density
    end

    return Profile(
        name = name,
        units = units,
        grid = grid,
        edge_values = interpolated_edges,
        midpoint_values = interpolated_midpoints,
        layer_densities = interpolated_layer_densities,
        exo_layer_density = exo_layer_density,
    )
end

const radiator_data_files = Dict("aerosol" => joinpath("radiators", "aerosol.v54.dat"))

"""
    radiator(name::AbstractString, height_grid::Grid, wavelength_grid::Grid) -> Radiator

The standard v5.4 radiator by name. `name` is one of `keys(radiator_data_files)`.
"""
radiator(name::AbstractString, height_grid::Grid, wavelength_grid::Grid) =
    radiator_from_map(radiator_data_files, name, height_grid, wavelength_grid)

"""
    radiator_from_map(file_map::AbstractDict, name::AbstractString, height_grid::Grid, wavelength_grid::Grid) -> Radiator

Build a `Radiator` from a standard-atmosphere `.dat` data file keyed by
`name` in `file_map`. Each data row is assigned to the nearest height and
wavelength section midpoint. Shared by [`Musica.V54`](@ref) and
[`Musica.VTS1`](@ref).
"""
function radiator_from_map(
    file_map::AbstractDict,
    name::AbstractString,
    height_grid::Grid,
    wavelength_grid::Grid,
)
    haskey(file_map, name) ||
        error("Radiator '$name' not found in this TUV-x configuration.")

    n_h = num_sections(height_grid)
    n_w = num_sections(wavelength_grid)
    height_mid = collect(midpoints(height_grid))
    wavelength_mid = collect(midpoints(wavelength_grid))
    optical_depths = zeros(n_h, n_w)
    single_scattering_albedos = zeros(n_h, n_w)
    asymmetry_factors = zeros(n_h, n_w)

    for raw in eachline(joinpath(_CONFIG_ROOT, "data", file_map[name]))
        stripped = strip(raw)
        (isempty(stripped) || startswith(stripped, "#")) && continue
        parts = split(stripped)
        length(parts) < 5 && continue
        file_height = parse(Float64, parts[1])
        file_wavelength = parse(Float64, parts[2])
        h_idx = argmin(abs.(height_mid .- file_height))
        w_idx = argmin(abs.(wavelength_mid .- file_wavelength))
        optical_depths[h_idx, w_idx] = parse(Float64, parts[3])
        single_scattering_albedos[h_idx, w_idx] = parse(Float64, parts[4])
        asymmetry_factors[h_idx, w_idx] = parse(Float64, parts[5])
    end

    return Radiator(
        name = name,
        height_grid = height_grid,
        wavelength_grid = wavelength_grid,
        optical_depths = optical_depths,
        single_scattering_albedos = single_scattering_albedos,
        asymmetry_factors = asymmetry_factors,
    )
end

"""
    get_tuvx_calculator() -> TUVX

A `TUVX` instance configured for the v5.4 photolysis setup.

The configuration file's data-file paths are relative to its own directory,
so `cd` there first, e.g. `cd(() -> V54.get_tuvx_calculator(), dirname(V54.config_file_path()))`.
"""
function get_tuvx_calculator()
    grids = GridMap()
    grids["height", "km"] = height_grid()
    grids["wavelength", "nm"] = wavelength_grid()

    profiles = ProfileMap()
    profiles["air", "molecule cm-3"] = profile("air", grids["height", "km"])
    profiles["O3", "molecule cm-3"] = profile("O3", grids["height", "km"])
    profiles["O2", "molecule cm-3"] = profile("O2", grids["height", "km"])
    profiles["temperature", "K"] = profile("temperature", grids["height", "km"])
    profiles["surface albedo", "none"] =
        profile("surface albedo", grids["wavelength", "nm"])
    profiles["extraterrestrial flux", "photon cm-2 s-1"] =
        profile("extraterrestrial flux", grids["wavelength", "nm"])

    radiators = RadiatorMap()
    radiators["aerosol"] =
        radiator("aerosol", grids["height", "km"], grids["wavelength", "nm"])

    return TUVX(
        grid_map = grids,
        profile_map = profiles,
        radiator_map = radiators,
        config_path = config_file_path(),
    )
end

end # module V54
