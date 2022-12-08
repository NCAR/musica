################################################################################
# Test utilities

include(test_util)

################################################################################
# Tests

create_standard_test(NAME connect_to_musica_core SOURCES musica_core.F90)

if (ENABLE_TUVX)
  create_standard_test(NAME connect_to_tuvx SOURCES tuvx.F90)
endif()