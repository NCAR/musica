FROM fedora:latest AS base

RUN dnf -y update \
  && dnf -y install \
    cmake \
    curl \
    gcc \
    gcc-c++ \
    gcc-gfortran \
    gdb \
    git \
    hdf5-devel \
    lapack-devel \
    lcov \
    libcurl-devel \
    m4 \
    make \
    netcdf-fortran-devel \
    pip \
    python \
    python-devel \
    tree \
    valgrind \ 
    vim \
    zlib-devel \
  && dnf clean all

FROM base AS build

ARG MUSICA_GIT_TAG=main
ARG BUILD_TYPE=Release

# Copy the musica code
COPY . musica

# Build
RUN cd musica \
    && cmake -S . \
             -B build \
             -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
             -D MUSICA_GIT_TAG=${MUSICA_GIT_TAG} \
             -D MUSICA_BUILD_FORTRAN_INTERFACE=ON \
    && cd build \
    && make install -j 8
    
# Try building a simple application
RUN cd musica/build \
    && gcc $(pkg-config --cflags musica) -o test_simple /musica/src/test/test_simple.c $(pkg-config --libs musica) \
    && ./test_simple

WORKDIR musica/build