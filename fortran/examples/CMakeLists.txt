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

# TUV-x v5.4 config uses relative data paths resolved from CWD
add_test(NAME tuvx_v54
         COMMAND ${CMAKE_BINARY_DIR}/test_tuvx_v54
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/configs/tuvx)
set_tests_properties(tuvx_v54 PROPERTIES TIMEOUT 120)
