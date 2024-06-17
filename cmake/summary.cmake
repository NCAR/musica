# Parse out a few individual variables
string(TIMESTAMP BUILD_CONFIG_DATE "%Y-%m-%d %H:%M:%S")
get_filename_component(C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)
set(C_COMPILER_NAME "${C_COMPILER_NAME} (${CMAKE_C_COMPILER_VERSION})")

if(MUSICA_BUILD_C_CXX_INTERFACE)
  get_filename_component(CXX_COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)
  set(CXX_COMPILER_NAME "${CXX_COMPILER_NAME} (${CMAKE_CXX_COMPILER_VERSION})")
else()
  set(CXX_COMPILER_NAME " ")
endif()

if(MUSICA_BUILD_FORTRAN_INTERFACE)
  get_filename_component(Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)
else()
  set(Fortran_COMPILER_NAME " ")
endif()

if(MUSICA_ENABLE_MICM)
  set(MUSICA_ENABLE_MICM_INFO "ON (repo: ${MICM_GIT_REPOSITORY} tag: ${MICM_GIT_TAG})")
else()
  set(MUSICA_ENABLE_MICM_INFO "OFF")
endif()

if(MUSICA_ENABLE_TUVX)
  set(MUSICA_ENABLE_TUVX_INFO "ON (repo: ${TUVX_GIT_REPOSITORY} tag: ${TUVX_GIT_TAG})")
else()
  set(MUSICA_ENABLE_TUVX_INFO "OFF")
endif()

# Configure musica.settings file
configure_file("${CMAKE_SOURCE_DIR}/cmake/musica.settings.in"
  "${CMAKE_BINARY_DIR}/musica.settings"
  @ONLY)

# Read in settings file, print out.
# Avoid using system-specific calls so that this
# might also work on Windows.
file(READ "${CMAKE_BINARY_DIR}/musica.settings"
  MUSICA_SETTINGS)
message(STATUS ${MUSICA_SETTINGS})