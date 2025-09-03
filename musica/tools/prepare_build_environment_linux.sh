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

dnf install -y tree wget zip lapack-devel

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