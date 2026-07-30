include(test_util)

# TUV-x v5.4 example / integration test
add_library(tuvx_v54_setup_lib OBJECT tuvx_v54_setup.F90)
target_link_libraries(tuvx_v54_setup_lib PUBLIC musica::musica-fortran)
set_target_properties(tuvx_v54_setup_lib
  PROPERTIES
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
)

add_executable(test_tuvx_v54 tuvx_v54.F90)
target_link_libraries(test_tuvx_v54 PUBLIC musica::musica-fortran tuvx_v54_setup_lib)
# These are Fortran programs that link the C++ musica libraries. Without this,
# CMake may pick C++ as the link driver (C++ outranks Fortran in linker
# preference), and nvhpc's nvc++ then omits the Fortran main shim -> "undefined
# reference to `main'". Force the Fortran driver so the program entry is linked.
set_target_properties(test_tuvx_v54 PROPERTIES LINKER_LANGUAGE Fortran)

# TUV-x v5.4 config uses relative data paths resolved from CWD
add_test(NAME tuvx_v54
         COMMAND ${CMAKE_BINARY_DIR}/test_tuvx_v54
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/configs/tuvx)
set_tests_properties(tuvx_v54 PROPERTIES TIMEOUT 120)

# Coupled TUV-x + MICM column model
add_library(column_model_setup_lib OBJECT column_model_setup.F90)
target_link_libraries(column_model_setup_lib PUBLIC musica::musica-fortran)
set_target_properties(column_model_setup_lib
  PROPERTIES
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
)

add_executable(test_column_model column_model.F90)
target_link_libraries(test_column_model PUBLIC
  musica::musica-fortran tuvx_v54_setup_lib column_model_setup_lib)
set_target_properties(test_column_model PROPERTIES LINKER_LANGUAGE Fortran)

# CWD = configs/tuvx for TUV-x data paths; MICM config uses ../v1/chapman/
add_test(NAME column_model
         COMMAND ${CMAKE_BINARY_DIR}/test_column_model
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/configs/tuvx)
set_tests_properties(column_model PROPERTIES TIMEOUT 600)

# TS1 box model driven by TUV-x photolysis rates
# (reuses the grid/profile/radiator helpers from tuvx_v54_setup_lib)
add_library(tuvx_ts1_setup_lib OBJECT tuvx_ts1_setup.F90)
target_link_libraries(tuvx_ts1_setup_lib PUBLIC musica::musica-fortran)
set_target_properties(tuvx_ts1_setup_lib
  PROPERTIES
    Fortran_MODULE_DIRECTORY ${MUSICA_MOD_DIR}
)
# tuvx_ts1_setup uses the tuvx_v54_setup module, so it must build after it
add_dependencies(tuvx_ts1_setup_lib tuvx_v54_setup_lib)

add_executable(test_ts1_box_model ts1_box_model.F90)
target_link_libraries(test_ts1_box_model PUBLIC
  musica::musica-fortran tuvx_v54_setup_lib tuvx_ts1_setup_lib)
set_target_properties(test_ts1_box_model PROPERTIES LINKER_LANGUAGE Fortran)

# CWD = configs/tuvx for TUV-x data paths; MICM config uses ../v1/ts1/
add_test(NAME ts1_box_model
         COMMAND ${CMAKE_BINARY_DIR}/test_ts1_box_model
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/configs/tuvx)
set_tests_properties(ts1_box_model PROPERTIES TIMEOUT 600)

if (MUSICA_ENABLE_MIEM AND MUSICA_ENABLE_MICM)
  # miem NOx box model: same TS1/TUV-x photolysis coupling as ts1_box_model,
  # plus a real EMISSION reaction (NO) driven each time step by a
  # musica_emissions module.
  add_executable(test_miem_nox_box_model miem_nox_box_model.F90)
  target_link_libraries(test_miem_nox_box_model PUBLIC
    musica::musica-fortran tuvx_v54_setup_lib tuvx_ts1_setup_lib)
  set_target_properties(test_miem_nox_box_model PROPERTIES LINKER_LANGUAGE Fortran)

  # CWD = configs/tuvx for TUV-x data paths; MICM config uses ../v1/miem_nox/,
  # miem config uses ../miem/
  add_test(NAME miem_nox_box_model
           COMMAND ${CMAKE_BINARY_DIR}/test_miem_nox_box_model
           WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/configs/tuvx)
  set_tests_properties(miem_nox_box_model PROPERTIES TIMEOUT 600)

  # Real-fixture variant: same TS1/TUV-x coupling, but driven from the real
  # committed x1.163842_2024_nox_subset.nc fixture, with a genuine
  # one-to-many NO/NO2 split instead of a 1:1 map.
  add_executable(test_miem_nox_box_model_real_fixture miem_nox_box_model_real_fixture.F90)
  target_link_libraries(test_miem_nox_box_model_real_fixture PUBLIC
    musica::musica-fortran tuvx_v54_setup_lib tuvx_ts1_setup_lib)
  set_target_properties(test_miem_nox_box_model_real_fixture PROPERTIES LINKER_LANGUAGE Fortran)

  add_test(NAME miem_nox_box_model_real_fixture
           COMMAND ${CMAKE_BINARY_DIR}/test_miem_nox_box_model_real_fixture
           WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/configs/tuvx)
  set_tests_properties(miem_nox_box_model_real_fixture PROPERTIES TIMEOUT 600)
endif()
