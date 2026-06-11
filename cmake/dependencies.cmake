include(FetchContent)

################################################################################
# Function to reduce repeated code, set a value to a variable only if the
# variable is not already defined. 
function(set_git_default git_var git_val)

  if(NOT ${git_var})
    set(${git_var} ${git_val} PARENT_SCOPE)
  endif()

endfunction(set_git_default)

################################################################################
# NetCDF library
#
# When MUSICA is built inside a host model (e.g. via add_subdirectory) the host
# has usually already located NetCDF and exposed an imported target. Different
# finders use different target names, so probe the common variants and reuse a
# host-provided target before searching ourselves. Only when nothing is found do
# we fall back to pkg-config. The resolved target names are stored in
# MUSICA_NETCDF_C_TARGET / MUSICA_NETCDF_FORTRAN_TARGET and linked directly by
# MUSICA's sources (we deliberately avoid wrapping them in a musica::-namespaced
# imported target, which would collide with the install(EXPORT NAMESPACE musica::)
# set and leak an unresolvable target into the exported link interface).

if (MUSICA_ENABLE_CARMA OR MUSICA_ENABLE_TUVX)
  # Return the first of ARGN that already exists as a target (target names are
  # case-sensitive, so we list each convention explicitly).
  function(musica_first_existing_target out_var)
    foreach(candidate ${ARGN})
      if(TARGET ${candidate})
        set(${out_var} ${candidate} PARENT_SCOPE)
        return()
      endif()
    endforeach()
    set(${out_var} "" PARENT_SCOPE)
  endfunction()

  musica_first_existing_target(MUSICA_NETCDF_C_TARGET
    netCDF::netcdf
    NetCDF::NetCDF_C
    netcdf
    NETCDF::NETCDF)
  musica_first_existing_target(MUSICA_NETCDF_FORTRAN_TARGET
    netCDF::netcdff
    NetCDF::NetCDF_Fortran
    netcdff)

  if(MUSICA_NETCDF_C_TARGET AND MUSICA_NETCDF_FORTRAN_TARGET)
    message(STATUS "MUSICA: reusing NetCDF targets from parent project: "
                   "${MUSICA_NETCDF_C_TARGET}, ${MUSICA_NETCDF_FORTRAN_TARGET}")
  else()
    message(STATUS "MUSICA: no parent NetCDF target found; locating via pkg-config")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)
    pkg_check_modules(netcdfc IMPORTED_TARGET REQUIRED netcdf)
    set(MUSICA_NETCDF_C_TARGET PkgConfig::netcdfc)
    set(MUSICA_NETCDF_FORTRAN_TARGET PkgConfig::netcdff)
  endif()
endif()

################################################################################
# Mechanism Configuration
# Skip if using prebuilt musica (already includes mechanism_configuration)
# fmt support enabled for NOAA CATChem model

