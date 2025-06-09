#!/bin/bash

# exit on error
set -e
# print commands as they are executed
set -x

CIBW_ARCHS=x86_64 \
CIBW_PLATFORM=linux \
CIBW_ENVIRONMENT="BUILD_GPU=1 CUDA_PATH=/usr/local/cuda PATH=/usr/local/cuda/bin:\$PATH LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH" \
cibuildwheel --output-dir wheelhouse > log.txt 2>&1

scp wheelhouse/musica-0.11.1.3-cp312-cp312-manylinux_2_17_x86_64.manylinux2014_x86_64.whl kshores@casper.hpc.ucar.edu:~/temp
