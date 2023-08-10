cmake_minimum_required(VERSION 3.21)

project(
  musica
  VERSION 0.3.0
  LANGUAGES Fortran CXX C
)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_MODULE_PATH}/SetDefaults.cmake")

# Library options to build
option(USE_MUSICA "Use MUSICA" OFF)
message(STATUS "Building MUSICA sources into a library : ${USE_MUSICA}")

option(USE_MUSICA_FORTRAN "Use MUSICA Fortran interface" OFF)
message(STATUS "Building MUSICA-Fortran sources into a library : ${USE_MUSICA_FORTRAN}")

if(USE_MUSICA)
  add_subdirectory(musica)
endif()

if(USE_MUSICA_FORTRAN)
  add_subdirectory(musica-fortran)
endif()