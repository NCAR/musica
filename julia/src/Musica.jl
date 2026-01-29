module Musica

using CxxWrap
using Libdl
using Preferences

# Library loading strategy:
# 1. If MUSICA_JULIA_LIB environment variable is set, use that path
# 2. If local deps/lib exists (development mode), use local build
# 3. Otherwise, try to use Musica_jll (once it exists after Yggdrasil merge)

const lib_ext = Libdl.dlext
const lib_prefix = Sys.iswindows() ? "" : "lib"

# Local library path (for development builds)
const local_lib_path = joinpath(@__DIR__, "..", "deps", "lib", "$(lib_prefix)musica_julia.$lib_ext")

"""
    get_library_path()

Determine the path to the MUSICA Julia library.

Returns the library path based on the following priority:
1. `MUSICA_JULIA_LIB` environment variable (if set)
2. Local development build in `deps/lib/`
3. Musica_jll package (when available)
"""
function get_library_path()
    # Priority 1: Environment variable override
    env_lib = get(ENV, "MUSICA_JULIA_LIB", nothing)
    if env_lib !== nothing && isfile(env_lib)
        return env_lib
    end

    # Priority 2: Local development build
    if isfile(local_lib_path)
        return local_lib_path
    end

    # Priority 3: JLL package (uncomment once Musica_jll exists)
    # try
    #     using Musica_jll
    #     return Musica_jll.libmusica_julia
    # catch
    # end

    error("""
        MUSICA Julia library not found.

        For development builds:
          Build MUSICA with -DMUSICA_ENABLE_JULIA=ON
          Library expected at: $local_lib_path

        For production use:
          Install the Musica package from the Julia General registry
          (requires Musica_jll to be available in Yggdrasil)

        To override library location:
          Set MUSICA_JULIA_LIB environment variable to the library path
    """)
end

# Store the library path for use at initialization
const libmusica_julia = Ref{String}()

function __init__()
    libmusica_julia[] = get_library_path()
    @wrapmodule(() -> libmusica_julia[])
    @initcxx
end

export get_version

end
