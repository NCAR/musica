cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran_wrapper
  VERSION 0.0.0
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(musica 0.3.0 REQUIRED)

################################################################################
# Tests
add_executable(test_musica_api test_musica_api.F90)

target_link_libraries(test_musica_api 
  PUBLIC 
    musica::musica
    musica::musica-fortran
)
enable_testing()

add_test(
  NAME test_musica_api 
  COMMAND $<TARGET_FILE:test_musica_api>
  )