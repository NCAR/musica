module Musica_jll

using Libdl

export libmusica_julia

# Path from this file: Musica_jll/src/ -> Musica_jll/ -> julia/ -> repo root -> build/lib/
# Assumes the cmake build directory is named "build" (documented in README).
const libmusica_julia =
    joinpath(@__DIR__, "..", "..", "..", "build", "lib", "libmusica_julia.$(Libdl.dlext)")

end
