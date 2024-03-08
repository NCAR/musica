################################################################################
# Add subdirectories containing tests

add_subdirectory(connections)
add_subdirectory(unit)

if (MUSICA_BUILD_FOTRAN_INTERFACE)
  add_subdirectory(fortran)
endif()