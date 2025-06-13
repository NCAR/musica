################################################################################
# Preamble

project(
  musica
  VERSION ${PROJECT_VERSION}
  LANGUAGES C CXX
)

message (STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

################################################################################
# Sources

include(silence_warnings)

set(MUSICA_COMMON_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/component_versions.cpp
  ${CMAKE_BINARY_DIR}/version.cpp
)

if(MUSICA_ENABLE_MICM)
  set(MUSICA_MICM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/micm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/micm_c_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/state_c_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/v0_parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/v1_parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/micm/cuda_availability.cpp
  )
endif()

if(MUSICA_ENABLE_TUVX)
  set(MUSICA_TUVX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface_grid.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface_grid_map.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface_profile.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface_profile_map.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface_radiator.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface_radiator_map.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/interface_util.F90
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/grid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/grid_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/profile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/profile_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/radiator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/radiator_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/tuvx.cpp
  )
endif()

set(MUSICA_SOURCES
  ${MUSICA_COMMON_SOURCES}
  ${MUSICA_MICM_SOURCES}
  ${MUSICA_TUVX_SOURCES}
  CACHE INTERNAL "List of sources for MUSICA"
)

# version
configure_file(version.cpp.in ${CMAKE_BINARY_DIR}/version.cpp @ONLY)

################################################################################
# MUSICA

include(setup_musica_target)

add_library(musica)
add_library(musica::musica ALIAS musica)

if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  target_sources(musica PRIVATE ${MUSICA_SOURCES})
  musica_setup_target(musica MODE GPU)
else()
  target_sources(musica PRIVATE ${MUSICA_SOURCES})
  musica_setup_target(musica MODE CPU)
endif()

set_target_properties(musica PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Set the rpath for the shared library
if(APPLE)
  set_target_properties(musica PROPERTIES
    INSTALL_RPATH "@loader_path"
  )
elseif(UNIX)
  set_target_properties(musica PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

################################################################################
# testing
if(MUSICA_ENABLE_TESTS)
  add_subdirectory(test)
endif()

################################################################################
# Packaging
if(MUSICA_ENABLE_INSTALL AND NOT MUSICA_ENABLE_PYTHON_LIBRARY)
  add_subdirectory(packaging)
endif()
