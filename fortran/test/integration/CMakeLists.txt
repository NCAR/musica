include(test_util)

if (MUSICA_ENABLE_MICM)
  create_standard_test_fortran(NAME micm_fortran_api SOURCES test_micm_api.F90)
  create_standard_test_fortran(NAME micm_box_model SOURCES test_micm_box_model.F90)
  create_standard_test_fortran(NAME micm_multiple_grid_cells SOURCES test_micm_multiple_grid_cells.F90)
  create_standard_test_fortran(NAME tutorial_multiple_mechanisms SOURCES ../tutorial/test_multiple_mechanisms.F90)
endif()

if (MUSICA_ENABLE_TUVX)
  create_standard_test_fortran(NAME tuvx_fortran_api SOURCES test_tuvx_api.F90)
endif()

if (MUSICA_ENABLE_MIEM)
  set(MIEM_FORTRAN_FIXTURE_PATH "${CMAKE_CURRENT_BINARY_DIR}/selected_emissions_fixture.nc")
  set(MIEM_FORTRAN_CONFIG_PATH "${CMAKE_CURRENT_BINARY_DIR}/selected_emissions_config.yaml")
  configure_file(selected_emissions_config.yaml.in "${MIEM_FORTRAN_CONFIG_PATH}" @ONLY)

  add_executable(generate_miem_fortran_fixture
    generate_emissions_fixture.cpp
    ${miem_SOURCE_DIR}/test/util/synthetic_nc.cpp)
  target_include_directories(generate_miem_fortran_fixture PRIVATE ${miem_SOURCE_DIR}/test/util)
  target_link_libraries(generate_miem_fortran_fixture PRIVATE musica::miem)
  add_custom_command(
    OUTPUT "${MIEM_FORTRAN_FIXTURE_PATH}"
    COMMAND generate_miem_fortran_fixture "${MIEM_FORTRAN_FIXTURE_PATH}"
    DEPENDS generate_miem_fortran_fixture
    VERBATIM)
  add_custom_target(generate_miem_fortran_test_data DEPENDS "${MIEM_FORTRAN_FIXTURE_PATH}")

  create_standard_test_fortran(NAME emissions_fortran_api SOURCES test_emissions_api.F90)
  add_dependencies(test_emissions_fortran_api generate_miem_fortran_test_data)
  target_compile_definitions(test_emissions_fortran_api PRIVATE
    MIEM_SELECTED_CONFIG_PATH="${MIEM_FORTRAN_CONFIG_PATH}")
  set_tests_properties(emissions_fortran_api PROPERTIES TIMEOUT 60)
endif()