if(MUSICA_BUILD_C_CXX_INTERFACE AND NOT MUSICA_USE_PREBUILT)
  set_git_default(MECH_CONFIG_GIT_REPOSITORY https://github.com/NCAR/MechanismConfiguration.git)
  set_git_default(MECH_CONFIG_GIT_TAG 0356ec366c3c202c3f8139e317a4241dac36a65b)

  FetchContent_Declare(mechanism_configuration
      GIT_REPOSITORY ${MECH_CONFIG_GIT_REPOSITORY}
      GIT_TAG ${MECH_CONFIG_GIT_TAG}
      GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
      FIND_PACKAGE_ARGS NAMES mechanism_configuration
  )

  set(MECH_CONFIG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(MECH_CONFIG_BUILD_SHARED_LIBS ${MUSICA_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
  set(MECH_CONFIG_USE_FMT ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(mechanism_configuration)
endif()

################################################################################
# google test

if(MUSICA_ENABLE_TESTS AND MUSICA_BUILD_C_CXX_INTERFACE)
  set_git_default(GOOGLETEST_GIT_REPOSITORY https://github.com/google/googletest.git)
  set_git_default(GOOGLETEST_GIT_TAG be03d00f5f0cc3a997d1a368bee8a1fe93651f48)

  FetchContent_Declare(googletest
    GIT_REPOSITORY ${GOOGLETEST_GIT_REPOSITORY}
    GIT_TAG ${GOOGLETEST_GIT_TAG}
    GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
    FIND_PACKAGE_ARGS NAMES GTest gtest
  )

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)
endif()

################################################################################
# OpenMP
if(MUSICA_ENABLE_OPENMP)
  find_package(OpenMP REQUIRED)
endif()

################################################################################
# MICM
# Skip if using prebuilt musica (already includes micm)

if (MUSICA_ENABLE_MICM AND MUSICA_BUILD_C_CXX_INTERFACE AND NOT MUSICA_USE_PREBUILT)
  set_git_default(MICM_GIT_REPOSITORY https://github.com/NCAR/micm.git)
  set_git_default(MICM_GIT_TAG 72222d12e649bc7a8a1dda74587517f10dc5f1ce)

  FetchContent_Declare(micm
      GIT_REPOSITORY ${MICM_GIT_REPOSITORY}
      GIT_TAG ${MICM_GIT_TAG}
      GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
      FIND_PACKAGE_ARGS NAMES micm
  )

  set(MICM_ENABLE_TESTS OFF)
  set(MICM_ENABLE_EXAMPLES OFF)
  set(MICM_DEFAULT_VECTOR_SIZE ${MUSICA_SET_MICM_DEFAULT_VECTOR_SIZE})
  set(MICM_BUILD_SHARED_LIBS ${MUSICA_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
  if(NOT APPLE)
    set(MICM_GPU_TYPE ${MUSICA_GPU_TYPE})
  endif()

  FetchContent_MakeAvailable(micm)
endif()

################################################################################
# TUV-x
# Skip if using prebuilt musica (already includes tuvx)

if (MUSICA_ENABLE_TUVX AND MUSICA_BUILD_C_CXX_INTERFACE AND NOT MUSICA_USE_PREBUILT)
  set(TUVX_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(TUVX_MOD_DIR ${MUSICA_MOD_DIR} CACHE STRING "" FORCE)
  set(TUVX_INSTALL_MOD_DIR ${MUSICA_INSTALL_MOD_DIR} CACHE STRING "" FORCE)
  set(TUVX_INSTALL_INCLUDE_DIR ${MUSICA_INSTALL_INCLUDE_DIR} CACHE STRING "" FORCE)

  # NOTE: `docker/Dockerfile.tuvx` extracts TUVX_GIT_REPOSITORY and TUVX_GIT_TAG
  #       from this script to set up tests against stand-alone TUV-x
  set_git_default(TUVX_GIT_REPOSITORY https://github.com/NCAR/tuv-x.git)
  set_git_default(TUVX_GIT_TAG 746f9ae6a1234362935c59987ae005cb799eb29b)

  FetchContent_Declare(tuvx
    GIT_REPOSITORY ${TUVX_GIT_REPOSITORY}
    GIT_TAG ${TUVX_GIT_TAG}
    GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
    FIND_PACKAGE_ARGS NAMES tuvx
  )

  set(TUVX_ENABLE_TESTS OFF)
  set(TUVX_ENABLE_REGRESSION_TESTS OFF)

  FetchContent_MakeAvailable(tuvx)
endif()

################################################################################
# CARMA
# Skip if using prebuilt musica (already includes carma)

if(MUSICA_ENABLE_CARMA AND MUSICA_BUILD_C_CXX_INTERFACE AND NOT MUSICA_USE_PREBUILT)
  set_git_default(CARMA_GIT_REPOSITORY https://github.com/NCAR/CARMA-ACOM-dev.git)
  set_git_default(CARMA_GIT_TAG eb7255bb2e933c2eb617bce785ab6dbfcba27295)

  set(CARMA_MOD_DIR ${MUSICA_MOD_DIR} CACHE STRING "" FORCE)
  set(CARMA_INSTALL_MOD_DIR ${MUSICA_INSTALL_MOD_DIR} CACHE STRING "" FORCE)
  set(CARMA_INSTALL_INCLUDE_DIR ${MUSICA_INSTALL_INCLUDE_DIR} CACHE STRING "" FORCE)

  # CARMA needs these
  find_package(BLAS REQUIRED)
  find_package(LAPACK REQUIRED)

  FetchContent_Declare(carma
      GIT_REPOSITORY ${CARMA_GIT_REPOSITORY}
      GIT_TAG ${CARMA_GIT_TAG}
      GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
  )

  set(CARMA_ENABLE_TESTS OFF)

  FetchContent_MakeAvailable(carma)
endif()

################################################################################
# pybind11
if(MUSICA_ENABLE_PYTHON_LIBRARY)
  set(PYBIND11_NEWPYTHON ON)

  set_git_default(PYBIND11_GIT_REPOSITORY https://github.com/pybind/pybind11)
  set_git_default(PYBIND11_GIT_TAG v3.0.1)

  FetchContent_Declare(pybind11
      GIT_REPOSITORY ${PYBIND11_GIT_REPOSITORY}
      GIT_TAG        ${PYBIND11_GIT_TAG}
      GIT_PROGRESS  NOT ${FETCHCONTENT_QUIET}
      FIND_PACKAGE_ARGS NAMES pybind11
  )

  FetchContent_MakeAvailable(pybind11)
endif()

################################################################################
# Julia

if (MUSICA_ENABLE_JULIA AND MUSICA_BUILD_C_CXX_INTERFACE)
  find_package(JlCxx REQUIRED)
endif()


################################################################################
# Docs

if(MUSICA_BUILD_DOCS)
  find_package(Doxygen REQUIRED)
  find_package(Sphinx REQUIRED)
endif()
