################################################################################
# Test utilities

include(test_util)

################################################################################
# Tests

create_standard_test_cxx(NAME util SOURCES util.cpp)

add_subdirectory(configuration)

if (MUSICA_ENABLE_MICM)
  add_subdirectory(micm)
endif()

if (MUSICA_ENABLE_MIAM)
  add_subdirectory(miam)
endif()

if (MUSICA_ENABLE_MIEM)
  add_subdirectory(miem)
endif()

if (MUSICA_ENABLE_TUVX)
  add_subdirectory(tuvx)
endif()

if (MUSICA_ENABLE_CARMA)
  add_subdirectory(carma)
endif()
