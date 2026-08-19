# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const ProfileMapPtr = CxxWrap.CxxWrapCore.CxxPtr{CppProfileMap}

"""
    ProfileMap

A collection of TUV-x [`Profile`](@ref) objects.

A profile is identified by its name and its units. The map supports both named
methods and dictionary-style access.

# Example

```julia
map = ProfileMap()
grid = Grid(name = "height", units = "km", num_sections = 5)
map["temperature", "K"] = Profile(name = "temperature", units = "K", grid = grid)
length(map)                          # 1
profile = map["temperature", "K"]
haskey(map, ("temperature", "K"))    # true
for profile in map
    println(get_name(profile))
end
```

Iteration and [`values`](@ref) return `Profile` objects. [`keys`](@ref) returns
`(name, units)` tuples. Index access with an integer is 1-based.
"""
mutable struct ProfileMap
    _ptr::ProfileMapPtr

    function ProfileMap()
        obj = new(cpp_create_profile_map())
        finalizer(obj) do m
            cpp_delete_profile_map(m._ptr)
        end
        return obj
    end
end

"""
    get_number_of_profiles(map::ProfileMap) -> Int

Get the number of profiles in the map.
"""
get_number_of_profiles(map::ProfileMap) = Int(cpp_profile_map_number_of_profiles(map._ptr))

"""
    add_profile!(map::ProfileMap, profile::Profile) -> ProfileMap

Add a profile to the map.

The map takes over the memory of the TUV-x profile. The `profile` object stays
usable and reads through the map from this point. Get a new view from
[`edge_values`](@ref), [`midpoint_values`](@ref), or [`layer_densities`](@ref)
after this call, because an older view points to memory that is free.
"""
function add_profile!(map::ProfileMap, profile::Profile)
    cpp_profile_map_add_profile!(map._ptr, profile._ptr)
    profile._owner = map
    return map
end

"""
    get_profile(map::ProfileMap, name::AbstractString, units::AbstractString) -> Profile
    get_profile(map::ProfileMap, index::Integer) -> Profile

Get a profile from the map by name and units, or by 1-based index.
"""
function get_profile(map::ProfileMap, name::AbstractString, units::AbstractString)
    ptr = cpp_profile_map_get_profile(map._ptr, String(name), String(units))
    return Profile(ptr, map)
end

function get_profile(map::ProfileMap, index::Integer)
    1 <= index <= get_number_of_profiles(map) || throw(BoundsError(map, index))
    ptr = cpp_profile_map_get_profile_by_index(map._ptr, Int64(index - 1))
    return Profile(ptr, map)
end

"""
    remove_profile!(map::ProfileMap, name::AbstractString, units::AbstractString) -> ProfileMap
    remove_profile!(map::ProfileMap, index::Integer) -> ProfileMap

Remove a profile from the map by name and units, or by 1-based index.
"""
function remove_profile!(map::ProfileMap, name::AbstractString, units::AbstractString)
    cpp_profile_map_remove_profile!(map._ptr, String(name), String(units))
    return map
end

function remove_profile!(map::ProfileMap, index::Integer)
    1 <= index <= get_number_of_profiles(map) || throw(BoundsError(map, index))
    cpp_profile_map_remove_profile_by_index!(map._ptr, Int64(index - 1))
    return map
end

Base.length(map::ProfileMap) = get_number_of_profiles(map)
Base.isempty(map::ProfileMap) = length(map) == 0
Base.eltype(::Type{ProfileMap}) = Profile

Base.getindex(map::ProfileMap, name::AbstractString, units::AbstractString) =
    get_profile(map, name, units)
Base.getindex(map::ProfileMap, key::Tuple{AbstractString,AbstractString}) =
    get_profile(map, key[1], key[2])
Base.getindex(map::ProfileMap, index::Integer) = get_profile(map, index)

function Base.setindex!(
    map::ProfileMap,
    profile::Profile,
    name::AbstractString,
    units::AbstractString,
)
    get_name(profile) == name && get_units(profile) == units || error(
        "Profile name and units must match the key: " *
        "(\"$(get_name(profile))\", \"$(get_units(profile))\") != (\"$name\", \"$units\").",
    )
    add_profile!(map, profile)
    return profile
end

Base.setindex!(map::ProfileMap, profile::Profile, key::Tuple{AbstractString,AbstractString}) =
    setindex!(map, profile, key[1], key[2])

"""
    haskey(map::ProfileMap, key::Tuple{AbstractString,AbstractString}) -> Bool

Report whether the map holds a profile with the given name and units.
"""
function Base.haskey(map::ProfileMap, key::Tuple{AbstractString,AbstractString})
    try
        get_profile(map, key[1], key[2])
        return true
    catch
        return false
    end
end

Base.haskey(map::ProfileMap, key) = false
Base.in(key::Tuple{AbstractString,AbstractString}, map::ProfileMap) = haskey(map, key)

"""
    keys(map::ProfileMap) -> Vector{Tuple{String,String}}

Get the `(name, units)` key of every profile in the map.
"""
Base.keys(map::ProfileMap) = [(get_name(profile), get_units(profile)) for profile in values(map)]

"""
    values(map::ProfileMap) -> Vector{Profile}

Get every profile in the map.
"""
Base.values(map::ProfileMap) = [get_profile(map, i) for i = 1:length(map)]

function Base.iterate(map::ProfileMap, index::Int = 1)
    index > length(map) && return nothing
    return (get_profile(map, index), index + 1)
end

"""
    empty!(map::ProfileMap) -> ProfileMap

Remove every profile from the map.
"""
function Base.empty!(map::ProfileMap)
    while !isempty(map)
        remove_profile!(map, 1)
    end
    return map
end

function Base.show(io::IO, map::ProfileMap)
    print(io, "ProfileMap(num_profiles=$(length(map)))")
end

export ProfileMap
export add_profile!, get_profile, remove_profile!, get_number_of_profiles
