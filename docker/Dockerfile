FROM fedora:35

ARG BUILD_TYPE=Release

RUN dnf -y update \
    && dnf -y install \
        cmake \
        gcc-c++ \
        gfortran \
        gdb \
        git \
        lapack-devel \
        lcov \
        make \
        netcdf-fortran-devel \
        json-devel \
        python \
        valgrind \
    && dnf clean all

# Copy the musica code
COPY . musica

# Build
RUN cd musica \
    && cmake -S . \
             -B build \
             -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
    && cd build \
    && make install -j 8 

WORKDIR musica/build
