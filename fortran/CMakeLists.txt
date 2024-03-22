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
  PUBLIC
    micm_core.F90
)

####################
# TUV-x
if (MUSICA_ENABLE_TUVX)
  set(TUVX_MOD_DIR ${MUSICA_MOD_DIR})
  set(TUVX_LIB_DIR ${MUSICA_LIB_DIR})
 
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/tuv-x/src ${MUSICA_LIB_DIR}/tuv-x/src)

  # for yaml-cpp
  target_compile_features(tuvx_object PRIVATE cxx_std_20)
  
  set_target_properties(tuvx_object PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  )

  target_include_directories(tuvx_object
    PUBLIC  
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

  target_link_libraries(tuvx_object 
    PUBLIC
      musicacore_object
  )

  target_link_libraries(musica-fortran 
    PUBLIC
      yaml-cpp::yaml-cpp
  )

  # add the sources to musica
  target_sources(musica-fortran
    PRIVATE
      $<TARGET_OBJECTS:tuvx_object>
  )
endif()


################################################################################
# testing

if(MUSICA_ENABLE_TESTS)
  add_subdirectory(test)
endif()