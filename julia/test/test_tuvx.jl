# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

# Tests for the TUV-x Grid, GridMap, Profile, ProfileMap, Radiator, and
# RadiatorMap interface.
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
            Profile(
                name = "t",
                units = "K",
                grid = temp_grid,
                edge_values = [1.0, 2.0, 3.0],
            ),
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
        specifications =
            [("temperature", "K"), ("pressure", "Pa"), ("density", "molecule cm-3")]
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

@testset "TUV-x Radiator" begin
    @testset "Construction" begin
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)

        # No arrays given: zero-filled
        radiator = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )
        @test get_name(radiator) == "aerosol"
        @test num_height_sections(radiator) == 2
        @test num_wavelength_sections(radiator) == 3
        @test size(optical_depths(radiator)) == (2, 3)
        @test collect(optical_depths(radiator)) == zeros(2, 3)
        @test collect(single_scattering_albedos(radiator)) == zeros(2, 3)
        @test collect(asymmetry_factors(radiator)) == zeros(2, 3)

        # Arrays given at construction. h varies down rows, w across columns.
        od = [1.0 2.0 3.0; 4.0 5.0 6.0]
        ssa = [0.1 0.2 0.3; 0.4 0.5 0.6]
        asy = [0.01 0.02 0.03; 0.04 0.05 0.06]
        radiator = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
            optical_depths = od,
            single_scattering_albedos = ssa,
            asymmetry_factors = asy,
        )
        @test collect(optical_depths(radiator)) == od
        @test collect(single_scattering_albedos(radiator)) == ssa
        @test collect(asymmetry_factors(radiator)) == asy

        # Element-wise access preserves (height, wavelength) orientation
        @test optical_depths(radiator)[1, 1] == 1.0
        @test optical_depths(radiator)[2, 1] == 4.0
        @test optical_depths(radiator)[1, 3] == 3.0
        @test optical_depths(radiator)[2, 3] == 6.0
    end

    @testset "Array views" begin
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        radiator = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )

        @test size(optical_depths(radiator)) == (2, 3)
        @test optical_depths(radiator) isa AbstractMatrix{Float64}

        # A write through the view changes the radiator
        optical_depths(radiator) .= [1.0 2.0 3.0; 4.0 5.0 6.0]
        @test collect(optical_depths(radiator)) == [1.0 2.0 3.0; 4.0 5.0 6.0]

        single_scattering_albedos(radiator) .= [0.1 0.2 0.3; 0.4 0.5 0.6]
        @test collect(single_scattering_albedos(radiator)) == [0.1 0.2 0.3; 0.4 0.5 0.6]

        asymmetry_factors(radiator) .= [0.01 0.02 0.03; 0.04 0.05 0.06]
        @test collect(asymmetry_factors(radiator)) == [0.01 0.02 0.03; 0.04 0.05 0.06]

        # The setter functions check the shape
        @test set_optical_depths!(radiator, zeros(2, 3)) === radiator
        @test_throws ErrorException set_optical_depths!(radiator, zeros(3, 2))
        @test set_single_scattering_albedos!(radiator, ones(2, 3)) === radiator
        @test_throws ErrorException set_single_scattering_albedos!(radiator, ones(1, 3))
        @test set_asymmetry_factors!(radiator, ones(2, 3) .* 0.5) === radiator
        @test_throws ErrorException set_asymmetry_factors!(radiator, ones(2, 2))

        # The view keeps the radiator alive
        temp_height = Grid(name = "height", units = "km", num_sections = 1)
        temp_wavelength = Grid(name = "wavelength", units = "nm", num_sections = 2)
        view = optical_depths(
            Radiator(
                name = "temp",
                height_grid = temp_height,
                wavelength_grid = temp_wavelength,
                optical_depths = [1.0 2.0],
            ),
        )
        GC.gc()
        @test collect(view) == [1.0 2.0]
    end

    @testset "Display" begin
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        radiator = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )
        @test sprint(show, radiator) ==
              "Radiator(name=\"aerosol\", num_height_sections=2, num_wavelength_sections=3)"
    end
end

