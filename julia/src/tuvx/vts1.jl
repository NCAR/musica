# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    Musica.VTS1

Convenience grids, profiles, radiators, and a ready-to-use [`TUVX`](@ref)
calculator for TUV-x's TS1/TSMLT configuration. Shares its height grid and
data-file parsing with [`Musica.V54`](@ref).

# Example

```julia
using Musica
using Musica.VTS1

tuvx = cd(dirname(VTS1.config_file_path())) do
    VTS1.get_tuvx_calculator()
end
result = run!(tuvx, deg2rad(30.0), 1.0)
```

The configuration file's data-file paths are relative to its own directory
([`TUVX`](@ref) does no directory handling), so `cd` there first as shown
above.
"""
module VTS1

using ..Musica: Grid, GridMap, Profile, ProfileMap, Radiator, RadiatorMap, TUVX
using ..Musica: set_edges!, set_midpoints!, edges
using ..V54: height_grid, profile_from_map, radiator_from_map

export config_file_path, height_grid, wavelength_grid, profile, radiator, get_tuvx_calculator
export profile_data_files, radiator_data_files

const _CONFIG_ROOT = normpath(joinpath(@__DIR__, "..", "..", "..", "configs", "tuvx"))

"""
    config_file_path() -> String

Path to the TUV-x TS1/TSMLT configuration file.
"""
config_file_path() = joinpath(_CONFIG_ROOT, "ts1_tsmlt.json")

"""
    wavelength_grid() -> Grid

The TS1/TSMLT wavelength grid: 102 sections from 120 to 750 nm.
"""
function wavelength_grid()
    wavelength_edges = [
        120.0, 121.4, 121.9, 123.5, 124.3, 125.5, 126.3, 127.1,
        130.1, 131.1, 135.0, 140.0, 145.0, 150.0, 155.0, 160.0,
        165.0, 168.0, 171.0, 173.0, 174.4, 175.4, 177.0, 178.6,
        180.2, 181.8, 183.5, 185.2, 186.9, 188.7, 190.5, 192.3,
        194.2, 196.1, 198.0, 200.0, 202.0, 204.1, 206.2, 208.0,
        211.0, 214.0, 217.0, 220.0, 223.0, 226.0, 229.0, 232.0,
        235.0, 238.0, 241.0, 244.0, 247.0, 250.0, 253.0, 256.0,
        259.0, 263.0, 267.0, 271.0, 275.0, 279.0, 283.0, 287.0,
        291.0, 295.0, 298.5, 302.5, 305.5, 308.5, 311.5, 314.5,
        317.5, 322.5, 327.5, 332.5, 337.5, 342.5, 347.5, 350.0,
        355.0, 360.0, 365.0, 370.0, 375.0, 380.0, 385.0, 390.0,
        395.0, 400.0, 405.0, 410.0, 415.0, 420.0, 430.0, 440.0,
        450.0, 500.0, 550.0, 600.0, 650.0, 700.0, 750.0,
    ]
    wavelengths = Grid(name = "wavelength", units = "nm", num_sections = length(wavelength_edges) - 1)
    set_edges!(wavelengths, wavelength_edges)
    set_midpoints!(wavelengths, 0.5 .* (edges(wavelengths)[1:(end - 1)] .+ edges(wavelengths)[2:end]))
    return wavelengths
end

const profile_data_files = Dict(
    "O2" => joinpath("profiles", "atmosphere", "o2.v54.dat"),
    "O3" => joinpath("profiles", "atmosphere", "o3.v54.dat"),
    "air" => joinpath("profiles", "atmosphere", "air.v54.dat"),
    "temperature" => joinpath("profiles", "atmosphere", "temperature.v54.dat"),
    "surface albedo" => joinpath("profiles", "solar", "surface_albedo.ts1.dat"),
    "extraterrestrial flux" => joinpath("profiles", "solar", "extraterrestrial_flux.ts1.dat"),
)

"""
    profile(name::AbstractString, grid::Grid) -> Profile

The standard TS1/TSMLT profile by name, interpolated onto `grid` if `grid`
does not match the data file's native grid. `name` is one of
`keys(profile_data_files)`.
"""
profile(name::AbstractString, grid::Grid) = profile_from_map(profile_data_files, name, grid)

const radiator_data_files = Dict("aerosol" => joinpath("radiators", "aerosol.ts1.dat"))

"""
    radiator(name::AbstractString, height_grid::Grid, wavelength_grid::Grid) -> Radiator

The standard TS1/TSMLT radiator by name. `name` is one of
`keys(radiator_data_files)`.
"""
radiator(name::AbstractString, height_grid::Grid, wavelength_grid::Grid) =
    radiator_from_map(radiator_data_files, name, height_grid, wavelength_grid)

"""
    get_tuvx_calculator() -> TUVX

A `TUVX` instance configured for the TS1/TSMLT photolysis setup.

The configuration file's data-file paths are relative to its own directory,
so `cd` there first, e.g. `cd(() -> VTS1.get_tuvx_calculator(), dirname(VTS1.config_file_path()))`.
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
    profiles["surface albedo", "none"] = profile("surface albedo", grids["wavelength", "nm"])
    profiles["extraterrestrial flux", "photon cm-2 s-1"] =
        profile("extraterrestrial flux", grids["wavelength", "nm"])

    radiators = RadiatorMap()
    radiators["aerosol"] = radiator("aerosol", grids["height", "km"], grids["wavelength", "nm"])

    return TUVX(
        grid_map = grids,
        profile_map = profiles,
        radiator_map = radiators,
        config_path = config_file_path(),
    )
end

end # module VTS1
