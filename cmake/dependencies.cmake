find_package(PkgConfig REQUIRED)

################################################################################
# json-fortran library
find_path(JSON_INCLUDE_DIR json_module.mod
  DOC "json-fortran include directory (must include json_*.mod files)"
  PATHS
    $ENV{JSON_FORTRAN_HOME}/lib
    /opt/local/lib
    /usr/local/lib
    /usr/local/lib64)
find_library(JSON_LIB jsonfortran
  DOC "json-fortran library"
  PATHS
    $ENV{JSON_FORTRAN_HOME}/lib
    /opt/local/lib
    /usr/local/lib
    /usr/local/lib64)
include_directories(${JSON_INCLUDE_DIR})

################################################################################
# NetCDF library
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

################################################################################
# google test
if(MUSICA_ENABLE_TESTS)
  include(FetchContent)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG be03d00f5f0cc3a997d1a368bee8a1fe93651f48
  )

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)
endif()

################################################################################
# OpenMP
if(MUSICA_ENABLE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_Fortran_FOUND)
    message(STATUS "Compiling with OpenMP support")
  else()
    message(FATAL_ERROR "OpenMP package not found")
  endif()
endif()

################################################################################
# MICM

if (MUSICA_ENABLE_MICM)
  FetchContent_Declare(json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.11.2
  )
  FetchContent_MakeAvailable(json)
endif()