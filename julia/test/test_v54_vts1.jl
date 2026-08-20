# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

# Tests for the Musica.V54 and Musica.VTS1 convenience configurations.
#
# TUV-x is an optional Fortran component. The caller includes this file only
# when `Musica.tuvx_available()` is true.

using Test
using Musica
using Musica.V54
using Musica.VTS1

@testset "TUV-x V54" begin
    @testset "Grids" begin
        heights = V54.height_grid()
        @test num_sections(heights) == 120
        @test edges(heights)[1] == 0.0
        @test edges(heights)[end] == 120.0

        wavelengths = V54.wavelength_grid()
        @test num_sections(wavelengths) == 156
        @test edges(wavelengths)[1] == 120.0
        @test edges(wavelengths)[end] == 735.0
    end

    @testset "Profile" begin
        heights = V54.height_grid()

        o3 = V54.profile("O3", heights)
        @test get_name(o3) == "O3"
        @test get_units(o3) == "molecule cm-3"
        @test num_sections(o3) == num_sections(heights)
        @test all(collect(midpoint_values(o3)) .>= 0.0)

        surface_albedo = V54.profile("surface albedo", V54.wavelength_grid())
        @test get_name(surface_albedo) == "surface albedo"

        @test_throws ErrorException V54.profile("not a profile", heights)

        # A grid that doesn't match the data file's native grid exercises the
        # interpolation path.
        coarse = Grid(name = "height", units = "km", num_sections = 3)
        set_edges!(coarse, [0.0, 40.0, 80.0, 120.0])
        set_midpoints!(coarse, 0.5 .* (edges(coarse)[1:(end - 1)] .+ edges(coarse)[2:end]))
        interpolated = V54.profile("O3", coarse)
        @test num_sections(interpolated) == 3
        # O3 concentration decreases with height over this range.
        values = collect(midpoint_values(interpolated))
        @test values[1] > values[2] > values[3] >= 0.0
    end

    @testset "Radiator" begin
        heights = V54.height_grid()
        wavelengths = V54.wavelength_grid()
        aerosol = V54.radiator("aerosol", heights, wavelengths)
        @test get_name(aerosol) == "aerosol"
        @test num_height_sections(aerosol) == 120
        @test num_wavelength_sections(aerosol) == 156

        @test_throws ErrorException V54.radiator("not a radiator", heights, wavelengths)

        # Cross-check a handful of rows directly against the data file, the
        # same way the radiator was built, to guard against a transposed
        # axis or an off-by-one in the nearest-index matching.
        height_mid = collect(midpoints(heights))
        wavelength_mid = collect(midpoints(wavelengths))
        checked = 0
        for raw in eachline(joinpath(V54._CONFIG_ROOT, "data", V54.radiator_data_files["aerosol"]))
            stripped = strip(raw)
            (isempty(stripped) || startswith(stripped, "#")) && continue
            parts = split(stripped)
            length(parts) < 5 && continue
            checked += 1
            checked > 25 && break
            file_height = parse(Float64, parts[1])
            file_wavelength = parse(Float64, parts[2])
            h_idx = argmin(abs.(height_mid .- file_height))
            w_idx = argmin(abs.(wavelength_mid .- file_wavelength))
            @test optical_depths(aerosol)[h_idx, w_idx] == parse(Float64, parts[3])
            @test single_scattering_albedos(aerosol)[h_idx, w_idx] == parse(Float64, parts[4])
            @test asymmetry_factors(aerosol)[h_idx, w_idx] == parse(Float64, parts[5])
        end
        @test checked > 0
    end

    @testset "Full run" begin
        tuvx = cd(dirname(V54.config_file_path())) do
            V54.get_tuvx_calculator()
        end

        # This configuration defines photolysis and dose rates but no
        # heating rates.
        @test photolysis_rate_constant_count(tuvx) > 0
        @test heating_rate_count(tuvx) == 0
        @test dose_rate_count(tuvx) > 0
        @test num_height_midpoints(tuvx) == 120
        @test num_wavelength_midpoints(tuvx) == 156

        result = run!(tuvx, 0.3, 1.0)
        n_reactions = photolysis_rate_constant_count(tuvx)
        n_dose = dose_rate_count(tuvx)
        @test size(result.photolysis_rate_constants) == (n_reactions, 121)
        @test size(result.dose_rates) == (n_dose, 121)
        @test size(result.actinic_flux) == (156, 121, 3)
        @test all(result.photolysis_rate_constants .>= 0.0)
        @test any(result.photolysis_rate_constants .> 0.0)

        # The grid map TUV-x reports back matches what was supplied.
        grids = get_grid_map(tuvx)
        @test collect(edges(grids["height", "km"])) == collect(edges(V54.height_grid()))
        @test collect(edges(grids["wavelength", "nm"])) == collect(edges(V54.wavelength_grid()))

        # Doubling O2/O3/air concentrations decreases surface-level
        # photolysis rates (self-shading). This does not hold at every one
        # of the 121 layers: near the top of the atmosphere, the
        # delta-Eddington solver's multiple-scattering response to more
        # absorber is genuinely non-monotonic with height for some
        # reactions, confirmed by inspecting the per-layer values directly
        # rather than assumed.
        names = photolysis_rate_names(tuvx)
        profiles = get_profile_map(tuvx)
        height_grid = grids["height", "km"]
        for (name, units) in (("O2", "molecule cm-3"), ("O3", "molecule cm-3"), ("air", "molecule cm-3"))
            p = profiles[name, units]
            set_midpoint_values!(p, 2.0 .* midpoint_values(p))
            calculate_layer_densities!(p, height_grid)
            calculate_exo_layer_density!(p, 8.5)
        end
        doubled = run!(tuvx, 0.3, 1.0).photolysis_rate_constants

        for reaction in ("O2+hv->O+O", "O3+hv->O2+O(1D)", "O3+hv->O2+O(3P)")
            i = names[reaction] + 1
            @test doubled[i, 1] <= result.photolysis_rate_constants[i, 1]
        end
    end
