FROM fedora:35

RUN dnf -y update \
    && dnf -y install \
        cmake \
        gcc-c++ \
        gcc-gfortran \
        lcov \
        make \
        netcdf-fortran-devel \
        valgrind \
    && dnf clean all

# install json-fortran
RUN curl -LO https://github.com/jacobwilliams/json-fortran/archive/8.2.0.tar.gz \
    && tar -zxvf 8.2.0.tar.gz \
    && cd json-fortran-8.2.0 \
    && export FC=gfortran \
    && mkdir build \
    && cd build \
    && cmake -D SKIP_DOC_GEN:BOOL=TRUE .. \
    && make install

# copy the musica core code
COPY . /musica/

# get a tag and build the model
RUN mkdir /build \
    && cd /build \
    && export JSON_FORTRAN_HOME="/usr/local/jsonfortran-gnu-8.2.0" \
    && cmake -D ENABLE_COVERAGE:BOOL=FALSE \
          ../musica \
    && make install -j 8

WORKDIR /build