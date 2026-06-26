#!/bin/bash
set -e
set -x

brew install netcdf netcdf-fortran lapack ffmpeg

################################################################################
# Prebuild musica library
# This runs once before all Python version builds, so we only compile the
# C++/Fortran code once instead of once per Python version.
################################################################################

echo "=== Prebuilding musica library ==="

MUSICA_PREBUILT_DIR="/tmp/musica-prebuilt"
MUSICA_BUILD_DIR="/tmp/musica-build"

# Get the source directory (script is in python/tools/, source is two levels up)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MUSICA_SOURCE_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

mkdir -p "$MUSICA_BUILD_DIR"
cd "$MUSICA_BUILD_DIR"

# Configure musica build
cmake_args=(
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX="$MUSICA_PREBUILT_DIR"
  -DMUSICA_ENABLE_PYTHON_LIBRARY=OFF
  -DMUSICA_BUILD_FORTRAN_INTERFACE=OFF
  -DMUSICA_ENABLE_TESTS=OFF
  -DMUSICA_ENABLE_INSTALL=ON
  -DMUSICA_BUILD_SHARED_LIBS=ON
  -DMUSICA_SET_MICM_DEFAULT_VECTOR_SIZE=4
  -DMUSICA_GPU_TYPE=None
  -DCMAKE_C_COMPILER=clang
  -DCMAKE_CXX_COMPILER=clang++
)

cmake "${cmake_args[@]}" "$MUSICA_SOURCE_DIR"

# Build and install
cmake --build . --parallel "$(sysctl -n hw.ncpu)"
cmake --install .

echo "=== Musica prebuilt to $MUSICA_PREBUILT_DIR ==="
ls -la "$MUSICA_PREBUILT_DIR"
