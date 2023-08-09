cmake_minimum_required(VERSION 3.21)

# TODO: jiwon 8/9 - how to name this project?
project(musica_build_options)

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