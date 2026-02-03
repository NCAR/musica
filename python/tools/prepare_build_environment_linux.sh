#!/bin/bash
set -e
set -x

target_arch="$(uname -m)"
echo "Detected target_arch: $target_arch"

dnf -y update

# For 64 bit systems can enable our fortran components, but we require netcdf
if [[ "$target_arch" == "x86_64" || "$target_arch" == "aarch64" ]]; then
  dnf install -y epel-release
  dnf install -y netcdf-devel netcdf-fortran-devel
fi

dnf install -y tree wget zip lapack-devel cmake

# 64 bit intel and amd systems support building cuda
if [ "$target_arch" = "x86_64" ]; then
  # Install CUDA 12.8 for x86_64 on AlmaLinux 8 (manylinux_2_28) - supports GCC 14
  dnf config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
  dnf install --setopt=obsoletes=0 -y \
      cuda-nvcc-12-8 \
      cuda-cudart-devel-12-8 \
      libcurand-devel-12-8 \
      libcublas-devel-12-8
  ln -sf cuda-12.8 /usr/local/cuda

  # Verify CUDA installation
  echo "=== CUDA Installation Verification ==="
  /usr/local/cuda/bin/nvcc --version
fi

################################################################################
# Prebuild musica library
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

# Configure musica build
cmake_args=(
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX="$MUSICA_PREBUILT_DIR"
  -DMUSICA_ENABLE_PYTHON_LIBRARY=OFF
  -DMUSICA_BUILD_FORTRAN_INTERFACE=OFF
  -DMUSICA_ENABLE_TESTS=OFF
  -DMUSICA_ENABLE_INSTALL=ON
  -DMUSICA_ENABLE_PIC=ON
  -DMUSICA_SET_MICM_DEFAULT_VECTOR_SIZE=4
)

# Platform-specific options
if [[ "$target_arch" == "x86_64" ]]; then
  cmake_args+=(
    -DMUSICA_GPU_TYPE=all_major
    -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc
  )
else
  cmake_args+=(-DMUSICA_GPU_TYPE=None)
fi

# 32-bit has limited features
if [[ "$target_arch" == "i686" ]]; then
  cmake_args+=(
    -DMUSICA_ENABLE_TUVX=OFF
    -DMUSICA_ENABLE_CARMA=OFF
  )
fi

cmake "${cmake_args[@]}" "$MUSICA_SOURCE_DIR"

# Build and install
cmake --build . --parallel "$(nproc)"
cmake --install .

echo "=== Musica prebuilt to $MUSICA_PREBUILT_DIR ==="
ls -la "$MUSICA_PREBUILT_DIR"