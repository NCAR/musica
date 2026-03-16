include(test_util)

if (MUSICA_ENABLE_MICM)
  create_standard_test_fortran(NAME micm_fortran_api SOURCES test_micm_api.F90)
  create_standard_test_fortran(NAME micm_box_model SOURCES test_micm_box_model.F90)
  create_standard_test_fortran(NAME micm_multiple_grid_cells SOURCES test_micm_multiple_grid_cells.F90)
  create_standard_test_fortran(NAME demo_fortran SOURCES ../tutorial/demo.F90)
endif()

if (MUSICA_ENABLE_TUVX)
  create_standard_test_fortran(NAME tuvx_fortran_api SOURCES test_tuvx_api.F90)
endif()
