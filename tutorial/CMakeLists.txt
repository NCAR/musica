cmake_minimum_required(VERSION 3.21)

project(
  musica-demo
  VERSION 0.1
)

option(TEST_OPTION "test option" ON)

set(MUSICA_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../include")
set(MUSICA_LIBRARY_DIR "${PROJECT_SOURCE_DIR}")

message(STATUS "${MUSICA_INCLUDE_DIR}")
message(STATUS "${MUSICA_LIBRARY_DIR}")

add_executable(demo demo.cpp)
target_include_directories(demo PUBLIC "${MUSICA_INCLUDE_DIR}")
