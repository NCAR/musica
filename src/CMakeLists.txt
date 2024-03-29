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

set(MUSICA_COMPILE_DEFINITIONS)

if(MUSICA_ENABLE_MICM)
  list(APPEND MUSICA_COMPILE_DEFINITIONS -DMUSICA_USE_MICM)
endif()

if(MUSICA_ENABLE_MPI)
  list(APPEND MUSICA_COMPILE_DEFINITIONS -DMUSICA_USE_MPI)
endif()

if(MUSICA_ENABLE_OPENMP)
  list(APPEND MUSICA_COMPILE_DEFINITIONS -DMUSICA_USE_OPENMP)
endif()

target_compile_definitions(musica PRIVATE ${MUSICA_COMPILE_DEFINITIONS})

set_target_properties(musica PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(musica
  PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

# version
configure_file(version.c.in ${CMAKE_BINARY_DIR}/version.c @ONLY)

target_sources(musica
  PRIVATE
    component_versions.c
    ${CMAKE_BINARY_DIR}/version.c
)

####################
# MICM
if(MUSICA_ENABLE_MICM)
  target_compile_features(musica PUBLIC cxx_std_20)

  target_include_directories(musica
    PUBLIC  
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/micm/include>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

  target_link_libraries(musica PRIVATE nlohmann_json::nlohmann_json)

  add_subdirectory(micm)
endif()

################################################################################
# testing

if(MUSICA_ENABLE_TESTS)
  add_subdirectory(test)
endif()

################################################################################
# Packaging
if(MUSICA_ENABLE_INSTALL)
  add_subdirectory(packaging)
endif()
