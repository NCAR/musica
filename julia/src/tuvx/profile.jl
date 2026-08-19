# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const ProfilePtr = CxxWrap.CxxWrapCore.CxxPtr{CppProfile}

"""
    ProfileView <: AbstractVector{Float64}

A zero-copy view into a value array of a [`Profile`](@ref) (edge values,
midpoint values, or layer densities).

The view shares memory with the TUV-x profile, so a write to the view changes
the profile itself. The view also holds a reference to its profile. The
reference stops the garbage collector from freeing the profile while the view
is alive.

Get a new view after you add the profile to a [`ProfileMap`](@ref). The map
takes over the memory of the profile, and an older view then points to memory
that is free.
"""
struct ProfileView <: AbstractVector{Float64}
    _data::Vector{Float64}
    _profile::Any  # prevents GC of the parent Profile while this view is alive
end

Base.size(view::ProfileView) = size(view._data)
Base.IndexStyle(::Type{ProfileView}) = IndexLinear()
Base.getindex(view::ProfileView, i::Int) = view._data[i]
Base.setindex!(view::ProfileView, value, i::Int) = (view._data[i] = value)

"""
    Profile

A physical quantity defined on a TUV-x [`Grid`](@ref), such as temperature or
a species concentration.

# Constructor

    Profile(; name, units, grid, edge_values=nothing, midpoint_values=nothing,
              layer_densities=nothing, calculate_layer_densities=false,
              exo_layer_density=0.0)

- `name::AbstractString`: The name of the profile
- `units::AbstractString`: The units of the profile values
- `grid::Grid`: The grid on which the profile is defined
- `edge_values::AbstractVector{<:Real}`: Values at grid edges, of length `num_sections(grid) + 1`
- `midpoint_values::AbstractVector{<:Real}`: Values at grid midpoints, of length `num_sections(grid)`
- `layer_densities::AbstractVector{<:Real}`: Layer densities, of length `num_sections(grid)`
- `calculate_layer_densities::Bool`: If `true`, calculate layer densities from midpoint values
- `exo_layer_density::Real`: The layer density above the top of the grid

Give at most one of `edge_values` or `midpoint_values`; the other is derived
from it by linear interpolation/extrapolation. Give at most one of
`layer_densities` or `calculate_layer_densities=true`.

# Example

```julia
grid = Grid(name = "height", units = "km", edges = [0.0, 2.0, 4.0])
profile = Profile(name = "temperature", units = "K", grid = grid,
                   midpoint_values = [270.0, 260.0])
```
"""
mutable struct Profile
    _ptr::ProfilePtr
    _owner::Any  # the ProfileMap that owns the TUV-x profile, or nothing

    function Profile(ptr::ProfilePtr, owner)
        obj = new(ptr, owner)
        finalizer(obj) do p
            cpp_delete_profile(p._ptr)
        end
        return obj
    end
end

function Profile(;
    name::AbstractString,
    units::AbstractString,
    grid::Grid,
    edge_values::Union{AbstractVector{<:Real},Nothing} = nothing,
    midpoint_values::Union{AbstractVector{<:Real},Nothing} = nothing,
    layer_densities::Union{AbstractVector{<:Real},Nothing} = nothing,
    calculate_layer_densities::Bool = false,
    exo_layer_density::Real = 0.0,
)
    if layer_densities !== nothing && calculate_layer_densities
        error("Cannot provide layer_densities and set calculate_layer_densities=true.")
    end
    exo_layer_density >= 0.0 || error("exo_layer_density must be non-negative.")

    profile = Profile(cpp_create_profile(String(name), String(units), grid._ptr), nothing)

    if edge_values === nothing && midpoint_values === nothing
        set_edge_values!(profile, zeros(Float64, num_sections(grid) + 1))
        set_midpoint_values!(profile, zeros(Float64, num_sections(grid)))
        set_layer_densities!(profile, zeros(Float64, num_sections(grid)))
    elseif edge_values !== nothing && midpoint_values !== nothing
        set_edge_values!(profile, edge_values)
        set_midpoint_values!(profile, midpoint_values)
    elseif edge_values !== nothing
        set_edge_values!(profile, edge_values)
        mids = 0.5 .* (edge_values[1:(end-1)] .+ edge_values[2:end])
        set_midpoint_values!(profile, mids)
    else
        derived_edges = zeros(Float64, length(midpoint_values) + 1)
        derived_edges[2:(end-1)] .=
            0.5 .* (midpoint_values[1:(end-1)] .+ midpoint_values[2:end])
        derived_edges[1] = midpoint_values[1] - (derived_edges[2] - midpoint_values[1])
        derived_edges[end] =
            midpoint_values[end] + (midpoint_values[end] - derived_edges[end-1])
        set_edge_values!(profile, derived_edges)
        set_midpoint_values!(profile, midpoint_values)
    end

    if layer_densities !== nothing
        set_layer_densities!(profile, layer_densities)
    elseif calculate_layer_densities
        calculate_layer_densities!(profile, grid)
    end

    exo_layer_density > 0.0 && set_exo_layer_density!(profile, exo_layer_density)

    return profile
end

"""
    get_name(profile::Profile) -> String

Get the name of the profile.
"""
get_name(profile::Profile) = String(cpp_profile_name(profile._ptr))

