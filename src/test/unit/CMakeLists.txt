################################################################################
# Test utilities

include(test_util)

################################################################################
# Tests

create_standard_test_cxx(NAME component_versions SOURCES component_versions.cpp)

if (MUSICA_ENABLE_MICM)
  add_subdirectory(micm)
endif()

if (MUSICA_ENABLE_TUVX)
  add_subdirectory(tuvx)
endif()