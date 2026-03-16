using Test
using Musica

@testset "Musica.jl Tests" begin
    @testset "Version" begin
        version = Musica.get_version()
        @test version isa AbstractString
        @test !isempty(version)
        # Test version format (semantic versioning: X.Y.Z or X.Y.Z.W)
        @test occursin(r"^\d+\.\d+\.\d+", version)
        println("MUSICA version: ", version)
    end
end
