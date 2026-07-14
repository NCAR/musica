include(test_util)

create_standard_test_cxx(NAME read_mechanism SOURCES read_mechanism.cpp)

if (MUSICA_ENABLE_MICM AND MUSICA_ENABLE_MIEM)
  create_standard_test_cxx(NAME one_parse_many_converters SOURCES one_parse_many_converters.cpp)
endif()