"""
    get_units(profile::Profile) -> String

Get the units of the profile values.
"""
get_units(profile::Profile) = String(cpp_profile_units(profile._ptr))

"""
    num_sections(profile::Profile) -> Int

Get the number of sections in the profile's grid.
"""
num_sections(profile::Profile) = Int(cpp_profile_num_sections(profile._ptr))

Base.length(profile::Profile) = num_sections(profile)

"""
    edge_values(profile::Profile) -> ProfileView

Get a zero-copy view of the profile values at grid edges. The view has length
`num_sections + 1`.

A write to the view changes the profile.
"""
function edge_values(profile::Profile)
    n_edges = num_sections(profile) + 1
    address = cpp_profile_edge_values_pointer(profile._ptr)
    data = unsafe_wrap(Array, Ptr{Float64}(address), n_edges; own = false)
    return ProfileView(data, profile)
end

"""
    midpoint_values(profile::Profile) -> ProfileView

Get a zero-copy view of the profile values at grid midpoints. The view has
length `num_sections`.

A write to the view changes the profile.
"""
function midpoint_values(profile::Profile)
    n_midpoints = num_sections(profile)
    address = cpp_profile_midpoint_values_pointer(profile._ptr)
    data = unsafe_wrap(Array, Ptr{Float64}(address), n_midpoints; own = false)
    return ProfileView(data, profile)
end

"""
    layer_densities(profile::Profile) -> ProfileView

Get a zero-copy view of the profile's layer densities. The view has length
`num_sections`.

A write to the view changes the profile.
"""
function layer_densities(profile::Profile)
    n_layers = num_sections(profile)
    address = cpp_profile_layer_densities_pointer(profile._ptr)
    data = unsafe_wrap(Array, Ptr{Float64}(address), n_layers; own = false)
    return ProfileView(data, profile)
end

"""
    set_edge_values!(profile::Profile, values::AbstractVector{<:Real}) -> Profile

Copy `values` into the profile edge values. The length must equal `num_sections + 1`.
"""
function set_edge_values!(profile::Profile, values::AbstractVector{<:Real})
    view = edge_values(profile)
    length(values) == length(view) ||
        error("edge_values must have length $(length(view)) (num_sections + 1).")
    view .= values
    return profile
end

"""
    set_midpoint_values!(profile::Profile, values::AbstractVector{<:Real}) -> Profile

Copy `values` into the profile midpoint values. The length must equal `num_sections`.
"""
function set_midpoint_values!(profile::Profile, values::AbstractVector{<:Real})
    view = midpoint_values(profile)
    length(values) == length(view) ||
        error("midpoint_values must have length $(length(view)) (num_sections).")
    view .= values
    return profile
end

"""
    set_layer_densities!(profile::Profile, values::AbstractVector{<:Real}) -> Profile

Copy `values` into the profile's layer densities. The length must equal `num_sections`.
"""
function set_layer_densities!(profile::Profile, values::AbstractVector{<:Real})
    view = layer_densities(profile)
    length(values) == length(view) ||
        error("layer_densities must have length $(length(view)) (num_sections).")
    view .= values
    return profile
end

"""
    exo_layer_density(profile::Profile) -> Float64

Get the layer density above the top of the profile's grid.
"""
exo_layer_density(profile::Profile) = cpp_profile_exo_layer_density(profile._ptr)

"""
    set_exo_layer_density!(profile::Profile, value::Real) -> Profile

Set the layer density above the top of the profile's grid.
"""
function set_exo_layer_density!(profile::Profile, value::Real)
    cpp_profile_set_exo_layer_density!(profile._ptr, Float64(value))
    return profile
end

"""
    calculate_exo_layer_density!(profile::Profile, scale_height::Real) -> Profile

Calculate the layer density above the top of the profile's grid from the given
scale height.
"""
function calculate_exo_layer_density!(profile::Profile, scale_height::Real)
    cpp_profile_calculate_exo_layer_density!(profile._ptr, Float64(scale_height))
    return profile
end

"""
    calculate_layer_densities!(profile::Profile, grid::Grid; conv=nothing) -> Profile

Calculate layer densities from midpoint values and grid spacing.

`conv` is a conversion factor applied to the result. It defaults to `1.0`,
except when the grid is named `"height"` with units `"km"` and the profile
units are `"molecule cm-3"`, where it defaults to `1.0e5`.
"""
function calculate_layer_densities!(
    profile::Profile,
    grid::Grid;
    conv::Union{Real,Nothing} = nothing,
)
    if conv === nothing
        conv =
            get_name(grid) == "height" &&
            get_units(grid) == "km" &&
            get_units(profile) == "molecule cm-3" ? 1e5 : 1.0
    end
    deltas = edges(grid)[2:end] .- edges(grid)[1:(end-1)]
    set_layer_densities!(profile, midpoint_values(profile) .* deltas .* conv)
    return profile
end

function Base.show(io::IO, profile::Profile)
    print(
        io,
        "Profile(name=\"$(get_name(profile))\", units=\"$(get_units(profile))\", ",
        "num_sections=$(num_sections(profile)))",
    )
end

export Profile, ProfileView
export get_name, get_units, num_sections
export edge_values, midpoint_values, layer_densities
export set_edge_values!, set_midpoint_values!, set_layer_densities!
export exo_layer_density, set_exo_layer_density!, calculate_exo_layer_density!
export calculate_layer_densities!
