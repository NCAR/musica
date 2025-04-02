pybind11_add_module(musica_python 
  wrapper.cpp
)

target_link_libraries(musica_python 
  PRIVATE
    musica::musica
)

include(silence_warnings)
silence_warnings(musica_python)

set_target_properties(musica_python PROPERTIES
  OUTPUT_NAME musica
)

if (APPLE)
  # set the rpath for the shared library
  set_target_properties(musica_python PROPERTIES
    INSTALL_RPATH "@loader_path"
  )
elseif(UNIX)
  # set the rpath for the shared library
  set_target_properties(musica_python PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

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

install(TARGETS musica_python yaml-cpp musica mechanism_configuration LIBRARY DESTINATION .)