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

# include(musica_util)
# checkout_submodules()

################################################################################
# Projet wide setup options
option( ENABLE_TUVX "Builds TUV-x, a photolysis calculator library" ON )

# Set up include and lib directories
set( MUSICA_MOD_DIR "${PROJECT_BINARY_DIR}/include" )
set( MUSICA_LIB_DIR "${PROJECT_BINARY_DIR}/libs" )

include(GNUInstallDirs)
set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/ncar/musica-${PROJECT_VERSION}")
set(INSTALL_MOD_DIR "${INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")

################################################################################
# Dependencies

include(cmake/dependencies.cmake)

##############################################################################
# MUSICA targets
##############################################################################

# MUSICA
add_library(musica STATIC)

set_target_properties(musica PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_directories(musica
  PUBLIC 
    ${EXTRA_LIBS}
)

target_link_libraries(musica 
  PUBLIC
    PkgConfig::netcdff
)

target_include_directories(musica
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
)


# MUSICA-core
add_library(musicacore_object)
add_subdirectory(lib/musica-core/src)

set_target_properties(musicacore_object PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
)

target_include_directories(musicacore_object 
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
)

target_sources(musica
  PUBLIC
    $<TARGET_OBJECTS:musicacore_object>
)

# TUV-x
if (ENABLE_TUVX)
  add_library(tuvx_object)
  add_subdirectory(lib/tuv-x/src)

  set_target_properties(tuvx_object PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  )

  target_include_directories(tuvx_object 
    PUBLIC  
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${INSTALL_MOD_DIR}>
  )

  target_sources(musica
    PUBLIC
      $<TARGET_OBJECTS:tuvx_object>
  )
endif()

################################################################################
# Packaging

# only include packaging if we are the top level project being built
if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory(packaging)
endif()