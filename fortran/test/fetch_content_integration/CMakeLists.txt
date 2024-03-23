cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

FetchContent_Declare(musica-fortran
  GIT_REPOSITORY https://github.com/NCAR/musica.git
  GIT_TAG        main
)

set(MUSICA_BUILD_C_CXX_INTERFACE OFF)
set(MUSICA_BUILD_FORTRAN_INTERFACE ON)
set(MUSICA_ENABLE_MICM ON)
set(MUSICA_ENABLE_TUVX OFF)
set(MUSICA_ENABLE_TESTS OFF)

FetchContent_MakeAvailable(musica-fortran)

find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

enable_testing()

# API Test
add_executable(test_micm_fort_api test_micm_fort_api.F90)

target_link_libraries(test_micm_fort_api
  PRIVATE
    musica::musica-fortran
)

set_target_properties(test_micm_fort_api
  PROPERTIES
    LINKER_LANGUAGE Fortran
)

add_test(
  NAME test_micm_fort_api
  COMMAND $<TARGET_FILE:test_micm_fort_api>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_micm_fort_api_invalid test_micm_fort_api_invalid.F90)

target_link_libraries(test_micm_fort_api_invalid
  PRIVATE
    musica::musica-fortran
)

add_test(
  NAME test_micm_fort_api_invalid
  COMMAND $<TARGET_FILE:test_micm_fort_api_invalid>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

set_target_properties(test_micm_fort_api_invalid
  PROPERTIES
    LINKER_LANGUAGE Fortran
)