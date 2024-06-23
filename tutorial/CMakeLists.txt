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

add_executable(demo demo.cpp)
target_include_directories(demo PUBLIC "${MUSICA_INCLUDE_DIR}")
