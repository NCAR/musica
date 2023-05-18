################################################################################
# Test utilities

include(test_util)

################################################################################
# Tests

create_standard_test(NAME micm_ccpp_api SOURCES micm.F90 LIBRARIES musica::ccpp)