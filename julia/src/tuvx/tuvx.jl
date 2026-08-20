# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const TUVXPtr = CxxWrap.CxxWrapCore.CxxPtr{CppTUVX}

"""
    TUVX

The TUV-x photolysis calculator.

# Constructor

    TUVX(; grid_map, profile_map, radiator_map, config_path=nothing, config_string=nothing)

Provide exactly one of `config_path` or `config_string`.

- `grid_map::GridMap`: Grid definitions (height, wavelength) for the calculation
- `profile_map::ProfileMap`: Atmospheric profiles (temperature, species concentrations, surface albedo, ET flux)
- `radiator_map::RadiatorMap`: Optically active species
- `config_path::AbstractString`: Path to a JSON/YAML configuration file. Paths inside the
  configuration are resolved relative to this file's directory.
- `config_string::AbstractString`: A JSON/YAML configuration as a string

# Example

```julia
grids = GridMap()
profiles = ProfileMap()
radiators = RadiatorMap()
# ... populate grids, profiles, radiators to match the configuration ...
tuvx = TUVX(grid_map = grids, profile_map = profiles, radiator_map = radiators,
            config_path = "path/to/config.json")
result = run!(tuvx, deg2rad(30.0), 1.0)
result.photolysis_rate_constants  # (num_reactions, num_vertical_edges)
```
"""
mutable struct TUVX
    _ptr::TUVXPtr

    function TUVX(ptr::TUVXPtr)
        obj = new(ptr)
        finalizer(obj) do t
            cpp_delete_tuvx(t._ptr)
        end
        return obj
    end
end

function TUVX(;
    grid_map::GridMap,
    profile_map::ProfileMap,
    radiator_map::RadiatorMap,
    config_path::Union{AbstractString,Nothing} = nothing,
    config_string::Union{AbstractString,Nothing} = nothing,
)
    if (config_path === nothing) == (config_string === nothing)
        error("Provide exactly one of `config_path` or `config_string`")
    end

    tuvx = TUVX(cpp_create_tuvx())

    if config_path !== nothing
        # Data files referenced inside the configuration are resolved relative
        # to the configuration file's own directory, not the process's current
        # directory, so switch there for the call (mirrors the Python
        # interface; the C++/Fortran layer does no directory handling itself).
        dir = dirname(config_path)
        filename = basename(config_path)
        if isempty(dir)
            cpp_tuvx_create_from_file!(tuvx._ptr, filename, grid_map._ptr, profile_map._ptr, radiator_map._ptr)
        else
            cd(dir) do
                cpp_tuvx_create_from_file!(tuvx._ptr, filename, grid_map._ptr, profile_map._ptr, radiator_map._ptr)
            end
        end
    else
        cpp_tuvx_create_from_string!(
            tuvx._ptr,
            String(config_string),
            grid_map._ptr,
            profile_map._ptr,
            radiator_map._ptr,
        )
    end

    return tuvx
end

"""
    get_grid_map(tuvx::TUVX) -> GridMap

Get the grid map used by this TUV-x instance.
"""
get_grid_map(tuvx::TUVX) = GridMap(cpp_tuvx_get_grid_map(tuvx._ptr))

"""
    get_profile_map(tuvx::TUVX) -> ProfileMap

Get the profile map used by this TUV-x instance.
"""
get_profile_map(tuvx::TUVX) = ProfileMap(cpp_tuvx_get_profile_map(tuvx._ptr))

"""
    get_radiator_map(tuvx::TUVX) -> RadiatorMap

Get the radiator map used by this TUV-x instance.
"""
get_radiator_map(tuvx::TUVX) = RadiatorMap(cpp_tuvx_get_radiator_map(tuvx._ptr))

"""
    photolysis_rate_constant_count(tuvx::TUVX) -> Int

Get the number of photolysis reactions.
"""
photolysis_rate_constant_count(tuvx::TUVX) = Int(cpp_tuvx_photolysis_rate_constant_count(tuvx._ptr))

