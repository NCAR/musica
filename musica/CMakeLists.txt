################################################################################
# Preamble

cmake_minimum_required(VERSION 3.21)
set(CMAKE_USER_MAKE_RULES_OVERRIDE "cmake/SetDefaults.cmake")

project(
  musica
  VERSION 0.3.0
  LANGUAGES Fortran CXX C
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message (STATUS "CMake build configuration for musica(${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

include(musica_util)

# Add submodules
checkout_submodules()

################################################################################
# Projet wide setup options
include(CMakeDependentOption)

option( ENABLE_TESTS "Builds tests that ensures each enabled MUSICA component can be used" OFF )

option( ENABLE_TUVX "Builds TUV-x, a photolysis calculator library" ON )
option( ENABLE_MICM "Adds MICM, a model independent chemical mechanism solver" ON )

option( ENABLE_MPI "Enable MPI parallel support" OFF)
option( ENABLE_OPENMP "Enable OpemMP support" OFF )

option( CREATE_ENVIRONMENT_MODULE "Creates an Lmod environment module file that can be installed on the same machine this library is installed to." OFF )

if(CREATE_ENVIRONMENT_MODULE)
  set(INSTALL_MODULE_FILE_PATH "" CACHE STRING "This is the path of the modulefiles location that the Lmod files should be installed to.")
endif()

# Set up include and lib directories
set( MUSICA_MOD_DIR "${PROJECT_BINARY_DIR}/include" )
set( MUSICA_LIB_DIR "${PROJECT_BINARY_DIR}/libs" )

# Set parent directory to get submodules
cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH PARENT_DIR)

include(GNUInstallDirs)
set(INSTALL_PREFIX "musica-${PROJECT_VERSION}")
set(INSTALL_MOD_DIR "${INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")

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
# Dependencies

include(cmake/dependencies.cmake)

##############################################################################
# MUSICA targets

####################
# MUSICA
add_library(musica STATIC)
add_library(musica::musica ALIAS musica)

add_subdirectory(src)

set_target_properties(musica PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(musica 
  PUBLIC
    PkgConfig::netcdff
    ${JSON_LIB}
)

target_include_directories(musica
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
)

target_include_directories(musica
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
)

####################
# MUSICA-core
add_library(musicacore_object)
add_library(musica::musicacore ALIAS musicacore_object)

set(ENABLE_UTIL_ONLY ON)

# In case of absolute source path, build directory needs to be explicitly provided
add_subdirectory(${PARENT_DIR}/lib/musica-core/src ${MUSICA_LIB_DIR}/musica-core/src)

set_target_properties(musicacore_object PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
)

target_include_directories(musicacore_object 
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
)

target_link_libraries(musicacore_object 
  PUBLIC
    PkgConfig::netcdff
)

# add the sources to musica
target_sources(musica
  PRIVATE
    $<TARGET_OBJECTS:musicacore_object>
)

# ####################
# TUV-x
if (ENABLE_TUVX)
  add_definitions(-DMUSICA_USE_TUVX)

  set(TUVX_MOD_DIR ${MUSICA_MOD_DIR})
  set(TUVX_LIB_DIR ${MUSICA_LIB_DIR})
  
  # In case of absolute source path, build directory needs to be explicitly provided
  add_subdirectory(${PARENT_DIR}/lib/tuv-x/src ${MUSICA_LIB_DIR}/tuv-x/src)
  
  set_target_properties(tuvx_object PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  )

  target_include_directories(tuvx_object 
    PUBLIC  
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
  )

  target_link_libraries(tuvx_object 
    PUBLIC
      musicacore_object
  )

  # add the sources to musica
  target_sources(musica
    PRIVATE
      $<TARGET_OBJECTS:tuvx_object>
  )
endif()

# ####################
# MICM
if (ENABLE_MICM)
  add_definitions(-DMUSICA_USE_MICM)
  target_compile_features(musica PUBLIC cxx_std_20)

  target_include_directories(musica
    PUBLIC  
      $<BUILD_INTERFACE:${PARENT_DIR}/lib/micm/include>
      $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
  )

  message("what is install interface: ${INSTALL_MOD_DIR}")

  install(
    DIRECTORY 
      ${PARENT_DIR}/lib/micm/include/micm
    DESTINATION 
      ${INSTALL_MOD_DIR}
  )
endif()

# ################################################################################
# # Tests

# # only include packaging if we are the top level project being built
# if(PROJECT_IS_TOP_LEVEL AND ENABLE_TESTS)
#   enable_testing()
#   add_subdirectory(test)
# endif()

# ################################################################################
# # Packaging

# # only include packaging if we are the top level project being built
# if(PROJECT_IS_TOP_LEVEL)
#   add_subdirectory(packaging)
# endif()