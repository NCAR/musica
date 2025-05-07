include(test_util)

if (MUSICA_ENABLE_MICM)
  create_standard_test_fortran(NAME micm_fortran_api SOURCES ../fetch_content_integration/test_micm_api.F90)
  create_standard_test_fortran(NAME get_micm_version SOURCES ../fetch_content_integration/test_get_micm_version.F90)
  create_standard_test_fortran(NAME micm_box_model SOURCES ../fetch_content_integration/test_micm_box_model.F90)
  create_standard_test_fortran(NAME demo_fortran SOURCES ../tutorial/demo.F90)
endif()

if (MUSICA_ENABLE_TUVX)
  create_standard_test_fortran(NAME fortran_util SOURCES util.F90)
  create_standard_test_fortran(NAME connect_to_tuvx SOURCES tuvx.F90)
  create_standard_test_fortran(NAME tuvx_fortran_api SOURCES ../fetch_content_integration/test_tuvx_api.F90)

  if (MUSICA_ENABLE_OPENMP)
    create_standard_test_fortran(NAME connect_to_tuvx_openmp SOURCES tuvx_openmp.F90)
  endif()
endif()