"""
    heating_rate_count(tuvx::TUVX) -> Int

Get the number of heating rate types.
"""
heating_rate_count(tuvx::TUVX) = Int(cpp_tuvx_heating_rate_count(tuvx._ptr))

"""
    dose_rate_count(tuvx::TUVX) -> Int

Get the number of dose rate types.
"""
dose_rate_count(tuvx::TUVX) = Int(cpp_tuvx_dose_rate_count(tuvx._ptr))

"""
    num_height_midpoints(tuvx::TUVX) -> Int

Get the number of vertical layers (height grid midpoints).
"""
num_height_midpoints(tuvx::TUVX) = Int(cpp_tuvx_num_height_midpoints(tuvx._ptr))

"""
    num_wavelength_midpoints(tuvx::TUVX) -> Int

Get the number of wavelength grid midpoints.
"""
num_wavelength_midpoints(tuvx::TUVX) = Int(cpp_tuvx_num_wavelength_midpoints(tuvx._ptr))

function _ordering_dict(names, indices)
    return Dict{String,Int}(String(n) => Int(i) for (n, i) in zip(names, indices))
end

"""
    photolysis_rate_names(tuvx::TUVX) -> Dict{String, Int}

Get the mapping of photolysis reaction names to their 0-based index in the
output arrays from [`run!`](@ref).
"""
photolysis_rate_names(tuvx::TUVX) =
    _ordering_dict(cpp_tuvx_photolysis_rate_names(tuvx._ptr), cpp_tuvx_photolysis_rate_indices(tuvx._ptr))

"""
    heating_rate_names(tuvx::TUVX) -> Dict{String, Int}

Get the mapping of heating rate names to their 0-based index in the output
arrays from [`run!`](@ref).
"""
heating_rate_names(tuvx::TUVX) =
    _ordering_dict(cpp_tuvx_heating_rate_names(tuvx._ptr), cpp_tuvx_heating_rate_indices(tuvx._ptr))

"""
    dose_rate_names(tuvx::TUVX) -> Dict{String, Int}

Get the mapping of dose rate names to their 0-based index in the output
arrays from [`run!`](@ref).
"""
dose_rate_names(tuvx::TUVX) =
    _ordering_dict(cpp_tuvx_dose_rate_names(tuvx._ptr), cpp_tuvx_dose_rate_indices(tuvx._ptr))

# TUV-x writes its output row-major (the trailing dimension is contiguous).
# Reshaping into the reversed dims first reinterprets the same flat buffer
# with no copy, then a single permutedims puts the axes in the documented
# (leading-dimension-first) order. Unlike the Grid/Profile/Radiator views,
# `run!` output is already a fresh copy each call, so there is no benefit to
# keeping the reversed, zero-copy axis order the way those views do.
function _unflatten_row_major(flat::Vector{Float64}, dims::NTuple{N,Int}) where {N}
    reversed = ntuple(i -> dims[N + 1 - i], N)
    return permutedims(reshape(flat, reversed), ntuple(i -> N + 1 - i, N))
end

