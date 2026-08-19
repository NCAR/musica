# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

# Tests for the TUV-x Grid and GridMap interface.
#
# TUV-x is an optional Fortran component. The caller includes this file only
# when `Musica.tuvx_available()` is true.

using Test
using Musica

@testset "TUV-x Grid" begin
    @testset "Construction" begin
        # From a section count
        grid = Grid(name = "test", units = "m", num_sections = 5)
        @test get_name(grid) == "test"
        @test get_units(grid) == "m"
        @test num_sections(grid) == 5
        @test length(grid) == 5

        # From edges
        edge_values = [0.0, 1.0, 2.0, 3.0, 4.0, 5.0]
        grid = Grid(name = "test", units = "m", edges = edge_values)
        @test num_sections(grid) == 5
        @test collect(edges(grid)) == edge_values

        # From midpoints
        midpoint_values = [0.5, 1.5, 2.5, 3.5, 4.5]
        grid = Grid(name = "test", units = "m", midpoints = midpoint_values)
        @test num_sections(grid) == 5
        @test collect(midpoints(grid)) == midpoint_values

        # From both
        grid = Grid(
            name = "test",
            units = "m",
            edges = edge_values,
            midpoints = midpoint_values,
        )
        @test collect(edges(grid)) == edge_values
        @test collect(midpoints(grid)) == midpoint_values

        # Integer input is converted to Float64
        grid = Grid(name = "test", units = "m", edges = [0, 1, 2])
        @test collect(edges(grid)) == [0.0, 1.0, 2.0]

        # Invalid input
        @test_throws ErrorException Grid(name = "test", units = "m")
        @test_throws ErrorException Grid(name = "test", units = "m", num_sections = 0)
    end

    @testset "Array views" begin
        grid = Grid(name = "height", units = "km", num_sections = 4)

        @test length(edges(grid)) == 5
        @test length(midpoints(grid)) == 4
        @test edges(grid) isa AbstractVector{Float64}

        # A write through the view changes the grid
        edges(grid) .= [0.0, 2.0, 4.0, 6.0, 8.0]
        @test collect(edges(grid)) == [0.0, 2.0, 4.0, 6.0, 8.0]

        midpoints(grid) .= [1.0, 3.0, 5.0, 7.0]
        @test collect(midpoints(grid)) == [1.0, 3.0, 5.0, 7.0]

        # Element assignment
        view = edges(grid)
        view[1] = -1.0
        @test edges(grid)[1] == -1.0

        # The setter functions check the length
        @test set_edges!(grid, [0.0, 1.0, 2.0, 3.0, 4.0]) === grid
        @test collect(edges(grid)) == [0.0, 1.0, 2.0, 3.0, 4.0]
        @test_throws ErrorException set_edges!(grid, [0.0, 1.0])
        @test set_midpoints!(grid, [0.5, 1.5, 2.5, 3.5]) === grid
        @test collect(midpoints(grid)) == [0.5, 1.5, 2.5, 3.5]
        @test_throws ErrorException set_midpoints!(grid, [0.5])

        # The view keeps the grid alive
        view = edges(Grid(name = "temporary", units = "m", edges = [1.0, 2.0, 3.0]))
        GC.gc()
        @test collect(view) == [1.0, 2.0, 3.0]
    end

    @testset "Display" begin
        grid = Grid(name = "height", units = "km", num_sections = 5)
        @test sprint(show, grid) == "Grid(name=\"height\", units=\"km\", num_sections=5)"
    end
end

