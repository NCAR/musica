cmake_minimum_required(VERSION 3.21)

project(
  musica
  VERSION 0.3.0
  LANGUAGES Fortran CXX C
)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_MODULE_PATH}/SetDefaults.cmake")

include(CMakeDependentOption)

# Library options to build
option(USE_MUSICA "Use MUSICA" ON)
message(STATUS "Building MUSICA sources into a library : ${USE_MUSICA}")

option(USE_MUSICA_FORTRAN "Use MUSICA Fortran interface" OFF)
message(STATUS "Building MUSICA-Fortran sources into a library : ${USE_MUSICA_FORTRAN}")

# Projet wide setup options
option(ENABLE_TESTS "Builds tests that ensures each enabled MUSICA component can be used" ON)
message(STATUS "Build tests for MUSICA : ${ENABLE_TESTS}")

option(ENABLE_TUVX "Builds TUV-x, a photolysis calculator library" OFF)
message(STATUS "Build TUV-x : ${ENABLE_TUVX}")

option(ENABLE_MICM "Adds MICM, a model independent chemical mechanism solver" ON)
message(STATUS "Build MICM : ${ENABLE_MICM}")

option(ENABLE_MPI "Enable MPI parallel support" OFF)
message(STATUS "Enable MPI parallel support : ${ENABLE_MPI}")

option(ENABLE_OPENMP "Enable OpemMP support" OFF)
message(STATUS "Enable OpemMP support : ${ENABLE_OPENMP}")

option(CREATE_ENVIRONMENT_MODULE "Creates an Lmod environment module file that can be installed on the same machine this library is installed to." OFF)
message(STATUS "Creates an Lmod environment module : ${CREATE_ENVIRONMENT_MODULE}")

if(CREATE_ENVIRONMENT_MODULE)
  set(INSTALL_MODULE_FILE_PATH "" CACHE STRING "This is the path of the modulefiles location that the Lmod files should be installed to.")
endif()

include(GNUInstallDirs)
set(INSTALL_PREFIX "musica-${PROJECT_VERSION}")
set(INSTALL_MOD_DIR "${INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")

# MUSICA
if(USE_MUSICA)
  add_subdirectory(musica)
endif()

# MUSICA-Fortran
if(USE_MUSICA_FORTRAN)
  add_subdirectory(musica-fortran)
endif()

# MPI
if(ENABLE_MPI)
  add_definitions(-DMUSICA_USE_MPI)
endif()

# OpenMP
if(ENABLE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_Fortran_FOUND)
    message(STATUS "Compiling with OpenMP support")
    add_definitions(-DMUSICA_USE_OPENMP)
  else()
    message(FATAL_ERROR "OpenMP package not found")
  endif()
endif()

# Add flags for various compilers
if(${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
  add_definitions(-DMUSICA_USING_INTEL)
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
  add_definitions(-DMUSICA_USING_GNU)
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "PGI")
  add_definitions(-DMUSICA_USING_PGI)
endif()


################################################################################
# Tests
if(ENABLE_TESTS)
  enable_testing()
  add_subdirectory(musica/test)
endif()
