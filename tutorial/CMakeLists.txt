cmake_minimum_required(VERSION 3.21)

project(
  musica-demo
  VERSION 0.1
  LANGUAGES CXX C Fortran
)

option(TEST_OPTION "test option" ON)

# cmake -DMUSICA_INSTALL_DIR <install_dir> -Dmusica_DIR <package_dir>
find_package(musica)

# set(MUSICA_INCLUDE_DIR ${musica_INCLUDE_DIRS})

set(MUSICA_INCLUDE_DIR "${MUSICA_INSTALL_DIR}/include")
set(MUSICA_LIB_DIR "${MUSICA_INSTALL_DIR}/lib")

message(STATUS "${MUSICA_INCLUDE_DIR}")
message(STATUS "${MUSICA_LIB_DIR}")

add_executable(demo_c demo.c)
add_executable(demo_f demo.f90)

target_include_directories(demo_c PUBLIC ${MUSICA_INCLUDE_DIR})
target_link_directories(demo_c PUBLIC ${MUSICA_LIB_DIR})

target_include_directories(demo_f PUBLIC ${MUSICA_INCLUDE_DIR})
target_link_directories(demo_f PUBLIC ${MUSICA_LIB_DIR})
target_link_libraries(demo_f musica-fortran musica stdc++)
