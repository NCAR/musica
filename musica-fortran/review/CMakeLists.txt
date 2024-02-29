#----------------------------------------------------------------
# Build the following project through '/docker/Dockerfile.review'
#----------------------------------------------------------------
cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  LANGUAGES Fortran C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FortranCInterface)
FortranCInterface_VERIFY(CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

find_package(musica 0.5.0 REQUIRED)

add_library(musica-fortran SHARED)

target_sources(musica-fortran
  PUBLIC
    micm_core.F90
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

#----------------------------------------------------------------
# Add tests
#----------------------------------------------------------------
enable_testing()
include(CTest)

add_executable(test_micm_api test_call_micm_c_api.F90 test_micm_c_api.c)

target_link_libraries(test_micm_api
  PRIVATE
    musica-fortran
)
add_test(
  NAME test_micm_api
  COMMAND $<TARGET_FILE:test_micm_api>
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)