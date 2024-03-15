include(test_util)

if (MUSICA_ENABLE_MICM)
  create_standard_test_cxx(NAME connect_to_micm SOURCES micm.cpp)
  create_standard_test_cxx(NAME micm_c_api SOURCES micm_c_api.cpp)

  ################################################################################
  # Copy test data

  add_custom_target(copy_unit_test_configs ALL ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/configs ${CMAKE_BINARY_DIR}/configs)
endif()