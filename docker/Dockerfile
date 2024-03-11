FROM fedora:35

RUN dnf -y update \
    && dnf -y install \
        cmake \
        gcc-c++ \
        gcc-gfortran \
        gdb \
        git \
        lcov \
        make \
        netcdf-fortran-devel \
        json-devel \
        valgrind \
    && dnf clean all

# Set environment variables
ENV FC=gfortran

# Copy the musica code
COPY . musica

# Build
RUN cd musica \
    && cmake -S . \
             -B build \
             -D ENABLE_TESTS=ON \
    && cd build \
    && make install -j 8 

WORKDIR musica/build
