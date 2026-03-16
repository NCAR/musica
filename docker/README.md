# MUSICA Docker Builds
This directory contains dockerfiles use for both development
and testing in GitHub actions. Specifics for building some of 
these and when they are used are listed here.

If using the commands below, all targets can be run with 

```bash
docker run --rm -it musica
```

Many (not all) are setup to run tests immediately with `make test` after the container starts. Exceptions
are noted in their sections.


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

These are the commands that can be used to build each layer. Because this file is meant to test integration
with fetch content, we must also supply the current branch as a build argument, like `--build-arg MUSICA_GIT_TAG=feature_branch`,
so be sure to update that argument when running on your computer.

You may exclude the `--platform` argument, but if you want to check a build for a specific
architecture, they are provided in the commands here. The only exclusion is intel, which
requires an `amd64` architecture.


### GCC

```bash
docker build -t musica-gcc -f docker/Dockerfile.fetch-content --build-arg MUSICA_GIT_TAG=feature_branch --target gcc-test --platform linux/amd64 .

# or ARM
docker build -t musica-gcc -f docker/Dockerfile.fetch-content --build-arg MUSICA_GIT_TAG=feature_branch --target gcc-test --platform linux/arm64 .
```

### Intel

```bash
docker build -t musica-gcc -f docker/Dockerfile.fetch-content --build-arg MUSICA_GIT_TAG=feature_branch --target intel-test --platform linux/amd64 .
```

### NVHPC
```bash
docker build -t musica-gcc -f docker/Dockerfile.fetch-content --build-arg MUSICA_GIT_TAG=feature_branch --target nvhpc-test --platform linux/amd64 .

# or ARM
docker build -t musica-gcc -f docker/Dockerfile.fetch-content --build-arg MUSICA_GIT_TAG=feature_branch --target nvhpc-test --platform linux/arm64 .
```