cmake_minimum_required(VERSION 3.21)

project(
  ccpp_integration
  VERSION 0.0.0
  LANGUAGES Fortran
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(musica 0.2.0 REQUIRED)
find_package(PkgConfig REQUIRED)

# Find libstdc++
find_library(LIBSTDCXX_PATH NAMES libstdc++.so.6 
  PATHS 
    /usr/lib
    /usr/lib64 
  REQUIRED
)

# NetCDF library
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

# this path is the one expected in the docker image
add_subdirectory(/musica/ccpp ${CMAKE_BINARY_DIR}/__ccpp)

################################################################################
# Test utilities

include(test_util.cmake)

################################################################################
# Tests

create_standard_test(NAME micm_ccpp_api SOURCES micm.F90 LIBRARIES musica::ccpp)

target_link_libraries(test_micm_ccpp_api PRIVATE ${LIBSTDCXX_PATH})