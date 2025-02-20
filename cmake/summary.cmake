# Parse out a few individual variables
string(TIMESTAMP BUILD_CONFIG_DATE "%Y-%m-%d %H:%M:%S")
get_filename_component(C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)
set(C_COMPILER_NAME_INFO "${C_COMPILER_NAME} (${CMAKE_C_COMPILER_VERSION})")

if(MUSICA_BUILD_C_CXX_INTERFACE)
  get_filename_component(CXX_COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)
  set(CXX_COMPILER_NAME_INFO "${CXX_COMPILER_NAME} (${CMAKE_CXX_COMPILER_VERSION})")
else()
  set(CXX_COMPILER_NAME_INFO " ")
endif()

if(MUSICA_BUILD_FORTRAN_INTERFACE)
  get_filename_component(Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)
  set(Fortran_COMPILER_NAME_INFO "${Fortran_COMPILER_NAME}")
  if(${CMAKE_Fortran_COMPILER_VERSION})
    set(Fortran_COMPILER_NAME_INFO "${Fortran_COMPILER_NAME_INFO} (${CMAKE_Fortran_COMPILER_VERSION})")
  endif()
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

if(MUSICA_ENABLE_PYTHON_LIBRARY)
  set(MUSICA_ENABLE_PYTHON_LIBRARY_INFO "ON (repo: ${PYBIND11_GIT_REPOSITORY} tag: ${PYBIND11_GIT_TAG})")
else()
  set(MUSICA_ENABLE_PYTHON_LIBRARY_INFO "OFF")
endif()

set(MUSICA_ENABLE_MECH_CONFIG_INFO "   (repo: ${MECH_CONFIG_GIT_REPOSITORY} tag: ${MECH_CONFIG_GIT_TAG})")

# Configure musica.settings file
configure_file("${PROJECT_SOURCE_DIR}/cmake/musica.settings.in"
  "${PROJECT_BINARY_DIR}/musica.settings"
  @ONLY)

# Read in settings file, print out.
# Avoid using system-specific calls so that this
# might also work on Windows.
file(READ "${PROJECT_BINARY_DIR}/musica.settings"
  MUSICA_SETTINGS)
message(STATUS ${MUSICA_SETTINGS})