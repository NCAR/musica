include(test_util)

create_standard_test_cxx(NAME miem_parser SOURCES parser.cpp)

create_standard_test_cxx(NAME miem_end_to_end SOURCES end_to_end.cpp)
target_compile_definitions(test_miem_end_to_end PRIVATE
  MIEM_REAL_FIXTURE_PATH="${miem_SOURCE_DIR}/test/data/CAMS-GLOB-ANT_2012_MPAS_bc_subset.nc")
