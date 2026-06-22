# Create the Julia wrapper library. Each component lives in its own translation
# unit and registers itself with the module (see bindings/registration.hpp), so
# a component is included simply by compiling its source — no preprocessor guards.
add_library(musica_julia SHARED
  bindings/musica_julia.cpp
  bindings/micm.cpp
)

# TUV-x bindings are only compiled when TUV-x is enabled (they call into the
# TUV-x Fortran library). Mirrors how the Python bindings add their sources.
if(MUSICA_ENABLE_TUVX)
  target_sources(musica_julia PRIVATE bindings/tuvx.cpp)
endif()

target_compile_features(musica_julia PUBLIC cxx_std_20)

set_target_properties(musica_julia PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(musica_julia
  musica::musica
  JlCxx::cxxwrap_julia
  JlCxx::cxxwrap_julia_stl
)

# Musica_jll hardcodes build/lib as the library path — must match exactly.
set_target_properties(musica_julia PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# yaml-cpp is built in its own FetchContent subdirectory. Copy its soname file
# into build/lib/ alongside the other libraries so the dynamic linker finds it
# via the existing rpath entry for build/lib/. This mirrors what the Python
# bindings do (install yaml-cpp alongside the extension module).
if(UNIX AND NOT APPLE AND TARGET yaml-cpp)
  add_custom_command(TARGET musica_julia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_SONAME_FILE:yaml-cpp>
      ${CMAKE_BINARY_DIR}/lib/$<TARGET_SONAME_FILE_NAME:yaml-cpp>
    COMMENT "Copying libyaml-cpp to build/lib/"
  )
endif()

# Install the library
if(MUSICA_ENABLE_INSTALL)
  install(TARGETS musica_julia
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endif()
