# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# CMakeLists.txt for the CUDA plugin library (libmusica_cuda.so)
# This is only built on Linux when GPU support is enabled.

add_library(musica_cuda SHARED
  cuda_factory.cpp
  cuda_solver.cpp
)

# This library requires CUDA support
target_compile_definitions(musica_cuda PRIVATE MUSICA_ENABLE_CUDA)

# Include directories
target_include_directories(musica_cuda
  PUBLIC
    $<BUILD_INTERFACE:${MUSICA_PROJECT_SRC_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

# Link against MICM (CPU and CUDA) and mechanism_configuration
target_link_libraries(musica_cuda
  PRIVATE
    musica::micm
    musica::micm_cuda
    musica::mechanism_configuration
)

# C++20 standard
target_compile_features(musica_cuda PUBLIC cxx_std_20)

# Position independent code
set_target_properties(musica_cuda PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Set the rpath for the shared library
set_target_properties(musica_cuda PROPERTIES
  INSTALL_RPATH "$ORIGIN"
  BUILD_WITH_INSTALL_RPATH TRUE
)

# Install the CUDA plugin library
install(TARGETS musica_cuda
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
