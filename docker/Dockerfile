FROM fedora:latest

ARG BUILD_TYPE=Debug

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
        python \
        valgrind \
        tree \
    && dnf clean all

# Copy the musica code
COPY . musica

# Build
RUN cd musica \
    && cmake -S . \
        -DCMAKE_C_FLAGS="-fsanitize=address -g" \
        -DCMAKE_CXX_FLAGS="-fsanitize=address -g" \
        -DCMAKE_Fortran_FLAGS="-fsanitize=address -g" \
        -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" \
        -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=address" \
        -DMUSICA_BUILD_FORTRAN_INTERFACE=ON \
        -B build \
        -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
    && cd build \
    && make install -j 1
    
# Try building a simple application
RUN cd musica/build \
    && gcc -fsanitize=address $(pkg-config --cflags musica) -o test_simple /musica/src/test/test_simple.c $(pkg-config --libs musica) -fsanitize=address \
    && ./test_simple

# Copy the tuvx configs for testing
RUN cp -r /musica/configs/tuvx/* /musica/build/

WORKDIR /musica/build