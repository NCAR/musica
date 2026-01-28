module Musica

using CxxWrap
using Libdl

const lib_ext = Libdl.dlext
const lib_prefix = Sys.iswindows() ? "" : "lib"
const libmusica_julia = joinpath(@__DIR__, "..", "deps", "lib",
                                 "$(lib_prefix)musica_julia.$lib_ext")

function __init__()
    if !isfile(libmusica_julia)
        error("MUSICA Julia library not found at: $libmusica_julia")
    end
    @wrapmodule(() -> libmusica_julia)
    @initcxx
end

export get_version

end
