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

@testset "TUV-x Profile" begin
    @testset "Construction" begin
        grid = Grid(name = "height", units = "km", num_sections = 3)

        # Neither edge_values nor midpoint_values given: zero-filled
        profile = Profile(name = "temperature", units = "K", grid = grid)
        @test get_name(profile) == "temperature"
        @test get_units(profile) == "K"
        @test num_sections(profile) == 3
        @test length(profile) == 3
        @test collect(edge_values(profile)) == zeros(4)
        @test collect(midpoint_values(profile)) == zeros(3)
        @test collect(layer_densities(profile)) == zeros(3)
        @test exo_layer_density(profile) == 0.0

        # From edge_values only
        profile = Profile(
            name = "temperature",
            units = "K",
            grid = grid,
            edge_values = [270.0, 268.0, 266.0, 264.0],
        )
        @test collect(edge_values(profile)) == [270.0, 268.0, 266.0, 264.0]
        @test collect(midpoint_values(profile)) == [269.0, 267.0, 265.0]

        # From midpoint_values only
        profile = Profile(
            name = "temperature",
            units = "K",
            grid = grid,
            midpoint_values = [269.0, 267.0, 265.0],
        )
        @test collect(midpoint_values(profile)) == [269.0, 267.0, 265.0]
        @test collect(edge_values(profile)) == [270.0, 268.0, 266.0, 264.0]

        # From both
        profile = Profile(
            name = "temperature",
            units = "K",
            grid = grid,
            edge_values = [270.0, 268.0, 266.0, 264.0],
            midpoint_values = [269.0, 267.0, 265.0],
        )
        @test collect(edge_values(profile)) == [270.0, 268.0, 266.0, 264.0]
        @test collect(midpoint_values(profile)) == [269.0, 267.0, 265.0]

        # layer_densities
        profile = Profile(
            name = "density",
            units = "molecule cm-3",
            grid = grid,
            midpoint_values = [1.0, 2.0, 3.0],
            layer_densities = [10.0, 20.0, 30.0],
        )
        @test collect(layer_densities(profile)) == [10.0, 20.0, 30.0]

        # exo_layer_density
        profile = Profile(
            name = "density",
            units = "molecule cm-3",
            grid = grid,
            midpoint_values = [1.0, 2.0, 3.0],
            exo_layer_density = 5.0,
        )
        @test exo_layer_density(profile) == 5.0

        # Invalid input
        @test_throws ErrorException Profile(
            name = "bad",
            units = "K",
            grid = grid,
            midpoint_values = [1.0, 2.0, 3.0],
            layer_densities = [1.0, 2.0, 3.0],
            calculate_layer_densities = true,
        )
        @test_throws ErrorException Profile(
            name = "bad",
            units = "K",
            grid = grid,
            midpoint_values = [1.0, 2.0, 3.0],
            exo_layer_density = -1.0,
        )
    end

    @testset "Array views" begin
        grid = Grid(name = "height", units = "km", num_sections = 4)
        profile = Profile(name = "temperature", units = "K", grid = grid)

        @test length(edge_values(profile)) == 5
        @test length(midpoint_values(profile)) == 4
        @test length(layer_densities(profile)) == 4
        @test edge_values(profile) isa AbstractVector{Float64}

        # A write through the view changes the profile
        edge_values(profile) .= [1.0, 2.0, 3.0, 4.0, 5.0]
        @test collect(edge_values(profile)) == [1.0, 2.0, 3.0, 4.0, 5.0]

        midpoint_values(profile) .= [1.5, 2.5, 3.5, 4.5]
        @test collect(midpoint_values(profile)) == [1.5, 2.5, 3.5, 4.5]

        layer_densities(profile) .= [10.0, 20.0, 30.0, 40.0]
        @test collect(layer_densities(profile)) == [10.0, 20.0, 30.0, 40.0]

        # The setter functions check the length
        @test set_edge_values!(profile, [0.0, 1.0, 2.0, 3.0, 4.0]) === profile
        @test_throws ErrorException set_edge_values!(profile, [0.0, 1.0])
        @test set_midpoint_values!(profile, [0.5, 1.5, 2.5, 3.5]) === profile
        @test_throws ErrorException set_midpoint_values!(profile, [0.5])
        @test set_layer_densities!(profile, [1.0, 2.0, 3.0, 4.0]) === profile
        @test_throws ErrorException set_layer_densities!(profile, [1.0])

        # The view keeps the profile alive
        temp_grid = Grid(name = "height", units = "km", num_sections = 2)
        view = edge_values(
            Profile(name = "t", units = "K", grid = temp_grid, edge_values = [1.0, 2.0, 3.0]),
        )
        GC.gc()
        @test collect(view) == [1.0, 2.0, 3.0]
    end

    @testset "Exo layer density" begin
        grid = Grid(name = "height", units = "km", num_sections = 2)
        profile = Profile(name = "density", units = "molecule cm-3", grid = grid)

        @test set_exo_layer_density!(profile, 3.0) === profile
        @test exo_layer_density(profile) == 3.0

        @test calculate_exo_layer_density!(profile, 8.0) === profile
        @test exo_layer_density(profile) >= 0.0
    end

    @testset "Calculate layer densities" begin
        grid = Grid(name = "height", units = "km", edges = [0.0, 1.0, 2.0])
        profile = Profile(
            name = "O2",
            units = "molecule cm-3",
            grid = grid,
            midpoint_values = [2.0, 3.0],
        )

        @test calculate_layer_densities!(profile, grid) === profile
        # height/km + molecule cm-3 uses a 1e5 conversion factor
        @test collect(layer_densities(profile)) == [2.0e5, 3.0e5]

        other_grid = Grid(name = "wavelength", units = "nm", edges = [0.0, 1.0, 2.0])
        other_profile = Profile(
            name = "flux",
            units = "photon cm-2 s-1",
            grid = other_grid,
            midpoint_values = [2.0, 3.0],
        )
        calculate_layer_densities!(other_profile, other_grid)
        @test collect(layer_densities(other_profile)) == [2.0, 3.0]
    end

    @testset "Display" begin
        grid = Grid(name = "height", units = "km", num_sections = 5)
        profile = Profile(name = "temperature", units = "K", grid = grid)
        @test sprint(show, profile) ==
              "Profile(name=\"temperature\", units=\"K\", num_sections=5)"
    end
