cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  VERSION ${PROJECT_VERSION}
  LANGUAGES Fortran
)

message (STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")
message (STATUS "  MICM: ${MUSICA_ENABLE_MICM}")
message (STATUS "  TUV-x: ${MUSICA_ENABLE_TUVX}")
message (STATUS "  Tests: ${MUSICA_ENABLE_TESTS}")
message (STATUS "  Install: ${MUSICA_ENABLE_INSTALL}")
message (STATUS "  C/C++ Interface: ${MUSICA_BUILD_C_CXX_INTERFACE}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)
pkg_check_modules(netcdfc IMPORTED_TARGET REQUIRED netcdf)

if(NOT TARGET musica)
  find_package(musica REQUIRED)
endif()

add_library(musica-fortran)
add_library(musica::musica-fortran ALIAS musica-fortran)

if(NOT BUILD_SHAREDS_LIBS)
  set_target_properties(musica-fortran
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
  )
endif()

set_target_properties(musica-fortran
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(musica-fortran
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)

target_link_libraries(musica-fortran
  PUBLIC
    musica::musica
)

target_sources(musica-fortran
  PRIVATE
    util.F90
)
if (MUSICA_ENABLE_MICM)
  target_sources(musica-fortran
    PRIVATE
      micm.F90
  )
endif()
if (MUSICA_ENABLE_TUVX)
  target_sources(musica-fortran
    PRIVATE
      tuvx.F90
  )
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