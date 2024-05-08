cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

FetchContent_Declare(musica_fortran
  GIT_REPOSITORY https://github.com/NCAR/musica.git
  GIT_TAG        develop-fix-build
)

set(MUSICA_BUILD_C_CXX_INTERFACE ON)
set(MUSICA_BUILD_FORTRAN_INTERFACE ON)
set(MUSICA_ENABLE_MICM ON)
set(MUSICA_ENABLE_TUVX ON)
set(MUSICA_ENABLE_TESTS OFF)
set(MUSICA_ENABLE_INSTALL OFF)

FetchContent_MakeAvailable(musica_fortran)

# copy config data from musica-fortran
add_custom_target(copy_unit_test_configs ALL ${CMAKE_COMMAND} -E copy_directory
  ${musica_fortran_SOURCE_DIR}/configs ${CMAKE_BINARY_DIR}/configs)

# copy config data from tuvx
# we know it'll be there, but we can't use tuvx_SOURCE_DIR because fetch content
# for tuvx is defined inside of the musica-fortran project so it's not avaialble at this level
add_custom_target(copy_tuvx_test_configs ALL ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_BINARY_DIR}/_deps/tuvx-src/examples ${CMAKE_BINARY_DIR}/examples)

add_custom_target(copy_tuvx_data ALL ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_BINARY_DIR}/_deps/tuvx-src/data ${CMAKE_BINARY_DIR}/data)


find_package(PkgConfig REQUIRED)
pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

enable_testing()

# API Test
add_executable(test_micm_fortran_api test_micm_api.F90)

target_link_libraries(test_micm_fortran_api
  PRIVATE
    musica::musica-fortran
)

set_target_properties(test_micm_fortran_api
  PROPERTIES
    LINKER_LANGUAGE Fortran
)

add_test(
  NAME test_micm_fortran_api
  COMMAND $<TARGET_FILE:test_micm_fortran_api>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# API Test
add_executable(test_tuvx_fortran_api test_tuvx_api.F90)

target_link_libraries(test_tuvx_fortran_api
  PRIVATE
    musica::musica-fortran
)

set_target_properties(test_tuvx_fortran_api
  PROPERTIES
    LINKER_LANGUAGE Fortran
)

add_test(
  NAME test_tuvx_fortran_api
  COMMAND $<TARGET_FILE:test_tuvx_fortran_api>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)