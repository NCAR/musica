################################################################################
# Preamble

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message (STATUS "CMake build configuration for musica(${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

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
add_library(musica STATIC)
add_library(musica::musica ALIAS musica)

add_subdirectory(src)

set_target_properties(musica PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
  Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(musica
  PRIVATE
    PkgConfig::netcdff
    ${JSON_LIB}
)

target_include_directories(musica
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)

target_include_directories(musica
  PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)

# target_include_directories(musica
#   PUBLIC
#     ${CMAKE_CURRENT_LIST_DIR}/include
    # $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    # $<INSTALL_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
# )

####################
# # MUSICA-core
# add_library(musicacore_object)
# add_library(musica::musicacore ALIAS musicacore_object)

# set(ENABLE_UTIL_ONLY ON)

# set_target_properties(musicacore_object PROPERTIES
#   ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
#   Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
# )

# target_include_directories(musicacore_object 
#   PUBLIC  
#     $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
#     $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
# )

# target_link_libraries(musicacore_object 
#   PRIVATE
#     PkgConfig::netcdff
# )

# # add the sources to musica
# target_sources(musica  
#   PRIVATE
#     $<TARGET_OBJECTS:musicacore_object>
# )

# add_subdirectory(${PROJECT_SOURCE_DIR}/lib/musica-core/src ${MUSICA_LIB_DIR}/musica-core/src)

####################
# TUV-x
if (ENABLE_TUVX)
  set(TUVX_MOD_DIR ${MUSICA_MOD_DIR})
  set(TUVX_LIB_DIR ${MUSICA_LIB_DIR})
  
  add_subdirectory(${PROJECT_SOURCE_DIR}/lib/tuv-x/src ${MUSICA_LIB_DIR}/tuv-x/src)
  
  set_target_properties(tuvx_object PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
  )

  target_include_directories(tuvx_object 
    PUBLIC  
      $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
      $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
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
if (ENABLE_MICM)
  target_compile_features(musica PUBLIC cxx_std_20)

  target_include_directories(musica
    PUBLIC  
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/micm/include>
      $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
  )
  target_include_directories(musica
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
      $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
  )

  install(
    DIRECTORY
      ${PROJECT_SOURCE_DIR}/lib/micm/include/
      ${CMAKE_CURRENT_LIST_DIR}/include/  ##TODO (jiwon)
    DESTINATION 
      ${INSTALL_INCLUDE_DIR}
  )
endif()

################################################################################
# Packaging
# add_subdirectory(packaging)

# # Tests
# if(ENABLE_TESTS)
#   add_subdirectory(test)
# endif()