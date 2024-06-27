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
if (MUSICA_BUILD_FORTRAN_INTERFACE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)
  pkg_check_modules(netcdfc IMPORTED_TARGET REQUIRED netcdf)
endif()

################################################################################
# google test
if(MUSICA_ENABLE_TESTS)

  set_git_default(GOOGLETEST_GIT_REPOSITORY https://github.com/google/googletest.git)
  set_git_default(GOOGLETEST_GIT_TAG be03d00f5f0cc3a997d1a368bee8a1fe93651f48)

  FetchContent_Declare(googletest
    GIT_REPOSITORY ${GOOGLETEST_GIT_REPOSITORY}
    GIT_TAG ${GOOGLETEST_GIT_TAG}
    GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
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

if (MUSICA_ENABLE_MICM AND MUSICA_BUILD_C_CXX_INTERFACE)

  set_git_default(MICM_GIT_REPOSITORY https://github.com/NCAR/micm.git)
  set_git_default(MICM_GIT_TAG v3.5.0)

  FetchContent_Declare(micm
      GIT_REPOSITORY ${MICM_GIT_REPOSITORY}
      GIT_TAG ${MICM_GIT_TAG}
      GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
  )
  set(MICM_ENABLE_TESTS OFF)
  set(MICM_ENABLE_EXAMPLES OFF)

  FetchContent_MakeAvailable(micm)
endif()

################################################################################
# TUV-x

if (MUSICA_ENABLE_TUVX AND MUSICA_BUILD_C_CXX_INTERFACE)
  set(TUVX_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(TUVX_MOD_DIR ${MUSICA_MOD_DIR} CACHE STRING "" FORCE)
  set(TUVX_INSTALL_MOD_DIR ${MUSICA_INSTALL_INCLUDE_DIR} CACHE STRING "" FORCE)
  set(TUVX_INSTALL_INCLUDE_DIR ${MUSICA_INSTALL_INCLUDE_DIR} CACHE STRING "" FORCE)

  set_git_default(TUVX_GIT_REPOSITORY https://github.com/NCAR/tuv-x.git)
  set_git_default(TUVX_GIT_TAG v0.9.0)

  FetchContent_Declare(tuvx
    GIT_REPOSITORY ${TUVX_GIT_REPOSITORY}
    GIT_TAG ${TUVX_GIT_TAG}
    GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
  )

  FetchContent_MakeAvailable(tuvx)
endif()

################################################################################
# pybind11
if(MUSICA_ENABLE_PYTHON_LIBRARY)
  set(PYBIND11_NEWPYTHON ON)

  set_git_default(PYBIND11_GIT_REPOSITORY https://github.com/pybind/pybind11)
  set_git_default(PYBIND11_GIT_TAG v2.12.0)

  FetchContent_Declare(pybind11
      GIT_REPOSITORY ${PYBIND11_GIT_REPOSITORY}
      GIT_TAG        ${PYBIND11_GIT_TAG}
      GIT_PROGRESS  NOT ${FETCHCONTENT_QUIET}
  )

  FetchContent_GetProperties(pybind11)
  if(NOT pybind11_POPULATED)
      FetchContent_Populate(pybind11)
      add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
  endif()
endif()

################################################################################
# Docs

if(MUSICA_BUILD_DOCS)
  find_package(Sphinx REQUIRED)
endif()
