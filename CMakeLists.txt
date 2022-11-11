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

set( LIB_DIR "libs")

set( MUSICALIB_MOD_DIR "${PROJECT_BINARY_DIR}/include" )
set( MUSICALIB_LIB_DIR "${PROJECT_BINARY_DIR}/${LIB_DIR}" )

message( STATUS "musica mod directory: ${MUSICALIB_MOD_DIR}" )
message( STATUS "musica lib directory: ${MUSICALIB_LIB_DIR}" )


##############################################################################
# MUSICA-core
add_subdirectory( ${LIB_DIR}/musica-core/src )
list( APPEND EXTRA_LIB_DIRS "${LIB_DIR}/musica-core/src" )
list( APPEND EXTRA_INCLUDE_DIRS "${LIB_DIR}/musica-core" )
list( APPEND EXTRA_LINKS musicalib )


##############################################################################
# Libraries
##############################################################################
# TUV-x
if ( ENABLE_TUVX )
  add_subdirectory( ${MUSICALIB_LIB_DIR}/tuv-x )
  list( APPEND EXTRA_LIB_DIRS "${LIB_DIR}/tuv-x" )
  list( APPEND EXTRA_INCLUDE_DIRS "${LIB_DIR}/tuv-x" )
  list( APPEND EXTRA_LINKS tuvxlib )
endif ()

##############################################################################
# The product
##############################################################################
add_executable( ${PROJECT_NAME} main.F90 )

target_include_directories( ${PROJECT_NAME}
  PUBLIC ${EXTRA_INCLUDE_DIRS}
)

target_link_directories( ${PROJECT_NAME}
  PUBLIC ${EXTRA_LIBS}
)

target_link_libraries(
 ${PROJECT_NAME} PUBLIC ${EXTRA_LINKS}
)