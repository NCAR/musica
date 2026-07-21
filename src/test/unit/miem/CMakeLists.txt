include(test_util)

create_standard_test_cxx(NAME miem_parser SOURCES parser.cpp)

create_standard_test_cxx(NAME miem_emissions_model_end_to_end SOURCES emissions_model_end_to_end.cpp)
target_compile_definitions(test_miem_emissions_model_end_to_end PRIVATE
  MIEM_REAL_FIXTURE_PATH="${miem_SOURCE_DIR}/test/data/CAMS-GLOB-ANT_2012_MPAS_bc_subset.nc")

create_standard_test_cxx(NAME miem_emissions_model_end_to_end_nox SOURCES emissions_model_end_to_end_nox.cpp)
target_compile_definitions(test_miem_emissions_model_end_to_end_nox PRIVATE
  MIEM_REAL_NOX_FIXTURE_PATH="${miem_SOURCE_DIR}/test/data/x1.163842_2024_nox_subset.nc")
