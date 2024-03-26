cmake_minimum_required(VERSION 3.21)

project(
  musica-distribution
  VERSION 0.6.1
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake)
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

cmake_dependent_option(
  MUSICA_ENABLE_TUVX "Builds TUV-x, a photolysis calculator library" ON "MUSICA_BUILD_FORTRAN_INTERFACE" OFF)

cmake_dependent_option(
  MUSICA_ENABLE_MICM "Adds MICM, a model independent chemical mechanism solver" ON "MUSICA_BUILD_C_CXX_INTERFACE" OFF)

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
set(MUSICA_FORTRAN_SRC_DIR ${CMAKE_SOURCE_DIR}/fortran)

# Add flags for various compilers
if(${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
  add_definitions(-DMUSICA_USING_INTEL)
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
  add_definitions(-DMUSICA_USING_GNU)
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "PGI")
  add_definitions(-DMUSICA_USING_PGI)
endif()

if(MUSICA_BUILD_C_CXX_INTERFACE)
  # on ubuntu with clang, an incorrect version of the c++ standard library was being linked
  if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      # If the compiler is Clang, use libc++
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()

  set(CMAKE_CXX_STANDARD 20)
endif()

################################################################################
# Dependencies

include(musica_util)

# Add submodules
checkout_submodules()

include(dependencies)

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