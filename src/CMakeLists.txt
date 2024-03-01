################################################################################
# Preamble

project(
  musica
  VERSION ${PROJECT_VERSION}
  LANGUAGES C CXX Fortran
)

message (STATUS "CMake build configuration for ${PROJECT_NAME} (${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

include(musica_util)
# Add submodules
checkout_submodules()

################################################################################
# Dependencies

include(dependencies)

##############################################################################
# MUSICA targets

####################
# MUSICA
add_library(musica)
add_library(musica::musica ALIAS musica)

set(MUSICA_COMPILE_DEFINITIONS)

if(MUSICA_ENABLE_TUVX)
  list(APPEND MUSICA_COMPILE_DEFINITIONS -DMUSICA_USE_TUVX)
endif()

if(MUSICA_ENABLE_MICM)
  list(APPEND MUSICA_COMPILE_DEFINITIONS -DMUSICA_USE_MICM)
endif()

if(MUSICA_ENABLE_MPI)
  list(APPEND MUSICA_COMPILE_DEFINITIONS -DMUSICA_USE_MPI)
endif()

list(APPEND MUSICA_COMPILE_DEFINITIONS -DMUSICA_USE_OPENMP)

target_compile_definitions(musica PRIVATE ${MUSICA_COMPILE_DEFINITIONS})

set_target_properties(musica PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(musica
  PRIVATE
    PkgConfig::netcdff
    ${JSON_LIB}
)

target_include_directories(musica
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

target_include_directories(musica
  PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

###################
# MUSICA-core
add_library(musicacore_object)
add_library(musica::musicacore ALIAS musicacore_object)

set(ENABLE_UTIL_ONLY ON)

set_target_properties(musicacore_object PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
)

target_include_directories(musicacore_object
  PUBLIC
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

target_link_libraries(musicacore_object
  PRIVATE
    PkgConfig::netcdff
    ${JSON_LIB}
)

# add the sources to musica
target_sources(musica
  PRIVATE
    $<TARGET_OBJECTS:musicacore_object>
)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/musica-core/src ${MUSICA_LIB_DIR}/musica-core/src)

# version
configure_file(version.c.in ${CMAKE_BINARY_DIR}/version.c @ONLY)

target_sources(musica
  PRIVATE
    component_versions.c
    ${CMAKE_BINARY_DIR}/version.c
)

####################
# TUV-x
if (MUSICA_ENABLE_TUVX)
  set(TUVX_MOD_DIR ${MUSICA_MOD_DIR})
  set(TUVX_LIB_DIR ${MUSICA_LIB_DIR})
  
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/tuv-x/src ${MUSICA_LIB_DIR}/tuv-x/src)
  
  set_target_properties(tuvx_object PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  )

  target_include_directories(tuvx_object
    PUBLIC  
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

  target_link_libraries(tuvx_object 
    PUBLIC
      musicacore_object
  )

  # add the sources to musica
  target_sources(musica
    PRIVATE
      $<TARGET_OBJECTS:tuvx_object>
  )
endif()

####################
# MICM
if (MUSICA_ENABLE_MICM)
  target_compile_features(musica PUBLIC cxx_std_20)

  target_include_directories(musica
    PUBLIC  
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/micm/include>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

  target_link_libraries(musica PRIVATE nlohmann_json::nlohmann_json)

  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/lib/micm/include/
    DESTINATION 
      ${MUSICA_INSTALL_INCLUDE_DIR}
  )

  add_subdirectory(micm)
endif()

################################################################################
# Packaging
if (MUSICA_ENABLE_INSTALL)
  add_subdirectory(packaging)
endif()

# Tests
if(MUSICA_ENABLE_TESTS)
  add_subdirectory(test)
endif()