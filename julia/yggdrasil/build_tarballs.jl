# Build script for Musica_jll
# Note: This file should be placed at M/Musica/build_tarballs.jl in Yggdrasil
#
# To test locally:
#   julia --project=@BinaryBuilder build_tarballs.jl --verbose --debug
#
# To build for a specific platform:
#   julia --project=@BinaryBuilder build_tarballs.jl x86_64-linux-gnu-cxx11

using BinaryBuilder, Pkg

name = "Musica"
version = v"0.14.4"

# Collection of sources required to build Musica
sources = [
    GitSource("https://github.com/NCAR/musica.git",
              "8af5e002e4e0b5e5e44e0eb73e3e2ed8de7fca77"),
]

# Bash recipe for building across all platforms
script = raw"""
cd $WORKSPACE/srcdir/musica

# Configure MUSICA with Julia wrapper enabled
cmake -B build -G Ninja \
    -DCMAKE_INSTALL_PREFIX=${prefix} \
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TARGET_TOOLCHAIN} \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${prefix}" \
    -DMUSICA_BUILD_C_CXX_INTERFACE=ON \
    -DMUSICA_ENABLE_JULIA=ON \
    -DMUSICA_ENABLE_MICM=ON \
    -DMUSICA_ENABLE_TUVX=OFF \
    -DMUSICA_ENABLE_CARMA=OFF \
    -DMUSICA_ENABLE_TESTS=OFF \
    -DMUSICA_ENABLE_INSTALL=ON \

cmake --build build --parallel ${nproc}
cmake --install build
"""

# These are the platforms the libcxxwrap_julia_jll is built on.
include("../../L/libjulia/common.jl")
platforms = libjulia_platforms(julia_version)
platforms = expand_cxxstring_abis(platforms)

# The products that we will ensure are always built
products = [
    LibraryProduct("libmusica_julia", :libmusica_julia),
    LibraryProduct("libmusica", :libmusica),
    LibraryProduct("libmechanism_configuration", :libmechanism_configuration),
]

# Dependencies that must be installed before this package can be built
dependencies = [
    BuildDependency("libjulia_jll"),
    Dependency("libcxxwrap_julia_jll"; compat="~0.13"),
]

# Build the tarballs
build_tarballs(ARGS, name, version, sources, script, platforms, products, dependencies;
               julia_compat="1.10",
               preferred_gcc_version=v"10")
