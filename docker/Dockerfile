FROM fedora:latest

ARG BUILD_TYPE=Release

RUN dnf -y install \
        cmake \
        curl \
        gcc \
        gcc-c++ \
        gcc-gfortran \
        gdb \
        git \
        hdf5-devel \
        lapack-devel \
        libasan \
        make \
        netcdf-fortran-devel \
        zlib-devel \
    && dnf clean all

ENV FC=gfortran
ENV CXX=g++
ENV CC=gcc
ENV FFLAGS="-I/usr/include/"

COPY . musica

RUN cd musica \
    && cmake -S . \
        -D CMAKE_C_FLAGS="-fsanitize=address -g" \
        -D CMAKE_CXX_FLAGS="-fsanitize=address -g" \
        -D CMAKE_Fortran_FLAGS="-fsanitize=address -g" \
        -D CMAKE_EXE_LINKER_FLAGS="-fsanitize=address" \
        -D CMAKE_SHARED_LINKER_FLAGS="-fsanitize=address" \
        -D MUSICA_BUILD_FORTRAN_INTERFACE=ON \
        -D MUSICA_ENABLE_TESTS=ON \
        -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -B build \
    && cd build \
    && make install -j$(nproc)

# Copy the tuvx configs for testing
RUN cp -r /musica/configs/tuvx/* /musica/build/

# Verify musica is findable via pkg-config with the GCC compiler.
RUN cd musica/build \
    && gcc -fsanitize=address $(pkg-config --cflags musica) -o test_simple /musica/src/test/test_simple.c $(pkg-config --libs musica) -fsanitize=address \
    && ./test_simple

WORKDIR /musica/build