cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  VERSION ${PROJECT_VERSION}
  LANGUAGES Fortran
)

message (STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

if(NOT TARGET musica)
  find_package(musica REQUIRED)
endif()

add_library(musica-fortran)
add_library(musica::musica-fortran ALIAS musica-fortran)

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
  INTERFACE
    musica::musica
)

target_sources(musica-fortran
  PRIVATE
    micm_core.F90
    util.F90
)

# Add flags for gfortran
if(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
  add_compile_options($<$<COMPILE_LANGUAGE:Fortran>:-ffree-line-length-none>)
endif()

####################
# TUV-x
if (MUSICA_ENABLE_TUVX)
  # add the sources to musica
  target_sources(musica-fortran
    PRIVATE
      $<TARGET_OBJECTS:tuvx_object>
  )
  target_link_libraries(musica-fortran
    PUBLIC
      tuvx
  )
endif()


################################################################################
# testing

if(MUSICA_ENABLE_TESTS)
  add_subdirectory(test)
endif()