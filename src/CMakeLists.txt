# micm version
configure_file(version.c.in ${CMAKE_BINARY_DIR}/version.c @ONLY)

target_sources(musica
  PRIVATE
    component_versions.c
    ${CMAKE_BINARY_DIR}/version.c
)

add_subdirectory(c_fortran_interface)