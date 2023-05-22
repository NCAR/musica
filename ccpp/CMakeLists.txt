####################
# CCPP library

add_library(ccpp STATIC)
add_library(musica::ccpp ALIAS ccpp)

# Overwrite the init values choosen by CMake
if (CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
  set(CMAKE_Fortran_FLAGS "-fpp")
endif()

# Set up include and lib directories
set(CCPP_MOD_DIR "${PROJECT_BINARY_DIR}/include/ccpp" )
set(CCPP_LIB_DIR "${PROJECT_BINARY_DIR}/libs/ccpp" )

set(CCPP_INSTALL_PREFIX "musica-ccpp-${PROJECT_VERSION}")
set(CCPP_INSTALL_MOD_DIR "${INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")

set_target_properties(ccpp PROPERTIES
  Fortran_MODULE_DIRECTORY ${CCPP_MOD_DIR}
)

set_target_properties(ccpp PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CCPP_LIB_DIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(ccpp
  PUBLIC  
    $<BUILD_INTERFACE:${CCPP_MOD_DIR}>
    $<INSTALL_INTERFACE:${CCPP_INSTALL_MOD_DIR}>
)


####################
# micm ccpp api

add_subdirectory(micm)