# we specify the sources by hand because sometimes musica builds with tuvx
# it is hard to guarantee there is a fortran compiler on windows
pybind11_add_module(musica_python 
  wrapper.cpp
  ${PROJECT_SOURCE_DIR}/src/micm/micm.cpp
  ${PROJECT_SOURCE_DIR}/src/micm/parse.cpp
  ${PROJECT_SOURCE_DIR}/src/micm/v0_parse.cpp
  ${PROJECT_SOURCE_DIR}/src/micm/v1_parse.cpp
  ${PROJECT_SOURCE_DIR}/src/component_versions.cpp
  ${PROJECT_SOURCE_DIR}/src/util.cpp
  ${CMAKE_BINARY_DIR}/version.cpp
)

target_link_libraries(musica_python 
  PUBLIC 
    open_atmos::mechanism_configuration
  PRIVATE 
    yaml-cpp
)

target_compile_features(musica_python PUBLIC cxx_std_20)

set_target_properties(musica_python PROPERTIES OUTPUT_NAME musica)

target_include_directories(musica_python
  PUBLIC  
    $<BUILD_INTERFACE:${micm_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

target_include_directories(musica_python
  PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

# Set the rpath for the shared library
if(APPLE)
  message(STATUS "Building for MacOS")
  set_target_properties(musica_python PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
elseif(UNIX)
  message(STATUS "Building for Linux")
  set_target_properties(musica_python PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

install(TARGETS musica_python yaml-cpp mechanism_configuration
  LIBRARY DESTINATION .
)

if(WIN32)
  # makefiles on windows don't need the config directory
  if (${CMAKE_GENERATOR} MATCHES "MinGW Makefiles")
      set(PYTHON_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
  else()
  # but visual studio does
      set(PYTHON_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
  endif()
else()
  set(PYTHON_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
