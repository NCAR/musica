FROM fedora:35

RUN dnf -y update \
    && dnf -y install \
        cmake \
        gcc-c++ \
        gdb \
        git \
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
    && cd build \
    && make install -j 8 

WORKDIR musica/build