@testset "TUV-x RadiatorMap" begin
    @testset "Empty map" begin
        map = RadiatorMap()
        @test length(map) == 0
        @test get_number_of_radiators(map) == 0
        @test isempty(map)
        @test sprint(show, map) == "RadiatorMap(num_radiators=0)"
        @test collect(map) == []
        @test keys(map) == []
    end

    @testset "Add and get" begin
        map = RadiatorMap()
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        radiator = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
            optical_depths = [1.0 2.0 3.0; 4.0 5.0 6.0],
        )

        @test add_radiator!(map, radiator) === map
        @test length(map) == 1
        @test !isempty(map)

        retrieved = get_radiator(map, "aerosol")
        @test get_name(retrieved) == "aerosol"
        @test collect(optical_depths(retrieved)) == [1.0 2.0 3.0; 4.0 5.0 6.0]

        # Index access is 1-based
        by_index = get_radiator(map, 1)
        @test get_name(by_index) == "aerosol"
        @test_throws BoundsError get_radiator(map, 0)
        @test_throws BoundsError get_radiator(map, 2)

        # An absent radiator raises
        @test_throws Exception get_radiator(map, "missing")
    end

    @testset "Dictionary-style access" begin
        map = RadiatorMap()
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        radiator = Radiator(
            name = "air",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )

        map["air"] = radiator
        @test length(map) == 1
        @test get_name(map["air"]) == "air"
        @test get_name(map[1]) == "air"

        # The key must match the name of the radiator
        other = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )
        @test_throws ErrorException map["air"] = other

        @test haskey(map, "air")
        @test !haskey(map, "aerosol")
        @test "air" in map
        @test !("aerosol" in map)
    end

    @testset "Iteration" begin
        map = RadiatorMap()
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        names = ["aerosol", "air", "surface_albedo"]
        for name in names
            map[name] = Radiator(
                name = name,
                height_grid = height_grid,
                wavelength_grid = wavelength_grid,
            )
        end

        @test length(map) == 3
        @test eltype(RadiatorMap) == Radiator

        @test Set(keys(map)) == Set(names)

        radiators = values(map)
        @test length(radiators) == 3
        @test all(radiator -> radiator isa Radiator, radiators)

        collected = [get_name(radiator) for radiator in map]
        @test Set(collected) == Set(names)
    end

    @testset "Removal" begin
        map = RadiatorMap()
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        map["aerosol"] = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )
        map["air"] = Radiator(
            name = "air",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )
        @test length(map) == 2

        @test remove_radiator!(map, "aerosol") === map
        @test length(map) == 1
        @test !haskey(map, "aerosol")

        @test remove_radiator!(map, 1) === map
        @test isempty(map)
        @test_throws BoundsError remove_radiator!(map, 1)

        map["air"] = Radiator(
            name = "air",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )
        map["aerosol"] = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
        )
        @test empty!(map) === map
        @test isempty(map)
    end

    @testset "Radiator memory after transfer to a map" begin
        map = RadiatorMap()
        height_grid = Grid(name = "height", units = "km", num_sections = 2)
        wavelength_grid = Grid(name = "wavelength", units = "nm", num_sections = 3)
        radiator = Radiator(
            name = "aerosol",
            height_grid = height_grid,
            wavelength_grid = wavelength_grid,
            optical_depths = [1.0 2.0 3.0; 4.0 5.0 6.0],
        )
        add_radiator!(map, radiator)

        # The radiator reads through the map after the transfer
        @test get_name(radiator) == "aerosol"
        @test num_height_sections(radiator) == 2

        # A write through a radiator from the map reaches the map
        from_map = get_radiator(map, 1)
        optical_depths(from_map) .= [10.0 20.0 30.0; 40.0 50.0 60.0]
        @test collect(optical_depths(map["aerosol"])) == [10.0 20.0 30.0; 40.0 50.0 60.0]

        # A radiator from the map keeps the map alive
        map = nothing
        GC.gc()
        @test collect(optical_depths(from_map)) == [10.0 20.0 30.0; 40.0 50.0 60.0]
    end
end