"""
    run!(tuvx::TUVX, solar_zenith_angle::Real, earth_sun_distance::Real) -> NamedTuple

Run the TUV-x photolysis calculator.

- `solar_zenith_angle`: Solar zenith angle in radians
- `earth_sun_distance`: Earth-Sun distance in astronomical units (AU)

Returns a `NamedTuple` with:

- `photolysis_rate_constants`: `(num_reactions, num_vertical_edges)` [s⁻¹]
- `heating_rates`: `(num_heating_rates, num_vertical_edges)` [K s⁻¹]
- `dose_rates`: `(num_dose_rates, num_vertical_edges)` [W m⁻²]
- `actinic_flux`: `(num_wavelengths, num_vertical_edges, 3)` [photons cm⁻² s⁻¹ nm⁻¹]
- `spectral_irradiance`: `(num_wavelengths, num_vertical_edges, 3)` [W m⁻² nm⁻¹]

The trailing dimension of `actinic_flux` and `spectral_irradiance` indexes the
direct, upwelling, and downwelling components, in that order. Use
[`photolysis_rate_names`](@ref), [`heating_rate_names`](@ref), and
[`dose_rate_names`](@ref) to look up a row by name.
"""
function run!(tuvx::TUVX, solar_zenith_angle::Real, earth_sun_distance::Real)
    n_reactions = photolysis_rate_constant_count(tuvx)
    n_heating = heating_rate_count(tuvx)
    n_dose = dose_rate_count(tuvx)
    n_wavelengths = num_wavelength_midpoints(tuvx)
    n_edges = num_height_midpoints(tuvx) + 1

    photolysis_buf = zeros(Float64, n_reactions * n_edges)
    heating_buf = zeros(Float64, n_heating * n_edges)
    dose_buf = zeros(Float64, n_dose * n_edges)
    actinic_buf = zeros(Float64, n_wavelengths * n_edges * 3)
    spectral_buf = zeros(Float64, n_wavelengths * n_edges * 3)

    cpp_tuvx_run!(
        tuvx._ptr,
        Float64(solar_zenith_angle),
        Float64(earth_sun_distance),
        photolysis_buf,
        heating_buf,
        dose_buf,
        actinic_buf,
        spectral_buf,
    )

    return (
        photolysis_rate_constants = _unflatten_row_major(photolysis_buf, (n_reactions, n_edges)),
        heating_rates = _unflatten_row_major(heating_buf, (n_heating, n_edges)),
        dose_rates = _unflatten_row_major(dose_buf, (n_dose, n_edges)),
        actinic_flux = _unflatten_row_major(actinic_buf, (n_wavelengths, n_edges, 3)),
        spectral_irradiance = _unflatten_row_major(spectral_buf, (n_wavelengths, n_edges, 3)),
    )
end

function _rate_row(names::Dict{String,Int}, name::AbstractString, values::AbstractMatrix{<:Real}, label::AbstractString)
    haskey(names, name) ||
        error("$label '$name' not found. Available: $(collect(keys(names)))")
    return values[names[name] + 1, :]
end

"""
    get_photolysis_rate_constant(tuvx::TUVX, reaction_name::AbstractString, photolysis_rate_constants::AbstractMatrix{<:Real}) -> Vector{Float64}

Extract the photolysis rate constants for one reaction across all vertical
edges from the `photolysis_rate_constants` output of [`run!`](@ref).
"""
get_photolysis_rate_constant(tuvx::TUVX, reaction_name::AbstractString, photolysis_rate_constants::AbstractMatrix{<:Real}) =
    _rate_row(photolysis_rate_names(tuvx), reaction_name, photolysis_rate_constants, "Reaction")

"""
    get_heating_rate(tuvx::TUVX, rate_name::AbstractString, heating_rates::AbstractMatrix{<:Real}) -> Vector{Float64}

Extract one heating rate across all vertical edges from the `heating_rates`
output of [`run!`](@ref).
"""
get_heating_rate(tuvx::TUVX, rate_name::AbstractString, heating_rates::AbstractMatrix{<:Real}) =
    _rate_row(heating_rate_names(tuvx), rate_name, heating_rates, "Heating rate")

"""
    get_dose_rate(tuvx::TUVX, rate_name::AbstractString, dose_rates::AbstractMatrix{<:Real}) -> Vector{Float64}

Extract one dose rate across all vertical edges from the `dose_rates` output
of [`run!`](@ref).
"""
get_dose_rate(tuvx::TUVX, rate_name::AbstractString, dose_rates::AbstractMatrix{<:Real}) =
    _rate_row(dose_rate_names(tuvx), rate_name, dose_rates, "Dose rate")

export TUVX
export get_grid_map, get_profile_map, get_radiator_map
export photolysis_rate_constant_count, heating_rate_count, dose_rate_count
export num_height_midpoints, num_wavelength_midpoints
export photolysis_rate_names, heating_rate_names, dose_rate_names
export run!
export get_photolysis_rate_constant, get_heating_rate, get_dose_rate