end

@testset "TUV-x ProfileMap" begin
    @testset "Empty map" begin
        map = ProfileMap()
        @test length(map) == 0
        @test get_number_of_profiles(map) == 0
        @test isempty(map)
        @test sprint(show, map) == "ProfileMap(num_profiles=0)"
        @test collect(map) == []
        @test keys(map) == []
    end

    @testset "Add and get" begin
        map = ProfileMap()
        grid = Grid(name = "height", units = "km", num_sections = 3)
        profile = Profile(
            name = "temperature",
            units = "K",
            grid = grid,
            midpoint_values = [270.0, 268.0, 266.0],
        )

        @test add_profile!(map, profile) === map
        @test length(map) == 1
        @test !isempty(map)

        retrieved = get_profile(map, "temperature", "K")
        @test get_name(retrieved) == "temperature"
        @test get_units(retrieved) == "K"
        @test collect(midpoint_values(retrieved)) == [270.0, 268.0, 266.0]

        # Index access is 1-based
        by_index = get_profile(map, 1)
        @test get_name(by_index) == "temperature"
        @test_throws BoundsError get_profile(map, 0)
        @test_throws BoundsError get_profile(map, 2)

        # An absent profile raises
        @test_throws Exception get_profile(map, "missing", "K")
    end

    @testset "Dictionary-style access" begin
        map = ProfileMap()
        grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        profile = Profile(name = "flux", units = "photon cm-2 s-1", grid = grid)

        map["flux", "photon cm-2 s-1"] = profile
        @test length(map) == 1
        @test get_name(map["flux", "photon cm-2 s-1"]) == "flux"
        @test get_name(map[("flux", "photon cm-2 s-1")]) == "flux"
        @test get_name(map[1]) == "flux"

        # The key must match the name and units of the profile
        other = Profile(name = "temperature", units = "K", grid = grid)
        @test_throws ErrorException map["flux", "photon cm-2 s-1"] = other

        @test haskey(map, ("flux", "photon cm-2 s-1"))
        @test !haskey(map, ("temperature", "K"))
        @test !haskey(map, "not a key")
        @test ("flux", "photon cm-2 s-1") in map
        @test !(("temperature", "K") in map)
    end

    @testset "Iteration" begin
        map = ProfileMap()
        grid = Grid(name = "height", units = "km", num_sections = 3)
        specifications = [("temperature", "K"), ("pressure", "Pa"), ("density", "molecule cm-3")]
        for (name, units) in specifications
            map[name, units] = Profile(name = name, units = units, grid = grid)
        end

        @test length(map) == 3
        @test eltype(ProfileMap) == Profile

        @test Set(keys(map)) == Set(specifications)

        profiles = values(map)
        @test length(profiles) == 3
        @test all(profile -> profile isa Profile, profiles)

        collected = [get_name(profile) for profile in map]
        @test Set(collected) == Set(["temperature", "pressure", "density"])
    end

    @testset "Removal" begin
        map = ProfileMap()
        grid = Grid(name = "height", units = "km", num_sections = 3)
        map["temperature", "K"] = Profile(name = "temperature", units = "K", grid = grid)
        map["pressure", "Pa"] = Profile(name = "pressure", units = "Pa", grid = grid)
        @test length(map) == 2

        @test remove_profile!(map, "temperature", "K") === map
        @test length(map) == 1
        @test !haskey(map, ("temperature", "K"))

        @test remove_profile!(map, 1) === map
        @test isempty(map)
        @test_throws BoundsError remove_profile!(map, 1)

        map["pressure", "Pa"] = Profile(name = "pressure", units = "Pa", grid = grid)
        map["temperature", "K"] = Profile(name = "temperature", units = "K", grid = grid)
        @test empty!(map) === map
        @test isempty(map)
    end

    @testset "Profile memory after transfer to a map" begin
        map = ProfileMap()
        grid = Grid(name = "height", units = "km", num_sections = 3)
        profile = Profile(
            name = "temperature",
            units = "K",
            grid = grid,
            midpoint_values = [270.0, 268.0, 266.0],
        )
        add_profile!(map, profile)

        # The profile reads through the map after the transfer
        @test get_name(profile) == "temperature"
        @test num_sections(profile) == 3

        # A write through a profile from the map reaches the map
        from_map = get_profile(map, 1)
        midpoint_values(from_map) .= [1.0, 2.0, 3.0]
        @test collect(midpoint_values(map["temperature", "K"])) == [1.0, 2.0, 3.0]

        # A profile from the map keeps the map alive
        map = nothing
        GC.gc()
        @test collect(midpoint_values(from_map)) == [1.0, 2.0, 3.0]
    end
end
