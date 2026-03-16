################################################################################
# Add subdirectories containing tests

add_subdirectory(unit)

################################################################################
# Copy test data

add_custom_target(copy_unit_test_configs ALL ${CMAKE_COMMAND} -E copy_directory
  ${MUSICA_PROJECT_SRC_DIR}/configs ${CMAKE_BINARY_DIR}/configs)