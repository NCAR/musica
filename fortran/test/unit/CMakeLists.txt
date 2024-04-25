include(test_util)

create_standard_test_fortran(NAME micm_fortran_api SOURCES ../fetch_content_integration/test_micm_api.F90)

if (MUSICA_ENABLE_TUVX)
  create_standard_test_fortran(NAME connect_to_tuvx SOURCES tuvx.F90)
  create_standard_test_fortran(NAME tuvx_fortran_api SOURCES ../fetch_content_integration/test_tuvx_api.F90)

  if (MUSICA_ENABLE_OPENMP)
    create_standard_test_fortran(NAME connect_to_tuvx_openmp SOURCES tuvx_openmp.F90)
  endif()
  if (MUSICA_ENABLE_MPI)
    create_standard_test_fortran(NAME connect_to_tuvx_mpi SOURCES tuvx_mpi.F90)
  endif()
endif()