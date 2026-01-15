module Musica

using CxxWrap

# Load the C++ library
const libmusica_julia = joinpath(@__DIR__, "..", "deps", "lib", "libmusica_julia.so")

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
