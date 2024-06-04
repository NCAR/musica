cmake_minimum_required(VERSION 3.21)

project(
  test_musica_fortran
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

set(MUSICA_GIT_TAG "801807d" CACHE STRING "Git tag for the musica_fortran repository")

FetchContent_Declare(musica_fortran
  GIT_REPOSITORY https://github.com/NCAR/musica.git
  GIT_TAG        ${MUSICA_GIT_TAG}
)

set(MUSICA_BUILD_C_CXX_INTERFACE OFF)
set(MUSICA_BUILD_FORTRAN_INTERFACE ON)
set(MUSICA_ENABLE_TESTS OFF)
set(MUSICA_ENABLE_INSTALL OFF)

FetchContent_MakeAvailable(musica_fortran)

# copy config data from musica-fortran
add_custom_target(copy_unit_test_configs ALL ${CMAKE_COMMAND} -E copy_directory
  ${musica_fortran_SOURCE_DIR}/configs ${CMAKE_BINARY_DIR}/configs)

enable_testing()

# API Test
if (MUSICA_ENABLE_MICM)
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
endif()

# API Test
if (MUSICA_ENABLE_TUVX)
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
endif()