# Fixture used with configs/tuvx/full_from_host/config_python.json, which
# supplies no grids or profiles of its own — everything below is required for
# TUV-x to have what it needs to run. That config's data-file paths are
# relative to its own directory, so callers must cd there themselves; see
# `_create_test_tuvx` below.
function _tuvx_fixed_grid_map()
    heights = Grid(name = "height", units = "km", num_sections = 3)
    set_edges!(heights, [0.0, 10.0, 20.0, 30.0])
    set_midpoints!(heights, 0.5 .* (edges(heights)[1:(end-1)] .+ edges(heights)[2:end]))
    wavelengths = Grid(name = "wavelength", units = "nm", num_sections = 5)
    set_edges!(wavelengths, [300.0, 400.0, 500.0, 600.0, 700.0, 800.0])
    set_midpoints!(
        wavelengths,
        0.5 .* (edges(wavelengths)[1:(end-1)] .+ edges(wavelengths)[2:end]),
    )
    grid_map = GridMap()
    grid_map["height", "km"] = heights
    grid_map["wavelength", "nm"] = wavelengths
    return grid_map
end

function _tuvx_profile_map(grid_map)
    height_grid = grid_map["height", "km"]
    wavelength_grid = grid_map["wavelength", "nm"]
    normalized = exp.(-(midpoints(height_grid) .- 0.5) ./ 25)

    ozone = Profile(
        name = "O3",
        units = "molecule cm-3",
        grid = height_grid,
        midpoint_values = 1.0e-6 * 2.54e19 .* normalized,
        calculate_layer_densities = true,
    )
    air = Profile(
        name = "air",
        units = "molecule cm-3",
        grid = height_grid,
        midpoint_values = 2.54e19 .* normalized,
        calculate_layer_densities = true,
    )
    oxygen = Profile(
        name = "O2",
        units = "molecule cm-3",
        grid = height_grid,
        midpoint_values = 0.21 * 2.54e19 .* normalized,
        calculate_layer_densities = true,
    )
    calculate_exo_layer_density!(ozone, 8.5)
    calculate_exo_layer_density!(oxygen, 8.5)
    calculate_exo_layer_density!(air, 8.5)

    temperature = Profile(
        name = "temperature",
        units = "K",
        grid = height_grid,
        midpoint_values = 298.0 .* normalized,
    )
    surface_albedo = Profile(
        name = "surface albedo",
        units = "none",
        grid = wavelength_grid,
        midpoint_values = 0.1 .* ones(num_sections(wavelength_grid)),
    )
    et_flux = Profile(
        name = "extraterrestrial flux",
        units = "photon cm-2 s-1",
        grid = wavelength_grid,
        midpoint_values = (1.0e18 * 1420.0 / 615.0 * 0.0001) .*
                          ones(num_sections(wavelength_grid)),
    )

    profile_map = ProfileMap()
    profile_map["O3", "molecule cm-3"] = ozone
    profile_map["air", "molecule cm-3"] = air
    profile_map["O2", "molecule cm-3"] = oxygen
    profile_map["temperature", "K"] = temperature
    profile_map["surface albedo", "none"] = surface_albedo
    profile_map["extraterrestrial flux", "photon cm-2 s-1"] = et_flux
    return profile_map
end

function _tuvx_radiator_map(grid_map)
    height_grid = grid_map["height", "km"]
    wavelength_grid = grid_map["wavelength", "nm"]
    n_h = num_sections(height_grid)
    n_w = num_sections(wavelength_grid)
    ssa = fill(0.99, n_h, n_w)
    asymmetry = fill(0.61, n_h, n_w)
    decay = exp.(-(midpoints(height_grid) .- 120) ./ 7)

    clouds = Radiator(
        name = "clouds",
        height_grid = height_grid,
        wavelength_grid = wavelength_grid,
        optical_depths = repeat(1.0e-6 .* decay, 1, n_w),
        single_scattering_albedos = ssa,
        asymmetry_factors = asymmetry,
    )
    balloons = Radiator(
        name = "hot air balloons",
        height_grid = height_grid,
        wavelength_grid = wavelength_grid,
        optical_depths = repeat(1.8e-8 .* decay, 1, n_w),
        single_scattering_albedos = ssa,
        asymmetry_factors = asymmetry,
    )

    radiator_map = RadiatorMap()
    radiator_map["clouds"] = clouds
    radiator_map["hot air balloons"] = balloons
    return radiator_map
