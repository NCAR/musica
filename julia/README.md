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

> **Note:** The package is not yet registered in the Julia General registry. See [Development Installation](#development-installation) below for building from source.

### Development Installation

#### Prerequisites

- Julia 1.10 or 1.11 (`juliaup` is recommended for managing Julia versions)
- CMake 3.24 or later
- A C++ compiler with C++20 support

#### Building from Source

1. Clone the MUSICA repository:
```bash
git clone https://github.com/NCAR/musica.git
cd musica
```

2. Build with Julia support enabled:
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

3. Install the Julia package:
```bash
cd julia
julia --project=. -e 'using Pkg; Pkg.instantiate()'
```

4. Test the installation:
```bash
julia --project=. test/runtests.jl
```

## Quick Start

```julia
using Musica

# Get the MUSICA version
version = Musica.get_version()
println("MUSICA version: ", version)
```

## Configuration

### Library Path Override

For testing a build of musica in a downstream package, 
you can override the library path using an environment variable:

```bash
export MUSICA_JULIA_LIB=/path/to/libmusica_julia.so
julia -e 'using Musica; println(Musica.get_version())'
```

## Testing

Run the test suite:

```bash
cd julia
julia --project=. test/runtests.jl
```

Or from within Julia:

```julia
using Pkg
Pkg.test("Musica")
```

## Documentation

For more information about MUSICA, visit:
- [MUSICA Documentation](https://ncar.github.io/musica/)
- [MUSICA Wiki](https://wiki.ucar.edu/display/MUSICA/MUSICA+Home)

## Contributing

See the [distribution guide](DISTRIBUTION.md) for information on the package release process.

## License

MUSICA is licensed under the Apache License 2.0. See the [LICENSE](../LICENSE) file for details.
