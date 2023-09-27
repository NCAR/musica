cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran_wrapper
  VERSION 0.0.0
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

set(USE_MUSICA OFF)
set(USE_MUSICA_FORTRAN ON)
set(ENABLE_TUVX OFF)

FetchContent_Declare(musica 
  GIT_REPOSITORY https://github.com/NCAR/musica.git
  GIT_TAG        bfad77f  # TODO(jiwon) - update the tag
)

FetchContent_MakeAvailable(musica)

################################################################################
# Tests
add_executable(test_musica_api test_musica_api.F90)

target_link_libraries(test_musica_api 
  PUBLIC 
    musica::musica-fortran
)

set_target_properties(test_musica_api 
PROPERTIES 
LINKER_LANGUAGE Fortran) 

enable_testing()

add_test(
  NAME test_musica_api 
  COMMAND $<TARGET_FILE:test_musica_api>
)