cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  VERSION 0.0.0
  LANGUAGES Fortran C CXX
)

add_library(musica-fortran STATIC)
add_library(musica::musica-fortran ALIAS musica-fortran)

# Set up include and lib directories
set(MUSICA_FORTRAN_MOD_DIR "${PROJECT_BINARY_DIR}/include")
set(MUSICA_FORTRAN_LIB_DIR "${PROJECT_BINARY_DIR}/libs")

set_target_properties(musica-fortran 
PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_FORTRAN_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_FORTRAN_MOD_DIR}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

# Find MUSICA package
find_package(musica 0.3.0 REQUIRED)

target_link_libraries(musica-fortran
 PUBLIC 
  musica::musica)

target_include_directories(musica-fortran
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_FORTRAN_LIB_DIR}>
    $<INSTALL_INTERFACE:${MUSICA_FORTRAN_LIB_DIR}>
)

add_subdirectory(src)
add_subdirectory(test)