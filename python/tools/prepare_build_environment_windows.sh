#! /bin/bash

set -e
set -x

CUDA_ROOT="C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2"
curl --netrc-optional -L -nv -o cuda.exe https://developer.download.nvidia.com/compute/cuda/12.2.2/local_installers/cuda_12.2.2_537.13_windows.exe
./cuda.exe -s nvcc_12.2 cudart_12.2 cublas_dev_12.2 curand_dev_12.2
rm cuda.exe

# Get the nvcc version
nvcc --version