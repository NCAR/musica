# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

# Core data types for building a version 1 mechanism configuration in code.
#
# These mirror the JavaScript (`javascript/mechanism_configuration`) and Python
# (`python/musica/mechanism_configuration`) interfaces: plain data objects with a
# `to_dict` method that produces the spaced-key schema the MICM parser expects.
#
# Arbitrary user properties are passed via `other_properties` (a `Dict{String,Any}`
# keyed by the human-readable name *without* the `__` prefix). On serialization each
# such key is emitted with a leading `__`, matching the convention used by the C++
# parser and the other language bindings.

"""
    _merge_other_properties!(d::Dict, other_properties::Dict)

Copy `other_properties` into `d`, prefixing each key with `__`.
"""
function _merge_other_properties!(d::Dict{String,Any}, other_properties::AbstractDict)
    for (key, value) in other_properties
        d["__"*String(key)] = value
    end
    return d
end

"""
    ReactionComponent(; species_name=nothing, name=nothing, coefficient=1.0,
                        other_properties=Dict())

A single reactant or product entry in a reaction. `name` is accepted as an alias for
`species_name` for parity with the Python interface and the schema, which allow either
spelling; provide exactly one.

# Fields
- `species_name::String`: Name of the species.
- `coefficient::Float64`: Stoichiometric coefficient (default `1.0`).
- `other_properties::Dict{String,Any}`: Arbitrary extra properties.
"""
struct ReactionComponent
    species_name::String
    coefficient::Float64
    other_properties::Dict{String,Any}
end

function ReactionComponent(;
    species_name::Union{AbstractString,Nothing} = nothing,
    name::Union{AbstractString,Nothing} = nothing,
    coefficient::Real = 1.0,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    resolved = species_name === nothing ? name : species_name
    resolved === nothing && throw(
        ArgumentError("ReactionComponent requires `species_name` (or its alias `name`)"),
    )
    return ReactionComponent(
        String(resolved),
        Float64(coefficient),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(c::ReactionComponent)
    d = Dict{String,Any}("species name" => c.species_name, "coefficient" => c.coefficient)
    _merge_other_properties!(d, c.other_properties)
    return d
end

"""
    Species(; name, molecular_weight=nothing, constant_concentration=nothing,
              constant_mixing_ratio=nothing, is_third_body=nothing,
              other_properties=Dict())

A chemical species. Optional fields that are left as `nothing` are omitted from the
serialized configuration.

# Fields
- `name::String`: Species name.
- `molecular_weight`: Molecular weight in kg mol⁻¹.
- `constant_concentration`: Fixed concentration in mol m⁻³.
- `constant_mixing_ratio`: Fixed mixing ratio in mol mol⁻¹.
- `is_third_body`: Whether the species acts as a third body (e.g. `M`).
- `other_properties::Dict{String,Any}`: Arbitrary extra properties.
"""
struct Species
    name::String
    molecular_weight::Union{Float64,Nothing}
    constant_concentration::Union{Float64,Nothing}
    constant_mixing_ratio::Union{Float64,Nothing}
    is_third_body::Union{Bool,Nothing}
    other_properties::Dict{String,Any}
end

function Species(;
    name::AbstractString,
    molecular_weight::Union{Real,Nothing} = nothing,
    constant_concentration::Union{Real,Nothing} = nothing,
    constant_mixing_ratio::Union{Real,Nothing} = nothing,
    is_third_body::Union{Bool,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Species(
        String(name),
        molecular_weight === nothing ? nothing : Float64(molecular_weight),
        constant_concentration === nothing ? nothing : Float64(constant_concentration),
        constant_mixing_ratio === nothing ? nothing : Float64(constant_mixing_ratio),
        is_third_body,
        Dict{String,Any}(other_properties),
    )
end

function to_dict(s::Species)
    d = Dict{String,Any}("name" => s.name)
    s.molecular_weight === nothing ||
        (d["molecular weight [kg mol-1]"] = s.molecular_weight)
    s.constant_concentration === nothing ||
        (d["constant concentration [mol m-3]"] = s.constant_concentration)
    s.constant_mixing_ratio === nothing ||
        (d["constant mixing ratio [mol mol-1]"] = s.constant_mixing_ratio)
    s.is_third_body === nothing || (d["is third body"] = s.is_third_body)
    _merge_other_properties!(d, s.other_properties)
    return d
end

"""
    PhaseSpecies(; name, diffusion_coefficient=nothing, other_properties=Dict())

A reference to a species within a phase, optionally carrying a diffusion coefficient.

# Fields
- `name::String`: Species name.
- `diffusion_coefficient`: Diffusion coefficient in m² s⁻¹.
- `other_properties::Dict{String,Any}`: Arbitrary extra properties.
"""
struct PhaseSpecies
    name::String
    diffusion_coefficient::Union{Float64,Nothing}
    other_properties::Dict{String,Any}
end

function PhaseSpecies(;
    name::AbstractString,
    diffusion_coefficient::Union{Real,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return PhaseSpecies(
        String(name),
        diffusion_coefficient === nothing ? nothing : Float64(diffusion_coefficient),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(s::PhaseSpecies)
    d = Dict{String,Any}("name" => s.name)
    s.diffusion_coefficient === nothing ||
        (d["diffusion coefficient [m2 s-1]"] = s.diffusion_coefficient)
    _merge_other_properties!(d, s.other_properties)
    return d
end

"""
    Phase(; name, species, other_properties=Dict())

A phase grouping a set of species.

# Fields
- `name::String`: Phase name.
- `species::Vector`: Members of the phase. Each entry may be a `PhaseSpecies`, a
  `Species` (only its name is used), or a plain species-name `String`.
- `other_properties::Dict{String,Any}`: Arbitrary extra properties.
"""
struct Phase
    name::String
    species::Vector{Any}
    other_properties::Dict{String,Any}
end

function Phase(;
    name::AbstractString,
    species::AbstractVector,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Phase(String(name), collect(Any, species), Dict{String,Any}(other_properties))
end

_phase_species_dict(s::PhaseSpecies) = to_dict(s)
_phase_species_dict(s::Species) = Dict{String,Any}("name" => s.name)
_phase_species_dict(s::AbstractString) = Dict{String,Any}("name" => String(s))

function to_dict(p::Phase)
    d = Dict{String,Any}(
        "name" => p.name,
        "species" => [_phase_species_dict(s) for s in p.species],
    )
    _merge_other_properties!(d, p.other_properties)
    return d
end