end

@testset "TUV-x VTS1" begin
    @testset "Grids" begin
        # VTS1 reuses V54's height grid.
        @test VTS1.height_grid === V54.height_grid

        wavelengths = VTS1.wavelength_grid()
        @test num_sections(wavelengths) == 102
        @test edges(wavelengths)[1] == 120.0
        @test edges(wavelengths)[end] == 750.0
    end

    @testset "Profile and radiator" begin
        heights = VTS1.height_grid()
        wavelengths = VTS1.wavelength_grid()

        o3 = VTS1.profile("O3", heights)
        @test get_name(o3) == "O3"
        @test num_sections(o3) == num_sections(heights)

        # VTS1 uses its own solar profiles, distinct from V54's.
        @test VTS1.profile_data_files["surface albedo"] != V54.profile_data_files["surface albedo"]

        aerosol = VTS1.radiator("aerosol", heights, wavelengths)
        @test get_name(aerosol) == "aerosol"
        @test num_wavelength_sections(aerosol) == 102
    end

    @testset "Full run" begin
        tuvx = cd(dirname(VTS1.config_file_path())) do
            VTS1.get_tuvx_calculator()
        end

        # This configuration defines photolysis and heating rates but no
        # dose rates.
        @test photolysis_rate_constant_count(tuvx) > 0
        @test heating_rate_count(tuvx) > 0
        @test dose_rate_count(tuvx) == 0
        @test num_wavelength_midpoints(tuvx) == 102

        result = run!(tuvx, 0.3, 1.0)
        @test all(result.photolysis_rate_constants .>= 0.0)
        @test any(result.photolysis_rate_constants .> 0.0)
        @test all(result.heating_rates .>= 0.0)
    end
end
