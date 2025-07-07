#!/bin/bash
set -e
set -x

target_arch="$(uname -m)"
echo "Detected target_arch: $target_arch"

# Set package manager based on architecture
# x86_64 and aarch64 use manylinux_2_28 (AlmaLinux 8) with dnf
# i686 uses manylinux2014 (CentOS 7) with yum
if [ "$target_arch" = "i686" ]; then
  PKG_MGR="yum"
else
  PKG_MGR="dnf"
fi

echo "Using package manager: $PKG_MGR"

# Install basic dependencies
$PKG_MGR -y update

# Handle EPEL repository setup based on architecture
if [ "$target_arch" = "x86_64" ]; then
  # For manylinux_2_28 (AlmaLinux 8), epel-release is required to get netcdf, for some reason
  $PKG_MGR install -y epel-release
  $PKG_MGR search netcdf
  $PKG_MGR install -y netcdf-devel netcdf-fortran-devel
fi

$PKG_MGR install -y tree wget zip 

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
  
  # list the installed CUDA packages
  # tree -L 4 /usr/local/cuda
fi