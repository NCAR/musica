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

if (MUSICA_ENABLE_CARMA OR MUSICA_ENABLE_TUVX)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)
  pkg_check_modules(netcdfc IMPORTED_TARGET REQUIRED netcdf)
endif()

################################################################################
# Mechanism Configuration
# Skip if using prebuilt musica (already includes mechanism_configuration)

if(MUSICA_BUILD_C_CXX_INTERFACE AND NOT MUSICA_USE_PREBUILT)
  set_git_default(MECH_CONFIG_GIT_REPOSITORY https://github.com/NCAR/MechanismConfiguration.git)
  set_git_default(MECH_CONFIG_GIT_TAG fcf550e5b3e97cb13d93a382cd4230578eb923f0)

  FetchContent_Declare(mechanism_configuration
      GIT_REPOSITORY ${MECH_CONFIG_GIT_REPOSITORY}
      GIT_TAG ${MECH_CONFIG_GIT_TAG}
      GIT_PROGRESS NOT ${FETCHCONTENT_QUIET}
      FIND_PACKAGE_ARGS NAMES mechanism_configuration
  )

  set(MECH_CONFIG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(MECH_CONFIG_BUILD_SHARED_LIBS ${MUSICA_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)

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
  set_git_default(MICM_GIT_TAG e854d2cf4d0abc7c6fb243e5406e751c462f3679)

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
  set_git_default(TUVX_GIT_TAG v0.14.0)

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
  set_git_default(CARMA_GIT_TAG develop-carma-box)

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
  # Check if JlCxx is already available in CMAKE_PREFIX_PATH (BinaryBuilder mode)
  # This allows building without running Julia, which is needed for cross-compilation
  find_package(JlCxx QUIET)

  if(JlCxx_FOUND)
    message(STATUS "Found JlCxx via CMAKE_PREFIX_PATH (BinaryBuilder mode)")
  else()
    # Standard mode: Use Julia to find CxxWrap
    find_program(Julia_EXECUTABLE julia REQUIRED)

    # Use the Julia project in the top-level julia subdirectory
    set(JULIA_PROJECT_DIR "${CMAKE_SOURCE_DIR}/julia")

    execute_process(
      COMMAND ${Julia_EXECUTABLE} --project=${JULIA_PROJECT_DIR} -e "using Pkg; Pkg.instantiate()"
      RESULT_VARIABLE PKG_INSTANTIATE_RESULT
      OUTPUT_VARIABLE PKG_INSTANTIATE_OUTPUT
      ERROR_VARIABLE PKG_INSTANTIATE_ERROR
    )

    if(NOT PKG_INSTANTIATE_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Failed to instantiate Julia project dependencies in ${JULIA_PROJECT_DIR}.\n"
            "Stdout: ${PKG_INSTANTIATE_OUTPUT}\n"
            "Stderr: ${PKG_INSTANTIATE_ERROR}"
        )
    endif()

    # Try to get CxxWrap prefix path
    execute_process(
        COMMAND ${Julia_EXECUTABLE} --project=${JULIA_PROJECT_DIR} -e "using CxxWrap; print(CxxWrap.prefix_path())"
        OUTPUT_VARIABLE CxxWrap_PREFIX
        RESULT_VARIABLE CxxWrap_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE CxxWrap_ERROR
    )
    if(CxxWrap_RESULT EQUAL 0 AND EXISTS ${CxxWrap_PREFIX})
        message(STATUS "CxxWrap prefix: ${CxxWrap_PREFIX}")
        list(APPEND CMAKE_PREFIX_PATH ${CxxWrap_PREFIX})
        find_package(JlCxx REQUIRED)
    else()
        message(FATAL_ERROR
            "Failed to find CxxWrap.jl. Error: ${CxxWrap_ERROR}\n"
            "Please ensure CxxWrap is properly installed"
        )
    endif()
  endif()
endif()


################################################################################
# Docs

if(MUSICA_BUILD_DOCS)
  find_package(Doxygen REQUIRED)
  find_package(Sphinx REQUIRED)
endif()
