# Musica.jl

Julia wrapper for MUSICA (Multiscale Interface for Chemistry and Aerosols).

## Overview

Musica.jl provides Julia bindings to the MUSICA atmospheric chemistry library, enabling you to perform chemical kinetics simulations from Julia code. This wrapper uses [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl) to interface with the underlying C++ library.

## Installation

### From Julia Registry (Recommended)

Once published, you will be able to install Musica.jl directly:

```julia
using Pkg
Pkg.add("Musica")
```

> **Note:** The package is not yet registered in the Julia General registry. See [Development Installation](#development-installation) below.

### Development Installation

Julia 1.10+ is required. Use [juliaup](https://github.com/JuliaLang/juliaup) to manage Julia versions.

There are three documented development workflows. Which to choose depends on
whether you are working on the Julia API or the underlying C++.

---

#### Workflow A: Local BinaryBuilder deploy (Musica_jll)

This builds a proper `Musica_jll` package for your machine and installs it into
`~/.julia/dev/`, which is the closest approximation to the production JLL
workflow without publishing to Yggdrasil. This is likely the easiest when you don't 
need to make many changes to the C++ API since building with Yggdrasil will require you to
update the commit hash used.

Clone (Yggdrasil)[https://github.com/JuliaPackaging/Yggdrasil] and build the musica tarball
for your platform.

**in Yggdrasil**
```bash
# From the directory containing build_tarballs.jl:
cd M/Musica
julia build_tarballs.jl "aarch64-apple-darwin-julia_version+1.11" --deploy=local --verbose
# Replace aarch64-apple-darwin with your platform (julia -e 'print(Base.BinaryPlatforms.HostPlatform())')
```

This will place a local `Musica_jll` pacakge into your local julia registry.

Then, you can use this to instantiate `Musica.jll` and iterate on the the julia API. If you need make changes in musica's
C++ API, you'll need to update the commit has in `build_tarballs.jl` and rebuild `Musica_jll`.

**in musica**
```bash
cd julia
julia +1.11 --project=. -e '
  using Pkg
  Pkg.develop(path=expanduser("~/.julia/dev/Musica_jll"))
  Pkg.instantiate()
'
julia +1.11 --project=. test/runtests.jl
```
---

#### Workflow B: Build from source (CMake + local stub JLL)

Use this when iterating heavily on the C++ bindings and not much on the Julia API. Run all commands
from the **repo root** unless noted.

**Prerequisites:** CMake 3.24+, a C++20 compiler.

```bash
# 1. Get the CxxWrap prefix for CMake
JLCXX_PREFIX=$(julia +1.11 -e 'import Pkg; Pkg.add("CxxWrap"); using CxxWrap; print(CxxWrap.prefix_path())')

# 2. Build the MUSICA library (output always lands in julia/deps/lib/)
cmake -S . -B build \
      -D CMAKE_PREFIX_PATH=$JLCXX_PREFIX \
      -D CMAKE_BUILD_TYPE=Release \
      -D MUSICA_ENABLE_JULIA=ON \
      -D MUSICA_ENABLE_MICM=ON \
      -D MUSICA_ENABLE_TUVX=OFF \
      -D MUSICA_ENABLE_CARMA=OFF \
      -D MUSICA_ENABLE_TESTS=OFF \
      -D MUSICA_BUILD_SHARED_LIBS=ON \
      -D MUSICA_ENABLE_INSTALL=ON
cmake --build build

# 3. Instantiate and test
cd julia
julia +1.11 --project=. -e 'using Pkg; Pkg.develop(PackageSpec(path="Musica_jll")); Pkg.instantiate()'
julia +1.11 --project=. test/runtests.jl
```

The repo includes a stub `Musica_jll` package at `julia/Musica_jll/` that points directly
to `build/lib/libmusica_julia.<ext>`. The build directory must be named `build` (as shown
above). After rebuilding the C++ library, re-run `Pkg.precompile()` to invalidate the
cache and pick up the new binary.

---

#### Workflow C: Using the registered Musica_jll

This is most useful if you're only updating the julia code and not the bindings. In that case
you can use the published `Musica_jll` package directly.

```bash
cd julia
julia +1.11 --project=. -e 'using Pkg; Pkg.instantiate()'
julia +1.11 --project=. test/runtests.jl
```

## Quick Start

```julia
using Musica

version = Musica.get_version()
println("MUSICA version: ", version)
```

## Documentation

- [MUSICA Documentation](https://ncar.github.io/musica/)
- [MUSICA Wiki](https://wiki.ucar.edu/display/MUSICA/MUSICA+Home)

## License

MUSICA is licensed under the Apache License 2.0. See the [LICENSE](../LICENSE) file for details.
