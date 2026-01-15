module Musica

using CxxWrap

const lib_ext = Sys.iswindows() ? "dll" : (Sys.isapple() ? "dylib" : "so")
const libmusica_julia = joinpath(@__DIR__, "..", "deps", "lib",
                                 "libmusica_julia.$lib_ext")

function __init__()
    if !isfile(libmusica_julia)
        error("MUSICA Julia library not found at: $libmusica_julia")
    end
    @wrapmodule(() -> libmusica_julia)
    @initcxx
end

export get_version

end
