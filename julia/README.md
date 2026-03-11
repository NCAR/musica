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

Julia 1.10 or 1.11 is required. Use [juliaup](https://github.com/JuliaLang/juliaup) to manage Julia versions.

There are two development workflows depending on whether you are working on the Julia API or the underlying C++.

---

#### Workflow A: Julia development (using pre-built JLL)

This is the fastest path if you are only working on the Julia layer. It uses
a locally-built `Musica_jll` artifact instead of a CMake build.

**Prerequisites:** A locally-built `Musica_jll` at `~/.julia/dev/Musica_jll`
(see [Building Musica_jll](#building-musica_jll) below).

```bash
cd julia
julia +1.11 --project=. -e 'using Pkg; Pkg.develop(path="~/.julia/dev/Musica_jll"); Pkg.instantiate()'
```

Run tests:

```bash
julia +1.11 --project=. -e 'using Pkg; Pkg.test()'
```

---

#### Workflow B: C++ development (CMake build + library override)

Use this when iterating on the C++ bindings in `julia/bindings/musica_julia.cpp`.

**Prerequisites:**

- CMake 3.24 or later
- A C++ compiler with C++20 support
- CxxWrap built for Julia 1.11 (CMake will find this via `Julia_PREFIX`)

1. Build with Julia support enabled:

```bash
cmake -S . -B build \
      -D MUSICA_ENABLE_JULIA=ON \
      -D MUSICA_ENABLE_MICM=ON \
      -D MUSICA_ENABLE_TUVX=OFF \
      -D MUSICA_ENABLE_CARMA=OFF \
      -D MUSICA_ENABLE_TESTS=OFF \
      -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

This places the compiled library in `julia/deps/lib/libmusica_julia.dylib` (macOS)
or `julia/deps/lib/libmusica_julia.so` (Linux).

2. Set up the Julia package:

```bash
cd julia
julia +1.11 --project=. -e 'using Pkg; Pkg.develop(path="~/.julia/dev/Musica_jll"); Pkg.instantiate()'
```

3. Override the JLL library with your local build:

```bash
export MUSICA_JULIA_LIB=$(pwd)/deps/lib/libmusica_julia.dylib  # macOS
export MUSICA_JULIA_LIB=$(pwd)/deps/lib/libmusica_julia.so     # Linux
julia +1.11 --project=. -e 'using Pkg; Pkg.test()'
```

---

#### Building Musica_jll

`Musica_jll` is built from the `build_tarballs.jl` script in the
[Yggdrasil](https://github.com/JuliaPackaging/Yggdrasil) repository.
To build it locally:

```bash
# BinaryBuilder requires Julia 1.7
julia --project=@BinaryBuilder build_tarballs.jl \
    "aarch64-apple-darwin-julia_version+1.11" \
    --verbose --deploy=local
```

Replace `aarch64-apple-darwin` with your platform as needed. The `--deploy=local`
flag automatically installs the artifact to `~/.julia/dev/Musica_jll/`.

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