end

const _TUVX_CONFIG_PATH = joinpath(
    @__DIR__,
    "..",
    "..",
    "configs",
    "tuvx",
    "full_from_host",
    "config_python.json",
)

# config_python.json's data-file paths are relative to its own directory, and
# TUVX does not do any directory handling itself, so the caller must be in
# that directory for those paths to resolve. This helper does the cd, the
# same way a real caller would.
function _create_test_tuvx(grid_map, profile_map, radiator_map)
    return cd(dirname(_TUVX_CONFIG_PATH)) do
        TUVX(
            grid_map = grid_map,
            profile_map = profile_map,
            radiator_map = radiator_map,
            config_path = _TUVX_CONFIG_PATH,
        )
    end
end

@testset "TUV-x TUVX" begin
    @testset "Create from file" begin
        grid_map = _tuvx_fixed_grid_map()
        profile_map = _tuvx_profile_map(grid_map)
        radiator_map = _tuvx_radiator_map(grid_map)
        tuvx = _create_test_tuvx(grid_map, profile_map, radiator_map)

        @test photolysis_rate_constant_count(tuvx) == 3
        @test heating_rate_count(tuvx) == 2
        @test dose_rate_count(tuvx) == 3
        @test num_height_midpoints(tuvx) == 3
        @test num_wavelength_midpoints(tuvx) == 5

        @test Set(keys(photolysis_rate_names(tuvx))) == Set(["jfoo", "jbar", "jbaz"])
        @test Set(keys(heating_rate_names(tuvx))) == Set(["jfoo", "jbar"])
        @test length(dose_rate_names(tuvx)) == 3

        result = run!(tuvx, 0.3, 1.0)
        @test size(result.photolysis_rate_constants) == (3, 4)
        @test size(result.heating_rates) == (2, 4)
        @test size(result.dose_rates) == (3, 4)
        @test size(result.actinic_flux) == (5, 4, 3)
        @test size(result.spectral_irradiance) == (5, 4, 3)

        @test all(result.photolysis_rate_constants .>= 0.0)
        @test any(result.photolysis_rate_constants .> 0.0)

        # get_photolysis_rate_constant selects the reaction's row (all
        # vertical edges for one reaction), matching a manual index lookup.
        for (name, idx) in photolysis_rate_names(tuvx)
            @test get_photolysis_rate_constant(
                tuvx,
                name,
                result.photolysis_rate_constants,
            ) == result.photolysis_rate_constants[idx+1, :]
        end
        for (name, idx) in heating_rate_names(tuvx)
            @test get_heating_rate(tuvx, name, result.heating_rates) ==
                  result.heating_rates[idx+1, :]
        end
        for (name, idx) in dose_rate_names(tuvx)
            @test get_dose_rate(tuvx, name, result.dose_rates) ==
                  result.dose_rates[idx+1, :]
        end
        @test_throws ErrorException get_photolysis_rate_constant(
            tuvx,
            "not a reaction",
            result.photolysis_rate_constants,
        )
    end

    @testset "Create from string" begin
        grid_map = _tuvx_fixed_grid_map()
        profile_map = _tuvx_profile_map(grid_map)
        radiator_map = _tuvx_radiator_map(grid_map)
        config_string = read(_TUVX_CONFIG_PATH, String)

        # TUVX does no directory handling, so the caller must already be in
        # the config's directory for its relative data-file paths to resolve.
        tuvx = cd(dirname(_TUVX_CONFIG_PATH)) do
            TUVX(
                grid_map = grid_map,
                profile_map = profile_map,
                radiator_map = radiator_map,
                config_string = config_string,
            )
        end

        @test photolysis_rate_constant_count(tuvx) == 3
        @test heating_rate_count(tuvx) == 2
        @test dose_rate_count(tuvx) == 3

        result = run!(tuvx, 0.3, 1.0)
        @test size(result.photolysis_rate_constants) == (3, 4)
    end

    @testset "Doubling concentrations decreases photolysis rates" begin
        grid_map = _tuvx_fixed_grid_map()
        profile_map = _tuvx_profile_map(grid_map)
        radiator_map = _tuvx_radiator_map(grid_map)
        tuvx = _create_test_tuvx(grid_map, profile_map, radiator_map)
        names = photolysis_rate_names(tuvx)

        before = run!(tuvx, 0.3, 1.0).photolysis_rate_constants

        height_grid = grid_map["height", "km"]
        for (name, units) in
            (("O3", "molecule cm-3"), ("O2", "molecule cm-3"), ("air", "molecule cm-3"))
            profile = profile_map[name, units]
            set_midpoint_values!(profile, 2.0 .* midpoint_values(profile))
            calculate_layer_densities!(profile, height_grid)
            calculate_exo_layer_density!(profile, 8.5)
        end

        after = run!(tuvx, 0.3, 1.0).photolysis_rate_constants

        for reaction in ("jfoo", "jbar", "jbaz")
            i = names[reaction] + 1
            @test all(after[i, :] .<= before[i, :])
        end
    end

    @testset "Errors" begin
        grid_map = _tuvx_fixed_grid_map()
        profile_map = _tuvx_profile_map(grid_map)
        radiator_map = _tuvx_radiator_map(grid_map)

        @test_throws ErrorException TUVX(
            grid_map = grid_map,
            profile_map = profile_map,
            radiator_map = radiator_map,
        )
        @test_throws ErrorException TUVX(
            grid_map = grid_map,
            profile_map = profile_map,
            radiator_map = radiator_map,
            config_path = _TUVX_CONFIG_PATH,
            config_string = "{}",
        )
        @test_throws Exception TUVX(
            grid_map = grid_map,
            profile_map = profile_map,
            radiator_map = radiator_map,
            config_path = "non_existent_config.json",
        )
    end

    @testset "Map accessors" begin
        grid_map = _tuvx_fixed_grid_map()
        profile_map = _tuvx_profile_map(grid_map)
        radiator_map = _tuvx_radiator_map(grid_map)
        tuvx = _create_test_tuvx(grid_map, profile_map, radiator_map)

        returned_grids = get_grid_map(tuvx)
        @test returned_grids isa GridMap
        @test get_name(returned_grids["height", "km"]) == "height"
        @test num_sections(returned_grids["height", "km"]) == 3

        returned_profiles = get_profile_map(tuvx)
        @test returned_profiles isa ProfileMap
        @test haskey(returned_profiles, ("temperature", "K"))

        returned_radiators = get_radiator_map(tuvx)
        @test returned_radiators isa RadiatorMap
        @test haskey(returned_radiators, "clouds")
    end
