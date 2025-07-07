#!/bin/bash
set -e
set -x

yum -y update

yum search netcdf

yum install -y tree wget zip netcdf-devel netcdf-fortran-devel

# Check pkg-config paths
echo "PKG_CONFIG_PATH is: $PKG_CONFIG_PATH"
pkg-config --variable pc_path pkg-config

# Try pkg-config
echo "=== Testing pkg-config ==="
pkg-config --libs --cflags netcdf-fortran || echo "netcdf-fortran pkg-config failed"
pkg-config --libs --cflags netcdf || echo "netcdf pkg-config failed"

target_arch="$(uname -m)"
echo "Detected target_arch: $target_arch"

if [ "$target_arch" = "x86_64" ]; then
  # Install CUDA 12.2 for x86_64:
  yum-config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
  yum install --setopt=obsoletes=0 -y \
      cuda-nvcc-12-2 \
      cuda-cudart-devel-12-2 \
      libcurand-devel-12-2 \
      libcublas-devel-12-2 
  ln -s cuda-12.2 /usr/local/cuda

  # list the installed CUDA packages
  # tree -L 4 /usr/local/cuda-12.2
fi