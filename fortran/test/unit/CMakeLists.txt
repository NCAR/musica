include(test_util)

create_standard_test_fortran(NAME musica_version SOURCES test_musica_version.F90)
create_standard_test_fortran(NAME fortran_util SOURCES util.F90)

if (MUSICA_ENABLE_MICM)
  create_standard_test_fortran(NAME micm_version SOURCES test_micm_version.F90)
endif()

if (MUSICA_ENABLE_TUVX)
  create_standard_test_fortran(NAME tuvx_version SOURCES test_tuvx_version.F90)

  create_standard_test_fortran(NAME connect_to_tuvx SOURCES tuvx.F90)

  if (MUSICA_ENABLE_OPENMP)
    create_standard_test_fortran(NAME connect_to_tuvx_openmp SOURCES tuvx_openmp.F90)
  endif()
endif()
