# MUSICA Docker Builds
This directory contains dockerfiles use for both development
and testing in GitHub actions. Specifics for building some of 
these and when they are used are listed here.


## [Dockerfile.fetch-content](./Dockerfile.fetch-content)

This file contains multiple build stages and shows that our C/C++
interface can be built with gcc, and that our Fortran
interface can be built with any other compiler and use
the GCC-built C/C++ interface. In addition, this file ensures that the pkg-config
builds work for our fortran interface.

| Stage        | Description                                         | Supported Platforms        |
|--------------|-----------------------------------------------------|---------------------------|
| Base         | Builds MUSICA with GCC                              | x86-64, ARM64            |
| GCC Test     | Builds Fortran tests with GCC                       | x86-64, ARM64            |
| Intel Test   | Builds tests with Intel oneAPI (`ifx/icx/icpx`)    | x86-64 only              |
| NVHPC Test   | Builds tests with NVHPC (`nvfortran/nvc/nvc++`), GPU-enabled | x86-64 only      |

These are the commans that can be used to build each layer

### GCC

```bash
docker build -t musica-gcc -f docker/Dockerfile.fortran-multistage --target gcc-test --platform linux/amd64 .

# or ARM
docker build -t musica-gcc -f docker/Dockerfile.fortran-multistage --target gcc-test --platform linux/arm64 .
```

### Intel

```bash
docker build -t musica-gcc -f docker/Dockerfile.fortran-multistage --target intel-test --platform linux/amd64 .
```

### NVHPC
```bash
docker build -t musica-gcc -f docker/Dockerfile.fortran-multistage --target nvhpc-test --platform linux/amd64 .

# or ARM
docker build -t musica-gcc -f docker/Dockerfile.fortran-multistage --target nvhpc-test --platform linux/arm64 .
```