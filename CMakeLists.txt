cmake_minimum_required(VERSION 3.21)

# must be on the same line so that pyproject.toml can correctly identify the version
project(musica-distribution VERSION 0.7.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_MODULE_PATH}/SetDefaults.cmake)
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

cmake_dependent_option(
  MUSICA_ENABLE_PYTHON_LIBRARY "Adds pybind11, a lightweight header-only library that exposes C++ types in Python and vice versa" OFF "MUSICA_BUILD_C_CXX_INTERFACE" OFF)

cmake_dependent_option(
  MUSICA_CREATE_ENVIRONMENT_MODULE "Creates an Lmod environment module file that can be installed on the same machine this library is installed to." OFF "MUSICA_BUILD_C_CXX_INTERFACE" OFF)

if(MUSICA_CREATE_ENVIRONMENT_MODULE)
  set(MUSICA_INSTALL_MODULE_FILE_PATH "" CACHE STRING "This is the path of the modulefiles location that the Lmod files should be installed to.")
  if(MUSICA_INSTALL_MODULE_FILE_PATH STREQUAL "")
    message(FATAL_ERROR "MUSICA_INSTALL_MODULE_FILE_PATH is required but not set")
  endif()
endif()

# shared libraries are required for python, but for some reason setting
# BUILD_SHARED_LIBS to ON doesn't work on windows
if (MUSICA_ENABLE_PYTHON_LIBRARY AND NOT WIN32) 
  message(STATUS "Python requires shared libraries to be built. Enabling shared libraries.")
  set(BUILD_SHARED_LIBS ON)
endif()

################################################################################
# Projet wide setup variables 
set(MUSICA_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(MUSICA_MOD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set(MUSICA_LIB_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(MUSICA_FORTRAN_SRC_DIR ${PROJECT_SOURCE_DIR}/fortran)
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

include(dependencies)

if(MUSICA_BUILD_DOCS)
  add_subdirectory(docs)
endif()

################################################################################
# Tests
if(MUSICA_ENABLE_TESTS)
  enable_testing()
endif()

################################################################################
# MUSICA
if(MUSICA_BUILD_C_CXX_INTERFACE)
  add_subdirectory(src)
endif()

################################################################################
# MUSICA-Fortran
if(MUSICA_BUILD_FORTRAN_INTERFACE)
  add_subdirectory(fortran)
endif()

################################################################################
# Musica python
if(MUSICA_ENABLE_PYTHON_LIBRARY)
  add_subdirectory(python)
endif()

################################################################################
###
# Configure and display a summary file for how musica was built.
###
include(summary)
