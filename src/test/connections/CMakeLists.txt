include(test_util)

if (MUSICA_ENABLE_MICM)
  create_standard_test_cxx(NAME connect_to_micm SOURCES micm.cpp)
  create_standard_test_cxx(NAME micm_c_api SOURCES micm_c_api.cpp)
endif()