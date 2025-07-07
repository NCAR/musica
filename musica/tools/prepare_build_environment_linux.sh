#!/bin/bash
set -e
set -x

# manylinux_2_28 uses AlmaLinux 8 - use dnf instead of yum
dnf -y update

dnf search netcdf

dnf install -y tree wget zip netcdf-devel netcdf-fortran-devel

target_arch="$(uname -m)"
echo "Detected target_arch: $target_arch"

if [ "$target_arch" = "x86_64" ]; then
  # Install CUDA 12.6 for x86_64 on AlmaLinux 8 (manylinux_2_28) - supports GCC 14
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
  
  # list the installed CUDA packages
  # tree -L 4 /usr/local/cuda
fi