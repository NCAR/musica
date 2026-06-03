FROM fedora:latest

ARG BUILD_TYPE=Release

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
        libcurl-devel \
        m4 \
        make \
        netcdf-fortran-devel \
        valgrind \
        zlib-devel \
    && dnf clean all

ENV FC=gfortran
ENV CXX=g++
ENV CC=gcc
ENV FFLAGS="-I/usr/include/"

COPY . musica

RUN cd musica \
    && cmake -S . \
             -B build \
             -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
             -D MUSICA_BUILD_FORTRAN_INTERFACE=ON \
             -D MUSICA_ENABLE_TESTS=ON \
    && cd build \
    && make install -j$(nproc)

# Verify musica is findable via pkg-config with the GCC compiler.
RUN cd musica/build \
    && $CC $(pkg-config --cflags musica) -o test_simple /musica/src/test/test_simple.c $(pkg-config --libs musica) \
    && ./test_simple

WORKDIR /musica/build