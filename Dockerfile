FROM fedora:35

# copy the musica core code
COPY . musica

RUN dnf -y update \
    && dnf -y install \
        cmake \
        gcc-c++ \
        gcc-gfortran \
        git \
        lcov \
        make \
        netcdf-fortran-devel \
        valgrind \
    && dnf clean all

ENV FC=gfortran

# install json-fortran
RUN curl -LO https://github.com/jacobwilliams/json-fortran/archive/8.2.0.tar.gz \
    && tar -zxvf 8.2.0.tar.gz \
    && cd json-fortran-8.2.0 \
    && mkdir build \
    && cd build \
    && cmake -D SKIP_DOC_GEN:BOOL=TRUE .. \
    && make install

# get a tag and build the model
RUN cd musica \
    && export JSON_FORTRAN_HOME="/usr/local/jsonfortran-gnu-8.2.0" \
    && cmake -S ./musica \
             -D ENABLE_TESTS=ON \
             -B ../build \
    && cd ../build \
    && make install -j 8 
    
WORKDIR ./build