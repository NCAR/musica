include(test_util)

create_standard_test_cxx(NAME tuvx_c_api SOURCES tuvx_c_api.cpp)
create_standard_test_cxx(NAME tuvx_run_from_config SOURCES tuvx_run_from_config.cpp)

################################################################################
# Copy tuvx test data

add_custom_target(copy_tuvx_test_configs ALL ${CMAKE_COMMAND} -E copy_directory
  ${tuvx_SOURCE_DIR}/examples ${CMAKE_BINARY_DIR}/examples)

add_custom_target(copy_tuvx_data ALL ${CMAKE_COMMAND} -E copy_directory
  ${tuvx_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data)