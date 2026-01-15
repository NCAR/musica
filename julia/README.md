# Musica.jl

Julia wrapper for MUSICA (Multiscale Interface for Chemistry and Aerosols).

## Overview

Musica.jl provides Julia bindings to the MUSICA atmospheric chemistry library, enabling you to perform chemical kinetics simulations from Julia code. This wrapper uses [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl) to interface with the underlying C++ library.

## Installation

### Prerequisites

- Julia 1.6 or later
- CMake 3.24 or later
- A C++ compiler with C++20 support
- NetCDF library (for full functionality)

### Building from Source

1. Clone the MUSICA repository:
```bash
git clone https://github.com/NCAR/musica.git
cd musica
```

2. Build with Julia support enabled:
```bash
cmake -S . -B build \
      -D MUSICA_ENABLE_JULIA=ON \
      -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

3. Install the Julia package:
```bash
cd julia
julia --project=. -e 'using Pkg; Pkg.instantiate()'
```

## Quick Start

```julia
using Musica

# Get the MUSICA version
version = Musica.get_version()
println("MUSICA version: ", version)
```

## Features

- Access to MUSICA version information
- (More features will be added as the wrapper develops)

## Testing

Run the test suite:

```bash
cd julia
julia --project=. test/runtests.jl
```

## Documentation

For more information about MUSICA, visit:
- [MUSICA Documentation](https://ncar.github.io/musica/)
- [MUSICA Wiki](https://wiki.ucar.edu/display/MUSICA/MUSICA+Home)

## Support

For questions or issues with the Julia wrapper, please file an issue on the [MUSICA GitHub repository](https://github.com/NCAR/musica/issues).

## License

MUSICA is licensed under the Apache License 2.0. See the [LICENSE](../LICENSE) file for details.
