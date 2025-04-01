pybind11_add_module(musica_python 
  wrapper.cpp
)

target_link_libraries(musica_python 
  PRIVATE
    musica::musica
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(musica_python PUBLIC -Wno-return-type-c-linkage)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(musica_python PUBLIC /wd4190)
endif()

set_target_properties(musica_python PROPERTIES
  OUTPUT_NAME musica
)

if (APPLE)
  # set the rpath for the shared library
  set_target_properties(musica_python PROPERTIES
    INSTALL_RPATH "@loader_path;@loader_path/../lib"
  )
elseif(UNIX)
  # set the rpath for the shared library
  set_target_properties(musica_python PROPERTIES
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
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