#!/bin/bash
set -e
set -x

brew install gcc netcdf netcdf-fortran lapack ffmpeg

# Force the Fortran toolchain to Homebrew's gcc. TUV-x/CARMA (Fortran) must link
# the SAME libgfortran as Homebrew's netcdf-fortran, otherwise the wheel ends up
# with two different libgfortran.5.dylib files and delocate refuses to repair it.
GCC_PREFIX="$(brew --prefix gcc)"
FC="$(ls "$GCC_PREFIX"/bin/gfortran-[0-9]* 2>/dev/null | sort -V | tail -n1)"
if [[ -z "$FC" ]]; then
  FC="$(brew --prefix)/bin/gfortran"
fi
export FC
echo "Using Fortran compiler: $FC"
"$FC" --version

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
  -DMUSICA_USE_FMT=OFF
  -DMUSICA_ENABLE_MIAM=ON
  -DMUSICA_ENABLE_MIEM=ON
  -DMUSICA_SET_MICM_DEFAULT_VECTOR_SIZE=4
  -DMUSICA_GPU_TYPE=None
  -DCMAKE_C_COMPILER=clang
  -DCMAKE_CXX_COMPILER=clang++
  -DCMAKE_Fortran_COMPILER="$FC"
)

cmake "${cmake_args[@]}" "$MUSICA_SOURCE_DIR"

# Build and install
cmake --build . --parallel "$(sysctl -n hw.ncpu)"
cmake --install .

echo "=== Musica prebuilt to $MUSICA_PREBUILT_DIR ==="
ls -la "$MUSICA_PREBUILT_DIR"
