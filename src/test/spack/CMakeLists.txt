# Minimal consumer of an installed MUSICA. Shipped in the release tarball so the
# Spack recipe can run `spack test run musica` against the installed prefix.
#
# The same sources are consumed twice: once through find_package (here) and once
# through pkg-config (pkgconfig_test.sh).
#
#   cmake -S . -B build -DCMAKE_PREFIX_PATH=<musica prefix> [-DTEST_FORTRAN=ON]
#   cmake --build build && ctest --test-dir build --output-on-failure
cmake_minimum_required(VERSION 3.21)
project(musica_spack_test LANGUAGES C CXX)

option(TEST_FORTRAN "Also build and test the Fortran interface" OFF)
option(TEST_MICM "Parse a mechanism and solve, instead of only checking the version" ON)
option(TEST_MIEM "Also link against MIEM, which is a separate archive pulling in netCDF-C" ON)
option(TEST_PKGCONFIG "Also consume the install through the installed .pc files" ON)

# Default resolves both in the source tree and in the Spack test cache, which
# keeps the same layout. Override when the configs are somewhere else.
set(TEST_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/../../../configs/v1/chapman/config.json"
    CACHE FILEPATH "v1 mechanism config.json used by the MICM test")

enable_testing()

find_package(musica REQUIRED)

add_executable(test_cxx main.cpp)
target_link_libraries(test_cxx PRIVATE musica::musica)
if(TEST_MIEM)
  target_compile_definitions(test_cxx PRIVATE MUSICA_TEST_MIEM)
endif()
add_test(NAME musica_cxx COMMAND test_cxx)

if(TEST_FORTRAN)
  enable_language(Fortran)
  # musica-fortran is its own package, matching the musica.pc / musica-fortran.pc
  # split. It pulls in musica::musica itself.
  find_package(musica-fortran REQUIRED)
  add_executable(test_fortran main.F90)
  target_link_libraries(test_fortran PRIVATE musica::musica-fortran)

  if(TEST_MICM)
    if(NOT EXISTS "${TEST_CONFIG}")
      message(FATAL_ERROR "TEST_MICM is ON but TEST_CONFIG does not exist: ${TEST_CONFIG}")
    endif()
    # main.F90 guards the MICM section with #ifdef, so it needs the preprocessor
    set_target_properties(test_fortran PROPERTIES Fortran_PREPROCESS ON)
    target_compile_definitions(test_fortran PRIVATE MUSICA_TEST_MICM)
    add_test(NAME musica_fortran COMMAND test_fortran "${TEST_CONFIG}")
  else()
    add_test(NAME musica_fortran COMMAND test_fortran)
  endif()
endif()

if(TEST_PKGCONFIG)
  find_program(BASH_PROGRAM bash REQUIRED)
  find_program(PKG_CONFIG_PROGRAM pkg-config REQUIRED)

  # musica_DIR is <prefix>/lib/cmake/musica; the .pc files sit in <prefix>/lib/pkgconfig
  get_filename_component(MUSICA_PREFIX "${musica_DIR}/../../.." ABSOLUTE)

  set(pkgconfig_args --prefix "${MUSICA_PREFIX}" --config "${TEST_CONFIG}")
  if(TEST_FORTRAN)
    list(APPEND pkgconfig_args --fortran)
  endif()
  if(NOT TEST_MICM)
    list(APPEND pkgconfig_args --no-micm)
  endif()
  if(NOT TEST_MIEM)
    list(APPEND pkgconfig_args --no-miem)
  endif()

  add_test(NAME musica_pkgconfig
           COMMAND ${BASH_PROGRAM} "${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig_test.sh"
                   ${pkgconfig_args})
  # The script shells out to the compilers directly, so hand it the same ones
  # CMake is using rather than letting it fall back to c++/gfortran.
  set(pkgconfig_env "CXX=${CMAKE_CXX_COMPILER}")
  if(TEST_FORTRAN)
    list(APPEND pkgconfig_env "FC=${CMAKE_Fortran_COMPILER}")
  endif()
  set_tests_properties(musica_pkgconfig PROPERTIES ENVIRONMENT "${pkgconfig_env}")
endif()
