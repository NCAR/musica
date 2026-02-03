#!/bin/bash
set -e
set -x

################################################################################
# Prebuild musica library for Windows (runs in MSYS2 environment)
# This runs once before all Python version builds, so we only compile the
# C++/Fortran code once instead of once per Python version.
################################################################################

echo "=== Prebuilding musica library ==="

MUSICA_PREBUILT_DIR="/opt/musica-prebuilt"
MUSICA_BUILD_DIR="/tmp/musica-build"

# Get the source directory (script is in python/tools/, source is two levels up)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MUSICA_SOURCE_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

mkdir -p "$MUSICA_BUILD_DIR"
cd "$MUSICA_BUILD_DIR"

# Determine number of processors
if command -v nproc &> /dev/null; then
  NPROC=$(nproc)
else
  NPROC=4
fi

# Configure musica build
cmake_args=(
  -G Ninja
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX="$MUSICA_PREBUILT_DIR"
  -DMUSICA_ENABLE_PYTHON_LIBRARY=OFF
  -DMUSICA_BUILD_FORTRAN_INTERFACE=OFF
  -DMUSICA_ENABLE_TESTS=OFF
  -DMUSICA_ENABLE_INSTALL=ON
  -DMUSICA_ENABLE_PIC=ON
  -DMUSICA_SET_MICM_DEFAULT_VECTOR_SIZE=4
  -DMUSICA_GPU_TYPE=None
)

# Windows ARM64 uses CLANGARM64 (no CARMA)
target_arch="$(uname -m)"
if [[ "$target_arch" == "aarch64" ]]; then
  cmake_args+=(-DMUSICA_ENABLE_CARMA=OFF)
fi

cmake "${cmake_args[@]}" "$MUSICA_SOURCE_DIR"

# Build and install
cmake --build . --parallel "$NPROC"
cmake --install .

echo "=== Musica prebuilt to $MUSICA_PREBUILT_DIR ==="
ls -la "$MUSICA_PREBUILT_DIR"
