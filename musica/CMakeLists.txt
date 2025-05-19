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
  execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import site; print(site.getsitepackages()[0])"
    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  message(STATUS "Adding RPATH for python site packages libs at ${PYTHON_SITE_PACKAGES}/nvidia")

  # set the rpath for the shared library
  set_target_properties(_musica PROPERTIES
    INSTALL_RPATH "$ORIGIN:${PYTHON_SITE_PACKAGES}/nvidia"
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