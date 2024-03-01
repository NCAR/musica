cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  VERSION ${PROJECT_VERSION}
  LANGUAGES Fortran
)

message (STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(MUSICA_FORTRAN_MOD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set(MUSICA_FORTRAN_LIB_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(INSTALL_INCLUDE_DIR ${CMAKE_PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_LIB_DIR ${CMAKE_PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

find_package(musica 0.5.0 REQUIRED)

add_library(musica-fortran)
add_library(musica::musica-fortran ALIAS musica-fortran)

add_subdirectory(src)

set_target_properties(musica-fortran
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${MUSICA_FORTRAN_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_FORTRAN_MOD_DIR}
)
target_include_directories(musica-fortran
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_FORTRAN_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)
target_link_libraries(musica-fortran
  PUBLIC
    musica
)

if(MUSICA_ENABLE_INSTALL)
  install(
    DIRECTORY
      ${MUSICA_FORTRAN_LIB_DIR}/
    DESTINATION
      ${INSTALL_LIB_DIR}
  )
  install(
    DIRECTORY 
      ${MUSICA_FORTRAN_MOD_DIR}/
    DESTINATION 
      ${INSTALL_INCLUDE_DIR}
    FILES_MATCHING PATTERN "*.mod"
  )
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/src/micm/
    DESTINATION
      ${CMAKE_PROJECT_NAME}/src/
    FILES_MATCHING PATTERN "*.F90"
  )
endif()