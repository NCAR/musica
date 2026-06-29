# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    Mechanism(; name, version="1.0.0", species, phases, reactions)

A complete version 1 mechanism configuration assembled in code.

# Fields
- `name::String`: Mechanism name.
- `version::String`: Configuration schema version (default `"1.0.0"`).
- `species::Vector`: A vector of [`Species`](@ref).
- `phases::Vector`: A vector of [`Phase`](@ref).
- `reactions::Vector`: A vector of reaction objects (e.g. [`Arrhenius`](@ref),
  [`UserDefined`](@ref), [`Photolysis`](@ref), …).

Serialize with [`to_json_string`](@ref) or [`to_yaml_string`](@ref) and pass the
result to `MICM(config_string = ...)`.
"""
struct Mechanism
    name::String
    version::String
    species::Vector{Any}
    phases::Vector{Any}
    reactions::Vector{Any}
end

function Mechanism(;
    name::AbstractString,
    version::AbstractString = "1.0.0",
    species::AbstractVector = [],
    phases::AbstractVector = [],
    reactions::AbstractVector = [],
)
    return Mechanism(
        String(name),
        String(version),
        collect(Any, species),
        collect(Any, phases),
        collect(Any, reactions),
    )
end

"""
    to_dict(m::Mechanism) -> Dict{String,Any}

Build the nested dictionary representation of the mechanism in the version 1 schema.
"""
function to_dict(m::Mechanism)
    return Dict{String,Any}(
        "name" => m.name,
        "version" => m.version,
        "species" => [to_dict(s) for s in m.species],
        "phases" => [to_dict(p) for p in m.phases],
        "reactions" => [to_dict(r) for r in m.reactions],
    )
end

"""
    to_json_string(m::Mechanism) -> String

Serialize the mechanism to a JSON string suitable for `MICM(config_string = ...)`.
"""
to_json_string(m::Mechanism) = JSON.json(to_dict(m))

"""
    to_yaml_string(m::Mechanism) -> String

Serialize the mechanism to a YAML string suitable for `MICM(config_string = ...)`.
"""
to_yaml_string(m::Mechanism) = sprint(YAML.write, to_dict(m))

"""
    to_string(m::Mechanism; format=:json) -> String

Serialize the mechanism to a string. `format` may be `:json` (default) or `:yaml`.
"""
function to_string(m::Mechanism; format::Symbol = :json)
    if format === :json
        return to_json_string(m)
    elseif format === :yaml
        return to_yaml_string(m)
    else
        error("Unknown format: $format (expected :json or :yaml)")
    end
end
