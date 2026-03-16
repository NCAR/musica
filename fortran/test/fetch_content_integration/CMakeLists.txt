cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

set(MUSICA_GIT_TAG "main" CACHE STRING "Git tag for the musica_fortran repository")

message(STATUS "Using MUSICA_GIT_TAG: ${MUSICA_GIT_TAG}")

FetchContent_Declare(musica_fortran
  GIT_REPOSITORY https://github.com/NCAR/musica.git
  GIT_TAG        ${MUSICA_GIT_TAG}
)

set(MUSICA_BUNDLE_DEPENDENCIES OFF)
set(MUSICA_BUILD_C_CXX_INTERFACE OFF)
set(MUSICA_BUILD_FORTRAN_INTERFACE ON)
set(MUSICA_ENABLE_TESTS OFF)
set(MUSICA_ENABLE_INSTALL OFF)
set(MUSICA_ENABLE_MICM ON)
set(MUSICA_ENABLE_TUVX ON)
set(MUSICA_ENABLE_CARMA OFF)

FetchContent_MakeAvailable(musica_fortran)

find_package(musica REQUIRED)

enable_testing()

function(add_fetch_content_test)
  set(prefix TEST)
  set(singleValues NAME)
  set(multiValues SOURCES)
  include(CMakeParseArguments)
  cmake_parse_arguments(${prefix} " " "${singleValues}" "${multiValues}" ${ARGN})

  add_executable(${TEST_NAME} ${TEST_SOURCES})
  target_link_libraries(${TEST_NAME} musica-fortran musica::musica)

  add_test(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:${TEST_NAME}>
      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )

  if (CMAKE_Fortran_COMPILER_ID STREQUAL "IntelLLVM" OR CMAKE_Fortran_COMPILER_ID STREQUAL "NVHPC")
      set_target_properties(${TEST_NAME} PROPERTIES LINKER_LANGUAGE Fortran)
  endif()
endfunction()

add_fetch_content_test(NAME test_musica SOURCES ../tutorial/demo.F90)
add_fetch_content_test(NAME test_micm_api SOURCES ../integration/test_micm_api.F90)
add_fetch_content_test(NAME test_micm_box_model SOURCES ../integration/test_micm_box_model.F90)
add_fetch_content_test(NAME test_micm_multiple_grid_cells SOURCES ../integration/test_micm_multiple_grid_cells.F90)
add_fetch_content_test(NAME test_tuvx_api SOURCES ../integration/test_tuvx_api.F90)
