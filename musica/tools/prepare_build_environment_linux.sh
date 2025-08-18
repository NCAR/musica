#!/bin/bash
set -e
set -x

target_arch="$(uname -m)"
echo "Detected target_arch: $target_arch"

# Detect the Linux distribution and set package manager
# Check if we're in a CentOS 7 environment (old manylinux2014) or AlmaLinux 8 (manylinux_2_28)
if [ -f /etc/centos-release ] && grep -q "CentOS Linux release 7" /etc/centos-release; then
  # CentOS 7 (manylinux2014) - uses yum
  PKG_MGR="yum"
  
  # CentOS 7 is EOL, so we need to use vault.centos.org for old builds
  # Replace the repo files to point to vault.centos.org
  sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-*.repo
  sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*.repo
else
  # AlmaLinux 8 or newer (manylinux_2_28) - uses dnf
  PKG_MGR="dnf"
fi

echo "Using package manager: $PKG_MGR"

$PKG_MGR -y update

# Install NetCDF if we're on a modern system (manylinux_2_28)
if [ "$PKG_MGR" = "dnf" ]; then
  # For manylinux_2_28 (AlmaLinux 8), epel-release is required to get netcdf
  $PKG_MGR install -y epel-release
  $PKG_MGR install -y netcdf-devel netcdf-fortran-devel
fi

$PKG_MGR install -y tree wget zip lapack-devel

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