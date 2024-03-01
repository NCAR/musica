FROM fedora:35

RUN dnf -y update \
    && dnf -y install \
        cmake \
        gcc-c++ \
        gcc-gfortran \
        git \
        lcov \
        make \
        netcdf-fortran-devel \
        json-devel \
        valgrind \
    && dnf clean all

# Install json-fortran
RUN curl -LO https://github.com/jacobwilliams/json-fortran/archive/8.2.0.tar.gz \
    && tar -zxvf 8.2.0.tar.gz \
    && cd json-fortran-8.2.0 \
    && mkdir build \
    && cd build \
    && cmake -D SKIP_DOC_GEN:BOOL=TRUE .. \
    && make install

# Set environment variables
ENV FC=gfortran
ENV JSON_FORTRAN_HOME="/usr/local/jsonfortran-gnu-8.2.0"

# Copy the musica code
COPY . musica

# Build
RUN cd musica \
    && cmake -S . \
             -B build \
             -D ENABLE_TESTS=ON \
    && cd build \
    && make install -j 8 

# Copy the configuration file for the MICM C API test
RUN cp -r musica/test/configs/chapman musica/build/chapman

WORKDIR musica/build
