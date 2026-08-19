# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const GridPtr = CxxWrap.CxxWrapCore.CxxPtr{CppGrid}

"""
    GridView <: AbstractVector{Float64}

A zero-copy view into the edge or midpoint array of a [`Grid`](@ref).

The view shares memory with the TUV-x grid, so a write to the view changes the
grid itself. The view also holds a reference to its grid. The reference stops
the garbage collector from freeing the grid while the view is alive.

Get a new view after you add the grid to a [`GridMap`](@ref). The map takes over
the memory of the grid, and an older view then points to memory that is free.
"""
struct GridView <: AbstractVector{Float64}
    _data::Vector{Float64}
    _grid::Any  # prevents GC of the parent Grid while this view is alive
end

Base.size(view::GridView) = size(view._data)
Base.IndexStyle(::Type{GridView}) = IndexLinear()
Base.getindex(view::GridView, i::Int) = view._data[i]
Base.setindex!(view::GridView, value, i::Int) = (view._data[i] = value)

"""
    Grid

A grid on which TUV-x profiles are defined.

Typical grids are a vertical grid and a wavelength grid.

# Constructor

    Grid(; name, units, num_sections=nothing, edges=nothing, midpoints=nothing)

Give at least one of `num_sections`, `edges`, or `midpoints`.

- `name::AbstractString`: The name of the grid
- `units::AbstractString`: The units of the grid values
- `num_sections::Integer`: The number of grid sections
- `edges::AbstractVector{<:Real}`: The edge values, of length `num_sections + 1`
- `midpoints::AbstractVector{<:Real}`: The midpoint values, of length `num_sections`

# Example

```julia
grid = Grid(name = "height", units = "km", edges = [0.0, 2.0, 4.0, 6.0])
get_name(grid)      # "height"
num_sections(grid)  # 3
midpoints(grid) .= [1.0, 3.0, 5.0]
```
"""
mutable struct Grid
    _ptr::GridPtr
    _owner::Any  # the GridMap that owns the TUV-x grid, or nothing

    function Grid(ptr::GridPtr, owner)
        obj = new(ptr, owner)
        finalizer(obj) do g
            cpp_delete_grid(g._ptr)
        end
        return obj
    end
end

function Grid(;
    name::AbstractString,
    units::AbstractString,
    num_sections::Union{Integer,Nothing} = nothing,
    edges::Union{AbstractVector{<:Real},Nothing} = nothing,
    midpoints::Union{AbstractVector{<:Real},Nothing} = nothing,
)
    if num_sections === nothing && edges === nothing && midpoints === nothing
        error("At least one of num_sections, edges, or midpoints must be provided.")
    end
    n_sections = if num_sections !== nothing
        Int(num_sections)
    elseif edges !== nothing
        length(edges) - 1
    else
        length(midpoints)
    end
    n_sections > 0 || error("num_sections must be greater than 0.")

    grid = Grid(cpp_create_grid(String(name), String(units), Int64(n_sections)), nothing)
    edges !== nothing && set_edges!(grid, edges)
    midpoints !== nothing && set_midpoints!(grid, midpoints)
    return grid
end

"""
    get_name(grid::Grid) -> String

Get the name of the grid.
"""
get_name(grid::Grid) = String(cpp_grid_name(grid._ptr))

"""
    get_units(grid::Grid) -> String

Get the units of the grid values.
"""
get_units(grid::Grid) = String(cpp_grid_units(grid._ptr))

"""
    num_sections(grid::Grid) -> Int

Get the number of sections in the grid.
"""
num_sections(grid::Grid) = Int(cpp_grid_num_sections(grid._ptr))

Base.length(grid::Grid) = num_sections(grid)

"""
    edges(grid::Grid) -> GridView

Get a zero-copy view of the grid edges. The view has length `num_sections + 1`.

A write to the view changes the grid.
"""
function edges(grid::Grid)
    n_edges = num_sections(grid) + 1
    address = cpp_grid_edges_pointer(grid._ptr)
    data = unsafe_wrap(Array, Ptr{Float64}(address), n_edges; own = false)
    return GridView(data, grid)
end

"""
    midpoints(grid::Grid) -> GridView

Get a zero-copy view of the grid midpoints. The view has length `num_sections`.

A write to the view changes the grid.
"""
function midpoints(grid::Grid)
    n_midpoints = num_sections(grid)
    address = cpp_grid_midpoints_pointer(grid._ptr)
    data = unsafe_wrap(Array, Ptr{Float64}(address), n_midpoints; own = false)
    return GridView(data, grid)
end

"""
    set_edges!(grid::Grid, values::AbstractVector{<:Real}) -> Grid

Copy `values` into the grid edges. The length must equal `num_sections + 1`.
"""
function set_edges!(grid::Grid, values::AbstractVector{<:Real})
    view = edges(grid)
    length(values) == length(view) ||
        error("edges must have length $(length(view)) (num_sections + 1).")
    view .= values
    return grid
end

"""
    set_midpoints!(grid::Grid, values::AbstractVector{<:Real}) -> Grid

Copy `values` into the grid midpoints. The length must equal `num_sections`.
"""
function set_midpoints!(grid::Grid, values::AbstractVector{<:Real})
    view = midpoints(grid)
    length(values) == length(view) ||
        error("midpoints must have length $(length(view)) (num_sections).")
    view .= values
    return grid
end

function Base.show(io::IO, grid::Grid)
    print(
        io,
        "Grid(name=\"$(get_name(grid))\", units=\"$(get_units(grid))\", ",
        "num_sections=$(num_sections(grid)))",
    )
end

export Grid, GridView
export get_name, get_units, num_sections
export edges, midpoints, set_edges!, set_midpoints!
