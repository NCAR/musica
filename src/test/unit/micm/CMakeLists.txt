include(test_util)

create_standard_test_cxx(NAME micm_wrapper SOURCES micm_wrapper.cpp)
create_standard_test_cxx(NAME micm_c_api SOURCES micm_c_api.cpp)
create_standard_test_cxx(NAME parser SOURCES parser.cpp)