end

# Mirrors tuv-x's own test/unit/radiative_transfer/solver_from_host.F90
# (test_core_with_host_radiation_field) and the reference tests in
# src/test/unit/tuvx/tuvx_c_api.cpp and python/test/integration/, using the
# same config and the same hand-computed reference formula, translated to
# the Julia API. The config and reference values come directly from that
# test, not reconstructed by hand.
@testset "TUV-x RadiationFieldUpdater" begin
    @testset "No updater for a non-host solver" begin
        grid_map = _tuvx_fixed_grid_map()
        profile_map = _tuvx_profile_map(grid_map)
        radiator_map = _tuvx_radiator_map(grid_map)
        tuvx = _create_test_tuvx(grid_map, profile_map, radiator_map)

        @test get_radiation_field_updater(tuvx) === nothing
    end

    @testset "Host-supplied radiation field drives photolysis and dose rates" begin
        config_path = joinpath(
            @__DIR__,
            "..",
            "..",
            "configs",
            "tuvx",
            "host_radiation_field",
            "config.json",
        )
        tuvx = TUVX(
            grid_map = GridMap(),
            profile_map = ProfileMap(),
            radiator_map = RadiatorMap(),
            config_path = config_path,
        )

        updater = get_radiation_field_updater(tuvx)
        @test updater isa RadiationFieldUpdater

        n_int = 5  # height grid: 1 to 5 km, delta 1 km -> 4 cells + 1
        n_bin = 6  # wavelength grid: 400 to 700 nm, delta 50 nm -> 6 cells
        etfl = 1.0e14  # photon cm^-2 s^-1, every bin
        xsqy = Dict("jfoo" => 2.0 * 0.5, "jbar" => 4.0 * 0.25)  # cross section * quantum yield
        lambda_bins = [425.0, 475.0, 525.0, 575.0, 625.0, 675.0]  # wavelength bin midpoints [nm]
        # dose rate "all bins" weights every bin; "upper bins" weights only bins above 500 nm
        dose_weights = Dict(
            "all bins" => ones(n_bin),
            "upper bins" => Float64.(lambda_bins .> 500.0),
        )
        earth_sun_distance = 0.9
        hc = 6.626068e-34 * 2.99792458e8  # Planck's constant * speed of light [J m]
        sza = deg2rad(42.0)

        # Arrays have shape (num_vertical_interfaces, num_wavelength_bins); interface 1 is
        # the lowest altitude. 1-based interface/bin numbers match tuv-x's own test exactly.
        I = 1:n_int
        B = (1:n_bin)'
        direct_actinic_flux = 0.2 .* I .+ 0.05 .* B
        upward_actinic_flux = 0.01 .* I .+ 0.0 .* B
        downward_actinic_flux = 0.0 .* I .+ 0.03 .* B
        direct_irradiance = 0.1 .* I .+ 0.02 .* B
        upward_irradiance = 0.004 .* I .+ 0.0 .* B
        downward_irradiance = 0.0 .* I .+ 0.006 .* B

        # TUV-x forms the photolysis rate constants from the actinic flux components
        # alone, so this call supplies only those three -- the irradiance components
        # default to zero, so the dose rates come back zero.
        update!(updater, direct_actinic_flux, upward_actinic_flux, downward_actinic_flux)

        result = run!(tuvx, sza, earth_sun_distance)
        names = photolysis_rate_names(tuvx)

        total_flux_per_interface =
            vec(sum(direct_actinic_flux .+ upward_actinic_flux .+ downward_actinic_flux, dims = 2))
        for (name, xsqy_value) in xsqy
            expected = etfl * earth_sun_distance * xsqy_value .* total_flux_per_interface
            actual = result.photolysis_rate_constants[names[name]+1, :]
            @test actual ≈ expected rtol = 1.0e-8
        end
        @test all(result.dose_rates .≈ 0.0)

        # A second call that also supplies the irradiance components must produce
        # non-zero dose rates, matching the exact energy-flux formula.
        update!(
            updater,
            direct_actinic_flux,
            upward_actinic_flux,
            downward_actinic_flux;
            direct_irradiance = direct_irradiance,
            upward_irradiance = upward_irradiance,
            downward_irradiance = downward_irradiance,
        )

        result2 = run!(tuvx, sza, earth_sun_distance)
        dose_names = dose_rate_names(tuvx)

        total_irradiance = direct_irradiance .+ upward_irradiance .+ downward_irradiance
        for (name, weights) in dose_weights
            per_bin = hc ./ (lambda_bins .* 1.0e-13)
            expected = vec(
                sum(total_irradiance .* earth_sun_distance .* etfl .* per_bin' .* weights', dims = 2),
            )
            actual = result2.dose_rates[dose_names[name]+1, :]
            @test actual ≈ expected rtol = 1.0e-8
        end
    end

    @testset "update! rejects a mismatched shape" begin
        config_path = joinpath(
            @__DIR__,
            "..",
            "..",
            "configs",
            "tuvx",
            "host_radiation_field",
            "config.json",
        )
        tuvx = TUVX(
            grid_map = GridMap(),
            profile_map = ProfileMap(),
            radiator_map = RadiatorMap(),
            config_path = config_path,
        )
        updater = get_radiation_field_updater(tuvx)

        wrong_shape = zeros(3, 3)
        @test_throws ErrorException update!(updater, wrong_shape, wrong_shape, wrong_shape)
    end
end
