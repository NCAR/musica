# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const GridMapPtr = CxxWrap.CxxWrapCore.CxxPtr{CppGridMap}

"""
    GridMap

A collection of TUV-x [`Grid`](@ref) objects.

A grid is identified by its name and its units. The map supports both named
methods and dictionary-style access.

# Example

```julia
map = GridMap()
map["height", "km"] = Grid(name = "height", units = "km", num_sections = 5)
length(map)                      # 1
grid = map["height", "km"]
haskey(map, ("height", "km"))    # true
for grid in map
    println(get_name(grid))
end
```

Iteration and [`values`](@ref) return `Grid` objects. [`keys`](@ref) returns
`(name, units)` tuples. Index access with an integer is 1-based.
"""
mutable struct GridMap
    _ptr::GridMapPtr

    function GridMap()
        obj = new(cpp_create_grid_map())
        finalizer(obj) do m
            cpp_delete_grid_map(m._ptr)
        end
        return obj
    end

    # Wraps a map pointer already returned by the C++ layer (e.g. from
    # TUVX). The underlying GridMap tracks whether it owns the live TUV-x
    # data, so deleting this wrapper is always safe.
    function GridMap(ptr::GridMapPtr)
        obj = new(ptr)
        finalizer(obj) do m
            cpp_delete_grid_map(m._ptr)
        end
        return obj
    end
end

"""
    get_number_of_grids(map::GridMap) -> Int

Get the number of grids in the map.
"""
get_number_of_grids(map::GridMap) = Int(cpp_grid_map_number_of_grids(map._ptr))

"""
    add_grid!(map::GridMap, grid::Grid) -> GridMap

Add a grid to the map.

The map takes over the memory of the TUV-x grid. The `grid` object stays usable
and reads through the map from this point. Get a new view from
[`edges`](@ref) or [`midpoints`](@ref) after this call, because an older view
points to memory that is free.
"""
function add_grid!(map::GridMap, grid::Grid)
    cpp_grid_map_add_grid!(map._ptr, grid._ptr)
    grid._owner = map
    return map
end

"""
    get_grid(map::GridMap, name::AbstractString, units::AbstractString) -> Grid
    get_grid(map::GridMap, index::Integer) -> Grid

Get a grid from the map by name and units, or by 1-based index.
"""
function get_grid(map::GridMap, name::AbstractString, units::AbstractString)
    ptr = cpp_grid_map_get_grid(map._ptr, String(name), String(units))
    return Grid(ptr, map)
end

function get_grid(map::GridMap, index::Integer)
    1 <= index <= get_number_of_grids(map) || throw(BoundsError(map, index))
    ptr = cpp_grid_map_get_grid_by_index(map._ptr, Int64(index - 1))
    return Grid(ptr, map)
end

"""
    remove_grid!(map::GridMap, name::AbstractString, units::AbstractString) -> GridMap
    remove_grid!(map::GridMap, index::Integer) -> GridMap

Remove a grid from the map by name and units, or by 1-based index.
"""
function remove_grid!(map::GridMap, name::AbstractString, units::AbstractString)
    cpp_grid_map_remove_grid!(map._ptr, String(name), String(units))
    return map
end

function remove_grid!(map::GridMap, index::Integer)
    1 <= index <= get_number_of_grids(map) || throw(BoundsError(map, index))
    cpp_grid_map_remove_grid_by_index!(map._ptr, Int64(index - 1))
    return map
end

Base.length(map::GridMap) = get_number_of_grids(map)
Base.isempty(map::GridMap) = length(map) == 0
Base.eltype(::Type{GridMap}) = Grid

Base.getindex(map::GridMap, name::AbstractString, units::AbstractString) =
    get_grid(map, name, units)
Base.getindex(map::GridMap, key::Tuple{AbstractString,AbstractString}) =
    get_grid(map, key[1], key[2])
Base.getindex(map::GridMap, index::Integer) = get_grid(map, index)

function Base.setindex!(
    map::GridMap,
    grid::Grid,
    name::AbstractString,
    units::AbstractString,
)
    get_name(grid) == name && get_units(grid) == units || error(
        "Grid name and units must match the key: " *
        "(\"$(get_name(grid))\", \"$(get_units(grid))\") != (\"$name\", \"$units\").",
    )
    add_grid!(map, grid)
    return grid
end

Base.setindex!(map::GridMap, grid::Grid, key::Tuple{AbstractString,AbstractString}) =
    setindex!(map, grid, key[1], key[2])

"""
    haskey(map::GridMap, key::Tuple{AbstractString,AbstractString}) -> Bool

Report whether the map holds a grid with the given name and units.
"""
function Base.haskey(map::GridMap, key::Tuple{AbstractString,AbstractString})
    try
        get_grid(map, key[1], key[2])
        return true
    catch
        return false
    end
end

Base.haskey(map::GridMap, key) = false
Base.in(key::Tuple{AbstractString,AbstractString}, map::GridMap) = haskey(map, key)

"""
    keys(map::GridMap) -> Vector{Tuple{String,String}}

Get the `(name, units)` key of every grid in the map.
"""
Base.keys(map::GridMap) = [(get_name(grid), get_units(grid)) for grid in values(map)]

"""
    values(map::GridMap) -> Vector{Grid}

Get every grid in the map.
"""
Base.values(map::GridMap) = [get_grid(map, i) for i = 1:length(map)]

function Base.iterate(map::GridMap, index::Int = 1)
    index > length(map) && return nothing
    return (get_grid(map, index), index + 1)
end

"""
    empty!(map::GridMap) -> GridMap

Remove every grid from the map.
"""
function Base.empty!(map::GridMap)
    while !isempty(map)
        remove_grid!(map, 1)
    end
    return map
end

function Base.show(io::IO, map::GridMap)
    print(io, "GridMap(num_grids=$(length(map)))")
end

export GridMap
export add_grid!, get_grid, remove_grid!, get_number_of_grids
