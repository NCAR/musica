cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  VERSION ${PROJECT_VERSION}
  LANGUAGES Fortran
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT TARGET musica)
  find_package(musica REQUIRED)
endif()

add_library(musica-fortran)
add_library(musica::musica-fortran ALIAS musica-fortran)

set_target_properties(musica-fortran
  PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(musica-fortran
  PUBLIC
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_MOD_DIR}>
)

target_link_libraries(musica-fortran
  PUBLIC
    musica::musica
)

target_sources(musica-fortran
  PRIVATE
    musica.F90
    util.F90
)

if (MUSICA_ENABLE_MICM)
  add_subdirectory(micm)
endif()

if (MUSICA_ENABLE_TUVX)
  add_subdirectory(tuvx)
endif()

# Add flags for gfortran
if(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
  target_compile_options(musica-fortran PUBLIC -ffree-line-length-none)
endif()

################################################################################
# testing

if(MUSICA_ENABLE_TESTS)
  add_subdirectory(test)
endif()

################################################################################
# installation

if(MUSICA_ENABLE_INSTALL AND NOT MUSICA_ENABLE_PYTHON_LIBRARY)
  add_subdirectory(packaging)
endif()