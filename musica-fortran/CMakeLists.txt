cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  LANGUAGES Fortran C CXX
)

#TODO(jiwon) This option can be temporary (depends on how CAM-SIMA grabs the source vs. lib)
option(PROVIDE_MICM_FORT_SRC "Provide MICM Fortran F90 files" ON)
message(STATUS "Providing MICM Fortran F90 files : ${PROVIDE_MUSICA_FORTRAN_SRC}")

include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(MUSICA_FORTRAN_MOD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set(MUSICA_FORTRAN_LIB_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(INSTALL_INCLUDE_DIR ${CMAKE_PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_LIB_DIR ${CMAKE_PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR})

include(FortranCInterface)
FortranCInterface_VERIFY(CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

find_package(musica 0.5.0 REQUIRED)

add_library(musica-fortran SHARED)
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
  PRIVATE
    musica
)
target_include_directories(musica-fortran
  PUBLIC
    $ENV{MUSICA_INCLUDE}  # set by dockerfile
)
target_compile_features(musica-fortran PUBLIC cxx_std_20)

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

if(PROVIDE_MICM_FORT_SRC)
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/src/micm/
    DESTINATION
      ${CMAKE_PROJECT_NAME}/src/
    FILES_MATCHING PATTERN "*.F90"
  )
endif()