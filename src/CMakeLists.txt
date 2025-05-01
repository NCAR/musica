################################################################################
# Preamble

project(
  musica
  VERSION ${PROJECT_VERSION}
  LANGUAGES C CXX
)

message (STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

##############################################################################
# MUSICA targets

####################
# MUSICA
add_library(musica)
add_library(musica::musica ALIAS musica)

if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None" AND NOT APPLE)
  # Add the CUDA library
  target_link_libraries(musica PUBLIC musica::micm_cuda)
  set_target_properties(micm_cuda PROPERTIES POSITION_INDEPENDENT_CODE ON)
  list(APPEND musica_compile_definitions MUSICA_ENABLE_CUDA)
endif()

if (MUSICA_ENABLE_PYTHON_LIBRARY OR MUSICA_ENABLE_PIC)
  # Add the -fPIC flag for position-independent code if we are building shared libraries
  # or the python library so that it links correctly on linux
  set_target_properties(musica PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

include(silence_warnings)
silence_warnings(musica)

# set the c++ standard for musica
target_compile_features(musica PUBLIC cxx_std_20)

if(MUSICA_ENABLE_MICM)
  list(APPEND musica_compile_definitions -DMUSICA_USE_MICM)
endif()

if(MUSICA_ENABLE_MPI)
  list(APPEND musica_compile_definitions -DMUSICA_USE_MPI)
endif()

if(MUSICA_ENABLE_OPENMP)
  list(APPEND musica_compile_definitions -DMUSICA_USE_OPENMP)
endif()

# version
configure_file(version.cpp.in ${CMAKE_BINARY_DIR}/version.cpp @ONLY)

target_sources(musica
  PRIVATE
    util.cpp
    component_versions.cpp
    ${CMAKE_BINARY_DIR}/version.cpp
)

target_link_libraries(musica
  PUBLIC
    musica::mechanism_configuration
)

target_compile_definitions(musica PUBLIC ${musica_compile_definitions})

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

target_include_directories(musica
  PUBLIC
    $<BUILD_INTERFACE:${MUSICA_PROJECT_SRC_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

####################
# MICM
if(MUSICA_ENABLE_MICM)
  target_link_libraries(musica PUBLIC musica::micm)

  add_subdirectory(micm)
endif()

####################
# TUV-x
if(MUSICA_ENABLE_TUVX)
  enable_language(Fortran)

  # Include the sources directly into musica
  target_sources(musica
    PRIVATE
      $<TARGET_OBJECTS:tuvx_object>
  )

  target_link_libraries(musica
    PUBLIC
      tuvx_object
  )

  target_include_directories(musica
    PUBLIC
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

  add_subdirectory(tuvx)
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
