cmake_minimum_required( VERSION 3.23.2 )

project( MUSICALIB )
enable_language( Fortran )

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(musica_util)

checkout_submodules()

##############################################################################
# Options
##############################################################################
option( ENABLE_TUVX "Builds TUV-x, a photolysis calculator library" OFF )

##############################################################################
# Set up include and lib directories
##############################################################################

set(MUSICALIB_MOD_DIR "${PROJECT_BINARY_DIR}/include")
set(MUSICALIB_LIB_DIR "${PROJECT_BINARY_DIR}/lib")

##############################################################################
# Libraries
##############################################################################
# TUV-x
if ( ENABLE_TUVX )
  add_subdirectory( libs/tuv-x )
  list( APPEND EXTRA_LIB_DIRS "${MUSICALIB_LIB_DIR}/tuv-x" )
  list( APPEND EXTRA_INCLUDE_DIRS "${MUSICALIB_LIB_DIR}/tuv-x" )
  list( APPEND EXTRA_LINKS tuvxlib )
endif ()

##############################################################################
# MUSICA-core
if ( NOT TARGET MUSICA )
  add_subdirectory(libs/musica-core/src)
endif ()

##############################################################################
# The product
##############################################################################
add_executable( ${PROJECT_NAME} main.F90 )

target_include_directories( ${PROJECT_NAME}
  PUBLIC libs/musica-core/src
  ${EXTRA_INCLUDE_DIRS}
)

target_link_directories( ${PROJECT_NAME}
  PUBLIC libs/musica-core
  ${EXTRA_LIBS}
)

target_link_libraries(
 ${PROJECT_NAME} PUBLIC musicalib
 ${EXTRA_LINKS}
)