cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  VERSION 0.0.0
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(MUSICA_FORTRAN_MOD_DIR ${PROJECT_BINARY_DIR}/include)
set(MUSICA_FORTRAN_LIB_DIR ${PROJECT_BINARY_DIR}/libs)

# ------------------ TEST -----------------
include(GNUInstallDirs)

set(INSTALL_PREFIX ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
set(INSTALL_INCLUDE_DIR ${INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_LIB_DIR ${INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
# ------------------ TEST -----------------

add_library(musica-fortran STATIC)
add_library(musica::musica-fortran ALIAS musica-fortran)

set_target_properties(musica-fortran
PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_FORTRAN_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_FORTRAN_MOD_DIR}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

find_package(musica 0.5.0 REQUIRED)

target_link_libraries(musica-fortran
 PUBLIC 
  musica::musica)

target_include_directories(musica-fortran
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_FORTRAN_LIB_DIR}>
    $<INSTALL_INTERFACE:${MUSICA_FORTRAN_LIB_DIR}>
)

# ------------------ TEST -----------------
install(TARGETS musica-fortran
  DESTINATION
    lib
)
# ------------------ TEST -----------------

add_subdirectory(src)