################################################################################
# Preamble

project(
  musica
  VERSION ${PROJECT_VERSION}
  LANGUAGES C CXX
)

message(STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

################################################################################
# version file
configure_file(version.cpp.in ${CMAKE_BINARY_DIR}/version.cpp @ONLY)

################################################################################
# MUSICA Library

add_library(musica)
add_library(musica::musica ALIAS musica)

# Common sources
target_sources(musica PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
  ${CMAKE_BINARY_DIR}/version.cpp
)

# MICM sources
if(MUSICA_ENABLE_MICM)
  add_subdirectory(micm)
endif()

# TUV-x sources
if(MUSICA_ENABLE_TUVX)
  add_subdirectory(tuvx)
endif()

# CARMA sources
if(MUSICA_ENABLE_CARMA)
  add_subdirectory(carma)
endif()

# MICM CUDA Solver
if(UNIX AND NOT APPLE AND NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  add_subdirectory(cuda)
endif()

################################################################################
# Configure musica target

# Compiler options
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (MSVC)
    target_compile_options(musica PUBLIC /WX)
  else()
    target_compile_options(musica PUBLIC -Werror)
  endif()
endif()

if (MUSICA_ENABLE_PYTHON_LIBRARY OR MUSICA_ENABLE_PIC)
  set_target_properties(musica PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(musica
  PUBLIC
    $<BUILD_INTERFACE:${MUSICA_PROJECT_SRC_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

target_compile_features(musica PUBLIC cxx_std_20)

target_link_libraries(musica
  PUBLIC
    musica::mechanism_configuration
)

# Compile definitions
if (MUSICA_ENABLE_MPI)
  list(APPEND musica_compile_definitions MUSICA_USE_MPI)
endif()

if (MUSICA_ENABLE_OPENMP)
  list(APPEND musica_compile_definitions MUSICA_USE_OPENMP)
endif()

# MICM dependencies
if (MUSICA_ENABLE_MICM)
  list(APPEND musica_compile_definitions MUSICA_USE_MICM)
  target_link_libraries(musica PUBLIC musica::micm)

  # Link against dl for dlopen/dlsym on Linux
  if(UNIX AND NOT APPLE)
    target_link_libraries(musica PUBLIC ${CMAKE_DL_LIBS})
  endif()
endif()

# TUV-x dependencies
if (MUSICA_ENABLE_TUVX)
  list(APPEND musica_compile_definitions MUSICA_USE_TUVX)
  target_sources(musica PRIVATE $<TARGET_OBJECTS:tuvx_object>)
  target_include_directories(musica
    PUBLIC
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_MOD_DIR}>
  )
endif()

# CARMA dependencies
if (MUSICA_ENABLE_CARMA)
  list(APPEND musica_compile_definitions MUSICA_USE_CARMA)
  target_sources(musica PRIVATE $<TARGET_OBJECTS:carma_object>)
  target_include_directories(musica
    PUBLIC
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_MOD_DIR}>
  )

  # Link CARMA dependencies
  find_package(BLAS REQUIRED)
  find_package(LAPACK REQUIRED)
  get_target_property(_carma_links carma_object INTERFACE_LINK_LIBRARIES)
  if (_carma_links)
    message(STATUS "Linking CARMA dependencies: ${_carma_links}")
    target_link_libraries(musica PUBLIC ${_carma_links})
  endif()

  if (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
    target_compile_options(musica PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-ffree-line-length-none>)
    target_compile_options(carma_object PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-ffree-line-length-none>)
  endif()
endif()

# NetCDF for CARMA or TUV-x
if (MUSICA_ENABLE_CARMA OR MUSICA_ENABLE_TUVX)
  target_link_libraries(musica PRIVATE PkgConfig::netcdfc PkgConfig::netcdff)
endif()

target_compile_definitions(musica PUBLIC ${musica_compile_definitions})

################################################################################
# Target properties

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