@testset "TUV-x GridMap" begin
    @testset "Empty map" begin
        map = GridMap()
        @test length(map) == 0
        @test get_number_of_grids(map) == 0
        @test isempty(map)
        @test sprint(show, map) == "GridMap(num_grids=0)"
        @test collect(map) == []
        @test keys(map) == []
    end

    @testset "Add and get" begin
        map = GridMap()
        grid = Grid(name = "height", units = "km", edges = [0.0, 2.0, 4.0, 6.0, 8.0, 10.0])

        @test add_grid!(map, grid) === map
        @test length(map) == 1
        @test !isempty(map)

        retrieved = get_grid(map, "height", "km")
        @test get_name(retrieved) == "height"
        @test get_units(retrieved) == "km"
        @test num_sections(retrieved) == 5
        @test collect(edges(retrieved)) == [0.0, 2.0, 4.0, 6.0, 8.0, 10.0]

        # Index access is 1-based
        by_index = get_grid(map, 1)
        @test get_name(by_index) == "height"
        @test_throws BoundsError get_grid(map, 0)
        @test_throws BoundsError get_grid(map, 2)

        # An absent grid raises
        @test_throws Exception get_grid(map, "missing", "km")
    end

    @testset "Dictionary-style access" begin
        map = GridMap()
        grid = Grid(name = "wavelength", units = "nm", num_sections = 3)

        map["wavelength", "nm"] = grid
        @test length(map) == 1
        @test get_name(map["wavelength", "nm"]) == "wavelength"
        @test get_name(map[("wavelength", "nm")]) == "wavelength"
        @test get_name(map[1]) == "wavelength"

        # The key must match the name and units of the grid
        other = Grid(name = "height", units = "km", num_sections = 2)
        @test_throws ErrorException map["wavelength", "nm"] = other

        @test haskey(map, ("wavelength", "nm"))
        @test !haskey(map, ("height", "km"))
        @test !haskey(map, "not a key")
        @test ("wavelength", "nm") in map
        @test !(("height", "km") in map)
    end

    @testset "Iteration" begin
        map = GridMap()
        specifications = [("height", "km", 5), ("wavelength", "nm", 3), ("time", "s", 2)]
        for (name, units, sections) in specifications
            map[name, units] = Grid(name = name, units = units, num_sections = sections)
        end

        @test length(map) == 3
        @test eltype(GridMap) == Grid

        @test Set(keys(map)) == Set([(name, units) for (name, units, _) in specifications])

        grids = values(map)
        @test length(grids) == 3
        @test all(grid -> grid isa Grid, grids)
        @test Set(num_sections.(grids)) == Set([5, 3, 2])

        collected = [get_name(grid) for grid in map]
        @test Set(collected) == Set(["height", "wavelength", "time"])
    end

    @testset "Removal" begin
        map = GridMap()
        map["height", "km"] = Grid(name = "height", units = "km", num_sections = 5)
        map["wavelength", "nm"] = Grid(name = "wavelength", units = "nm", num_sections = 3)
        @test length(map) == 2

        @test remove_grid!(map, "height", "km") === map
        @test length(map) == 1
        @test !haskey(map, ("height", "km"))

        @test remove_grid!(map, 1) === map
        @test isempty(map)
        @test_throws BoundsError remove_grid!(map, 1)

        map["time", "s"] = Grid(name = "time", units = "s", num_sections = 2)
        map["height", "km"] = Grid(name = "height", units = "km", num_sections = 5)
        @test empty!(map) === map
        @test isempty(map)
    end

    @testset "Grid memory after transfer to a map" begin
        map = GridMap()
        grid = Grid(name = "height", units = "km", num_sections = 3)
        add_grid!(map, grid)

        # The grid reads through the map after the transfer
        @test get_name(grid) == "height"
        @test num_sections(grid) == 3

        # A write through a grid from the map reaches the map
        from_map = get_grid(map, 1)
        edges(from_map) .= [10.0, 20.0, 30.0, 40.0]
        @test collect(edges(map["height", "km"])) == [10.0, 20.0, 30.0, 40.0]

        # A grid from the map keeps the map alive
        map = nothing
        GC.gc()
        @test collect(edges(from_map)) == [10.0, 20.0, 30.0, 40.0]
    end
end
