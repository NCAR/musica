pybind11_add_module(_musica
  binding.cpp
  musica.cpp
  mechanism_configuration.cpp
)

target_link_libraries(_musica
  PRIVATE
    musica::musica
)

include(silence_warnings)
silence_warnings(_musica)

if (APPLE)
  # set the rpath for the shared library
  set_target_properties(_musica PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
elseif(UNIX)
  # set the rpath for the shared library
  set_target_properties(_musica PROPERTIES
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

install(TARGETS _musica yaml-cpp musica LIBRARY DESTINATION .)