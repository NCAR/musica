# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const RadiatorMapPtr = CxxWrap.CxxWrapCore.CxxPtr{CppRadiatorMap}

"""
    RadiatorMap

A collection of TUV-x [`Radiator`](@ref) objects.

Unlike [`GridMap`](@ref) and [`ProfileMap`](@ref), a radiator is identified by
its name alone (there is no units key). The map supports both named methods
and dictionary-style access.

# Example

```julia
map = RadiatorMap()
height_grid = Grid(name = "height", units = "km", num_sections = 5)
wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
map["aerosol"] = Radiator(name = "aerosol", height_grid = height_grid, wavelength_grid = wavelength_grid)
length(map)             # 1
radiator = map["aerosol"]
haskey(map, "aerosol")  # true
for radiator in map
    println(get_name(radiator))
end
```

Iteration and [`values`](@ref) return `Radiator` objects. [`keys`](@ref)
returns radiator names. Index access with an integer is 1-based.
"""
mutable struct RadiatorMap
    _ptr::RadiatorMapPtr

    function RadiatorMap()
        obj = new(cpp_create_radiator_map())
        finalizer(obj) do m
            cpp_delete_radiator_map(m._ptr)
        end
        return obj
    end
end

"""
    get_number_of_radiators(map::RadiatorMap) -> Int

Get the number of radiators in the map.
"""
get_number_of_radiators(map::RadiatorMap) =
    Int(cpp_radiator_map_number_of_radiators(map._ptr))

"""
    add_radiator!(map::RadiatorMap, radiator::Radiator) -> RadiatorMap

Add a radiator to the map.

The map takes over the memory of the TUV-x radiator. The `radiator` object
stays usable and reads through the map from this point. Get a new view from
[`optical_depths`](@ref), [`single_scattering_albedos`](@ref), or
[`asymmetry_factors`](@ref) after this call, because an older view points to
memory that is free.
"""
function add_radiator!(map::RadiatorMap, radiator::Radiator)
    cpp_radiator_map_add_radiator!(map._ptr, radiator._ptr)
    radiator._owner = map
    return map
end

"""
    get_radiator(map::RadiatorMap, name::AbstractString) -> Radiator
    get_radiator(map::RadiatorMap, index::Integer) -> Radiator

Get a radiator from the map by name, or by 1-based index.
"""
function get_radiator(map::RadiatorMap, name::AbstractString)
    ptr = cpp_radiator_map_get_radiator(map._ptr, String(name))
    return Radiator(ptr, map)
end

function get_radiator(map::RadiatorMap, index::Integer)
    1 <= index <= get_number_of_radiators(map) || throw(BoundsError(map, index))
    ptr = cpp_radiator_map_get_radiator_by_index(map._ptr, Int64(index - 1))
    return Radiator(ptr, map)
end

"""
    remove_radiator!(map::RadiatorMap, name::AbstractString) -> RadiatorMap
    remove_radiator!(map::RadiatorMap, index::Integer) -> RadiatorMap

Remove a radiator from the map by name, or by 1-based index.
"""
function remove_radiator!(map::RadiatorMap, name::AbstractString)
    cpp_radiator_map_remove_radiator!(map._ptr, String(name))
    return map
end

function remove_radiator!(map::RadiatorMap, index::Integer)
    1 <= index <= get_number_of_radiators(map) || throw(BoundsError(map, index))
    cpp_radiator_map_remove_radiator_by_index!(map._ptr, Int64(index - 1))
    return map
end

Base.length(map::RadiatorMap) = get_number_of_radiators(map)
Base.isempty(map::RadiatorMap) = length(map) == 0
Base.eltype(::Type{RadiatorMap}) = Radiator

Base.getindex(map::RadiatorMap, name::AbstractString) = get_radiator(map, name)
Base.getindex(map::RadiatorMap, index::Integer) = get_radiator(map, index)

function Base.setindex!(map::RadiatorMap, radiator::Radiator, name::AbstractString)
    get_name(radiator) == name ||
        error("Radiator name must match the key: \"$(get_name(radiator))\" != \"$name\".")
    add_radiator!(map, radiator)
    return radiator
end

"""
    haskey(map::RadiatorMap, name::AbstractString) -> Bool

Report whether the map holds a radiator with the given name.
"""
function Base.haskey(map::RadiatorMap, name::AbstractString)
    try
        get_radiator(map, name)
        return true
    catch
        return false
    end
end

Base.haskey(map::RadiatorMap, key) = false
Base.in(name::AbstractString, map::RadiatorMap) = haskey(map, name)

"""
    keys(map::RadiatorMap) -> Vector{String}

Get the name of every radiator in the map.
"""
Base.keys(map::RadiatorMap) = [get_name(radiator) for radiator in values(map)]

"""
    values(map::RadiatorMap) -> Vector{Radiator}

Get every radiator in the map.
"""
Base.values(map::RadiatorMap) = [get_radiator(map, i) for i = 1:length(map)]

function Base.iterate(map::RadiatorMap, index::Int = 1)
    index > length(map) && return nothing
    return (get_radiator(map, index), index + 1)
end

"""
    empty!(map::RadiatorMap) -> RadiatorMap

Remove every radiator from the map.
"""
function Base.empty!(map::RadiatorMap)
    while !isempty(map)
        remove_radiator!(map, 1)
    end
    return map
end

function Base.show(io::IO, map::RadiatorMap)
    print(io, "RadiatorMap(num_radiators=$(length(map)))")
end

export RadiatorMap
export add_radiator!, get_radiator, remove_radiator!, get_number_of_radiators
