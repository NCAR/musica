#! /bin/bash

set -e
set -x

# Install CUDA 12.2:
yum-config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
# error mirrorlist.centos.org doesn't exists anymore.
sed -i s/mirror.centos.org/vault.centos.org/g /etc/yum.repos.d/*.repo
sed -i s/^#.*baseurl=http/baseurl=http/g /etc/yum.repos.d/*.repo
sed -i s/^mirrorlist=http/#mirrorlist=http/g /etc/yum.repos.d/*.repo
yum install --setopt=obsoletes=0 -y \
    cuda-nvcc-12-2-12.2.140-1 \
    cuda-cudart-devel-12-2-12.2.140-1 \
    libcurand-devel-12-2-10.3.3.141-1 \
    libcublas-devel-12-2-12.2.5.6-1 \
    libnccl-devel-2.19.3-1+cuda12.2
ln -s cuda-12.2 /usr/local/cuda

# Add CUDA to PATH and LD_LIBRARY_PATH
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

# Verify CUDA installation
if ! command -v nvcc &> /dev/null; then
    echo "CUDA compiler (nvcc) not found in PATH"
    exit 1
fi

nvcc --version

ls -l /usr/local/cuda
ls -l /usr/local/cuda/bin