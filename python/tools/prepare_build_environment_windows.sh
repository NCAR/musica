#! /bin/bash

set -e
set -x

CUDA_ROOT="C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8"
# curl --netrc-optional -L -nv -o cuda.exe https://developer.download.nvidia.com/compute/cuda/12.2.2/local_installers/cuda_12.2.2_537.13_windows.exe
curl --netrc-optional -L -nv -o cuda.exe https://developer.download.nvidia.com/compute/cuda/12.8.1/local_installers/cuda_12.8.1_572.61_windows.exe
./cuda.exe -s nvcc_12.8 cudart_12.8 cublas_dev_12.8 curand_dev_12.8 
rm cuda.exe

export CUDA_PATH="/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8"
export PATH="$CUDA_PATH/bin:$PATH"

ls "c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8/extras/visual_studio_integration/MSBuildExtensions"
ls "c/Program Files (x86)/Microsoft Visual Studio/"

# choco install cuda 

ls "$CUDA_PATH"
ls "$CUDA_PATH/bin"
which nvcc.exe