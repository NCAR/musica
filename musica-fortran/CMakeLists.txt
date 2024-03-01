cmake_minimum_required(VERSION 3.21)

project(
  musica-fortran
  VERSION ${PROJECT_VERSION}
  LANGUAGES Fortran
)

include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(MUSICA_FORTRAN_MOD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
set(MUSICA_FORTRAN_LIB_DIR ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(INSTALL_INCLUDE_DIR ${CMAKE_PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_LIB_DIR ${CMAKE_PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR})

option(MAKE_MUSICA_FORTRAN_INSTALLABLE ON)

# add_library(musica STATIC IMPORTED)
# set_target_properties(musica 
#   PROPERTIES
#     IMPORTED_LOCATION $ENV{MUSICA_LIB_HOME}
#     INTERFACE_INCLUDE_DIRECTORIES $ENV{MUSICA_INTERFACE}
# )
# find_library(MUSICA_LIBRARY
#     NAMES curl curllib libcurl_imp curllib_static
#     HINTS "${CMAKE_PREFIX_PATH}/curl/lib"
# )

add_library(musica-fortran STATIC)
add_library(musica::musica-fortran ALIAS musica-fortran)
add_subdirectory(src)
target_link_libraries(musica-fortran
  PUBLIC
    # musica
    $ENV{MUSICA_LIB_HOME}
)

set_target_properties(musica-fortran
  PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${MUSICA_FORTRAN_LIB_DIR}
    Fortran_MODULE_DIRECTORY ${MUSICA_FORTRAN_MOD_DIR}
    # LINKER_LANGUAGE CXX
)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)

# find_package(musica 0.5.0 REQUIRED)
# add_library(musica STATIC IMPORTED)
# set_target_properties(musica 
#   PROPERTIES
#     IMPORTED_LOCATION $ENV{MUSICA_LIB_HOME}
#     INTERFACE_INCLUDE_DIRECTORIES $ENV{MUSICA_INTERFACE}
# )

target_include_directories(musica-fortran
  PUBLIC  
    $<BUILD_INTERFACE:${MUSICA_FORTRAN_MOD_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)

# target_sources(musica-fortran
#   PUBLIC
#     ${CMAKE_SOURCE_DIR}/../musica/src/micm/micm_c_api.cpp
#     ${CMAKE_SOURCE_DIR}/../musica/src/micm/micm.cpp

# )
# target_include_directories(musica-fortran
#   PUBLIC  
#     $<BUILD_INTERFACE:${MUSICA_FORTRAN_MOD_DIR}>
#     $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
# )

# target_include_directories(musica-fortran
#   PUBLIC  
#     $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/micm/>
#     $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
# )

# install(
#   ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
#   INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR}
# )

# install(TARGETS ${PROJECT_NAME}
install(
  DIRECTORY 
    ${MUSICA_FORTRAN_LIB_DIR}/
  DESTINATION 
    ${INSTALL_LIB_DIR}
)

install(
  DIRECTORY 
    ${MUSICA_FORTRAN_MOD_DIR}/
  DESTINATION 
    ${INSTALL_INCLUDE_DIR}
  FILES_MATCHING PATTERN "*.mod"
)

# if(MAKE_MUSICA_FORTRAN_INSTALLABLE)
#   install(
#     DIRECTORY 
#       ${CMAKE_SOURCE_DIR}/src/micm/
#     DESTINATION 
#       # ${CMAKE_PROJECT_NAME}/include/
#       ${INSTALL_INCLUDE_DIR}
#     FILES_MATCHING PATTERN "*.F90"
#   )
# endif()