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

WORKDIR musica/build