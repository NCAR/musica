# version
configure_file(version.c.in ${CMAKE_BINARY_DIR}/version.c @ONLY)

target_sources(musica
  PRIVATE
    component_versions.c
    ${CMAKE_BINARY_DIR}/version.c
)

#TODO(jiwon) comment out if you use 'Dockerfile.review'
# if(ENABLE_MICM)
#   add_subdirectory(micm)
# endif()