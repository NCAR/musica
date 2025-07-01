#! /bin/bash

set -e
set -x

# Install MSYS2 and required dependencies for Windows builds
# This script sets up the environment needed for building MUSICA with TUV-x on Windows

# Update package database and install base dependencies
pacman -Syu --noconfirm

# Install build tools and compilers
pacman -S --noconfirm \
    base-devel \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-gcc-fortran \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-netcdf \
    mingw-w64-x86_64-netcdf-fortran

# Add mingw64 to PATH
export PATH="/mingw64/bin:$PATH"

echo "Windows build environment setup complete"
