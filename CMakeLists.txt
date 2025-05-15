cmake_minimum_required(VERSION 3.21)

# must be on the same line so that pyproject.toml can correctly identify the version
project(musica-distribution VERSION 0.11.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${PROJECT_SOURCE_DIR}/cmake/SetDefaults.cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

include(GNUInstallDirs)

################################################################################
# Library options to build
include(CMakeDependentOption)

option(MUSICA_BUILD_C_CXX_INTERFACE "Use MUSICA" ON)
option(MUSICA_BUILD_FORTRAN_INTERFACE "Use MUSICA-Fortran interface" OFF)
option(MUSICA_ENABLE_INSTALL "Install the musica library" ON)
option(MUSICA_ENABLE_TESTS "Builds tests that ensures each enabled MUSICA component can be used" ON)
option(MUSICA_ENABLE_MPI "Enable MPI parallel support" OFF)
option(MUSICA_ENABLE_OPENMP "Enable OpemMP support" OFF)
option(MUSICA_ENABLE_MEMCHECK "Enable memory checking" OFF)
option(MUSICA_BUILD_DOCS "Build the documentation" OFF)
option(MUSICA_ENABLE_MICM "Enable MICM" ON)
option(MUSICA_ENABLE_TUVX "Enable TUV-x" ON)
option(MUSICA_ENABLE_PIC "Build the library with position independent code" OFF)
option(MUSICA_ENABLE_COVERAGE "Enable code coverage output" OFF)
option(MUSICA_BUNDLE_DEPENDENCIES "Bundle dependencies with the library" ON)

set(MUSICA_GPU_TYPE "None" CACHE STRING "The GPU type being targeted")

set(MUSICA_SET_MICM_DEFAULT_VECTOR_SIZE "4" CACHE STRING "Set MICM vector-ordered matrix dimension")

cmake_dependent_option(
  MUSICA_ENABLE_PYTHON_LIBRARY "Adds pybind11, a lightweight header-only library that exposes C++ types in Python and vice versa" OFF "MUSICA_BUILD_C_CXX_INTERFACE" OFF)

################################################################################
# Projet wide setup variables 
set(MUSICA_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(MUSICA_MOD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set(MUSICA_LIB_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(MUSICA_PROJECT_SRC_DIR ${PROJECT_SOURCE_DIR})

set(musica_compile_definitions "")

# Add flags for various compilers
if(${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
  list(APPEND musica_compile_definitions MUSICA_USING_INTEL)
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
  list(APPEND musica_compile_definitions MUSICA_USING_GNU)
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "PGI")
  list(APPEND musica_compile_definitions MUSICA_USING_PGI)
endif()

# Set the C++ standard library if it is not already set
if(NOT CMAKE_CXX_STANDARD_LIBRARIES)
  if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++")
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++")
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "IBM")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++")
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "NVHPC")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++")
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++") # use GNU standard library
  else()
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++") # default to most common name
  endif()
endif()

if(MUSICA_BUILD_C_CXX_INTERFACE)
  # must be global so that it also applies to dependencies like google test, unless we want
  # to set it for each target
  # on ubuntu with clang, an incorrect version of the c++ standard library was being linked
  if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      # If the compiler is Clang and the Linux version is Ubuntu 20.04 or 22.04, use libc++ explicitly
      if (${CMAKE_HOST_SYSTEM_VERSION} MATCHES "20.04" OR ${CMAKE_HOST_SYSTEM_VERSION} MATCHES "22.04")
          message(STATUS "Using libc++ explicitly with Clang on Ubuntu 20.04 or 22.04")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -stdlib=libc++")
      endif()
  endif()
endif()

# Add flags when using the ClangCL toolset
if(CMAKE_GENERATOR_TOOLSET STREQUAL "ClangCL")
  list(APPEND musica_compile_definitions MUSICA_USING_CLANGCL)
endif()

# Set the Valgrind suppressions file for tests
set(MEMCHECK_SUPPRESS "--suppressions=${PROJECT_SOURCE_DIR}/valgrind.supp")

################################################################################
# Dependencies

if(MUSICA_BUNDLE_DEPENDENCIES)
  include(${PROJECT_SOURCE_DIR}/cmake/dependencies.cmake)
endif()

################################################################################
# Tests
if(MUSICA_ENABLE_TESTS)
  if(MUSICA_ENABLE_COVERAGE)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE "ctest"
        EXCLUDE "${PROJECT_SOURCE_DIR}/test/*"
        BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src")
    
  endif()
  
  enable_testing()
endif()

################################################################################
# MUSICA
if(MUSICA_BUILD_C_CXX_INTERFACE)
  add_subdirectory(src)
endif()

if(MUSICA_BUILD_DOCS)
  add_subdirectory(docs)
endif()

################################################################################
# MUSICA-Fortran
if(MUSICA_BUILD_FORTRAN_INTERFACE)
  add_subdirectory(fortran)
endif()

################################################################################
# Musica python
if(MUSICA_ENABLE_PYTHON_LIBRARY)
  add_subdirectory(musica)
endif()

################################################################################
###
# Configure and display a summary file for how musica was built.
###
include(${PROJECT_SOURCE_DIR}/cmake/summary.cmake)
