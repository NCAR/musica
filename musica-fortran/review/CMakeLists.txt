cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran_wrapper
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(MAKE_SHARED_LIBRARY OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

#--------------------------------
# Reference to 'Dockerfile.review'
#--------------------------------
if(MAKE_SHARED_LIBRARY)
  add_library(musica-fortran SHARED micm_core.F90)
  target_link_libraries(musica-fortran ${CMAKE_CURRENT_LIST_DIR}/libmusica.so)
else()
  message(STATUS "Build static library")
  add_library(musica-fortran STATIC micm_core.F90)
  target_link_libraries(musica-fortran ${CMAKE_CURRENT_LIST_DIR}/libmusica.a)
endif()

enable_testing()
include(CTest)
add_executable(test_musica_api test_musica_api.F90)
target_link_libraries(test_musica_api
  PUBLIC
    musica-fortran
)
add_test(
  NAME test_musica_api 
  COMMAND $<TARGET_FILE:test_musica_api>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)