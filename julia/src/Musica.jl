module Musica

using CxxWrap

# Determine the library extension based on platform
const lib_ext = Sys.iswindows() ? "dll" : (Sys.isapple() ? "dylib" : "so")
const libmusica_julia = joinpath(@__DIR__, "..", "deps", "lib", "libmusica_julia.$lib_ext")

# Check if the library exists and provide helpful error message
function __check_library()
    if !isfile(libmusica_julia)
        error("""
        MUSICA Julia library not found at: $libmusica_julia
        
        Please build the library first with:
            cmake -S . -B build -D MUSICA_ENABLE_JULIA=ON
            cmake --build build
        
        See the README.md for more information.
        """)
    end
end

__check_library()

@wrapmodule(() -> libmusica_julia)

function __init__()
    @initcxx
end

"""
    get_version() -> String

Returns the version string of the MUSICA library.

# Examples
```julia
version = Musica.get_version()
println("MUSICA version: ", version)
```
"""
get_version

export get_version

end # module
