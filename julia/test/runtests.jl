using Test
using Musica

@testset "Musica.jl Tests" begin
    @testset "Version" begin
        version = Musica.get_version()
        @test version isa String
        @test !isempty(version)
        @test occursin("0.14.4", version)
        println("MUSICA version: ", version)
    end
end
