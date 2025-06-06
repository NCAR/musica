#! /bin/bash

set -e
set -x

# Update the mirror list to use vault.centos.org
sed -i s/mirror.centos.org/vault.centos.org/g /etc/yum.repos.d/*.repo
sed -i s/^#.*baseurl=http/baseurl=http/g /etc/yum.repos.d/*.repo
sed -i s/^mirrorlist=http/#mirrorlist=http/g /etc/yum.repos.d/*.repo

sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-*
sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*

yum install -y zip tree wget

# Use CIBW_ARCHS or CIBW_ARCH if set, else fallback to uname -m
if [ -n "$CIBW_ARCHS" ]; then
  target_arch="$CIBW_ARCHS"
elif [ -n "$CIBW_ARCH" ]; then
  target_arch="$CIBW_ARCH"
else
  target_arch="$(uname -m)"
fi

echo "Detected target_arch: $target_arch"

if [ "$target_arch" = "x86_64" ]; then
  # Install CUDA 12.2 for x86_64:
  yum-config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
  # error mirrorlist.centos.org doesn't exists anymore.
  sed -i s/mirror.centos.org/vault.centos.org/g /etc/yum.repos.d/*.repo
  sed -i s/^#.*baseurl=http/baseurl=http/g /etc/yum.repos.d/*.repo
  sed -i s/^mirrorlist=http/#mirrorlist=http/g /etc/yum.repos.d/*.repo
  yum install --setopt=obsoletes=0 -y \
      cuda-nvcc-12-2 \
      cuda-cudart-devel-12-2 \
      libcurand-devel-12-2 \
      libcublas-devel-12-2 
  ln -s cuda-12.2 /usr/local/cuda

  # list the installed CUDA packages
  tree -L 4 /usr/local/cuda-12.2
fi
