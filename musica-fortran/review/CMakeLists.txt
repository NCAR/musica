cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FortranCInterface)
FortranCInterface_VERIFY(CXX)

find_package(musica 0.5.0 REQUIRED)

#--------------------------------
# Reference to '/docker/Dockerfile.review'
# for $ENV{MUSICA_INCLUDE}
#--------------------------------
add_library(musica-fortran SHARED)

target_sources(musica-fortran
  PUBLIC
    micm_core.F90
  PRIVATE
    micm_c_api.cpp
    micm.cpp
)
target_link_libraries(musica-fortran
  PRIVATE
    ${musica_LIBRARIES}
)
target_include_directories(musica-fortran
  PUBLIC
    $ENV{MUSICA_INCLUDE}
)
target_compile_features(musica-fortran PUBLIC cxx_std_20)

enable_testing()
include(CTest)

add_executable(test_musica_api test_musica_api.F90)

target_link_libraries(test_musica_api
  PRIVATE
    musica-fortran
)
add_test(
  NAME test_musica_api 
  COMMAND $<TARGET_FILE:test_musica_api>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)