cmake_minimum_required(VERSION 3.21)

project(
  musica-demo
  VERSION 0.1
)

option(TEST_OPTION "test option" ON)

set(MUSICA_INCLUDE_DIR "${MUSICA_INSTALL_DIR}/include")
set(MUSICA_LIB_DIR "${MUSICA_INSTALL_DIR}/lib")

message(STATUS "${MUSICA_INCLUDE_DIR}")
message(STATUS "${MUSICA_LIB_DIR}")

include("${MUSICA_LIB_DIR}/cmake/musica/musicaConfig.cmake")
include("${MUSICA_LIB_DIR}/cmake/musica/musicaConfigVersion.cmake")
include("${MUSICA_LIB_DIR}/cmake/musica/musica_Exports.cmake")
include("${MUSICA_LIB_DIR}/cmake/musica/musica_Exports-release.cmake")
# include("${MUSICA_LIB_DIR}/cmake/musica/musica_fortranConfig.cmake")
# include("${MUSICA_LIB_DIR}/cmake/musica/musica_fortranConfigVersion.cmake")
# include("${MUSICA_LIB_DIR}/cmake/musica/musica_fortran_Exports.cmake")
# include("${MUSICA_LIB_DIR}/cmake/musica/musica_fortran_Exports-release.cmake")

add_executable(demo_c demo.c)
target_include_directories(demo_c PUBLIC "${MUSICA_INCLUDE_DIR}")
