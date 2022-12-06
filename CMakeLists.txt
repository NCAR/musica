################################################################################
# Preamble

cmake_minimum_required(VERSION 3.21)
set(CMAKE_USER_MAKE_RULES_OVERRIDE "cmake/SetDefaults.cmake")

project(
  musica
  VERSION 0.1.0
  LANGUAGES Fortran
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/cmake-modules")

message ( STATUS "CMake build configuration for musica(${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}" )

include(musica_util)
checkout_submodules()

################################################################################
# Projet wide setup options
option( ENABLE_TUVX "Builds TUV-x, a photolysis calculator library" OFF )

# Set up include and lib directories
set( MUSICA_MOD_DIR "${PROJECT_BINARY_DIR}/include" )
set( MUSICA_LIB_DIR "${PROJECT_BINARY_DIR}/libs" )


################################################################################
# Dependencies

# MUSICA-core
add_library(musicacore)
add_library(musicacore::musicacore ALIAS musicacore)

add_subdirectory(libs/musica-core/src)

# TUV-x
if ( ENABLE_TUVX )
  add_subdirectory( libs/tuv-x )
  list( APPEND EXTRA_LIB_DIRS "libs/tuv-x" )
  list( APPEND EXTRA_INCLUDE_DIRS "libs/tuv-x" )
  list( APPEND EXTRA_LINKS tuvxlib )
endif ()

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

##############################################################################
# Musica target
##############################################################################
add_executable( musica main.F90 )

target_include_directories(musica
  PUBLIC
    ${EXTRA_INCLUDE_DIRS}
)

target_link_directories(musica
  PUBLIC 
    ${EXTRA_LIBS}
)

target_link_libraries(musica 
  PUBLIC 
    musicacore::musicacore
    ${EXTRA_LINKS}
)

################################################################################
# Packaging