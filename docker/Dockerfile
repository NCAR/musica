FROM fedora:latest

ARG MUSICA_GIT_TAG=main
ARG BUILD_TYPE=Release

RUN dnf -y update \
    && dnf -y install \
        cmake \
        gcc-c++ \
        gfortran \
        gdb \
        git \
        lcov \
        make \
        netcdf-fortran-devel \
        python \
        valgrind \
        tree \
    && dnf clean all

# Copy the musica code
COPY . musica

# Build
RUN cd musica \
    && cmake -S . \
             -B build \
             -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
             -D MUSICA_GIT_TAG=${MUSICA_GIT_TAG} \
    && cd build \
    && make install -j 8
    
# Try building a simple application
RUN cd musica/build \
    && gcc $(pkg-config --cflags musica) -o test_simple /musica/src/test/test_simple.c $(pkg-config --libs musica) \
    && ./test_simple

WORKDIR musica/build
