cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran
  LANGUAGES Fortran C CXX
)

option(USE_INTEL_COMPILERS "Use the Intel compilers" OFF)
message(STATUS "Use the Intel compilers : ${USE_INTEL_COMPILERS}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# include(FetchContent)

# FetchContent_Declare(musica-fortran
#   GIT_REPOSITORY https://github.com/NCAR/musica.git
#   GIT_TAG        61-add-fort-compilers
# )

# set(USE_MUSICA OFF)
# set(USE_MUSICA_FORTRAN ON)
# set(ENABLE_MICM ON)
# set(ENABLE_TUVX OFF)
# set(ENABLE_TESTS OFF)

# FetchContent_MakeAvailable(musica-fortran)

include(FortranCInterface)
FortranCInterface_VERIFY(CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

find_package(musica 0.5.0 REQUIRED)

enable_testing()
include(CTest)

# API Test
if(USE_INTEL_COMPILERS)
  add_executable(test_micm_fort_api test_call_micm_c_api.F90 test_micm_c_api.c)
else()
  add_executable(test_micm_fort_api test_micm_fort_api.F90)
endif()

target_link_libraries(test_micm_fort_api
  PUBLIC
    $ENV{MUSICA_FORTRAN_LIB}  # set by docker
  PRIVATE
    musica
)
target_include_directories(test_micm_fort_api
  PUBLIC
    $ENV{MUSICA_FORTRAN_INCLUDE}  # set by docker
  PRIVATE
    $ENV{MUSICA_INCLUDE}  # set by docker
)
set_target_properties(test_micm_fort_api PROPERTIES
  LINKER_LANGUAGE CXX
)
add_test(
  NAME test_micm_fort_api
  COMMAND $<TARGET_FILE:test_micm_fort_api>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# API Test (invalid configuration path)
if(NOT USE_INTEL_COMPILERS)
  add_executable(test_micm_fort_api_invalid test_micm_fort_api_invalid.F90)

  target_link_libraries(test_micm_fort_api_invalid
    PUBLIC
      $ENV{MUSICA_FORTRAN_LIB}  # set by docker
    PRIVATE
      musica
  )
  target_include_directories(test_micm_fort_api_invalid
    PUBLIC
      $ENV{MUSICA_FORTRAN_INCLUDE}  # set by docker
    PRIVATE
      $ENV{MUSICA_INCLUDE}  # set by docker
  )
  set_target_properties(test_micm_fort_api_invalid PROPERTIES
    LINKER_LANGUAGE CXX
  )
  add_test(
    NAME test_micm_fort_api_invalid
    COMMAND $<TARGET_FILE:test_micm_fort_api_invalid>
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
endif()