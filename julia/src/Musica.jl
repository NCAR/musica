module Musica

using CxxWrap
using Musica_jll

function __init__()
    # Allow overriding the JLL library for raw CMake development builds
    lib = get(ENV, "MUSICA_JULIA_LIB", nothing)
    if lib !== nothing && isfile(lib)
        @wrapmodule(() -> lib)
    else
        @wrapmodule(() -> Musica_jll.libmusica_julia)
    end
    @initcxx
end

export get_version

end
