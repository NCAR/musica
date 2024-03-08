include(test_util)

create_standard_test(NAME connect_to_musica_core SOURCES musica_core.F90)

if (MUSICA_ENABLE_OPENMP)
  create_standard_test(NAME connect_to_musica_core_openmp SOURCES musica_core_openmp.F90)
endif()

if (MUSICA_ENABLE_MPI)
  create_standard_test(NAME connect_to_musica_core_mpi SOURCES musica_core_mpi.F90)
endif()

if (MUSICA_ENABLE_TUVX)
  create_standard_test(NAME connect_to_tuvx SOURCES tuvx.F90)
  if (MUSICA_ENABLE_OPENMP)
    create_standard_test(NAME connect_to_tuvx_openmp SOURCES tuvx_openmp.F90)
  endif()
  if (MUSICA_ENABLE_MPI)
    create_standard_test(NAME connect_to_tuvx_mpi SOURCES tuvx_mpi.F90)
  endif()
endif()

if (MUSICA_ENABLE_MICM)
  create_standard_test_cxx(NAME connect_to_micm SOURCES micm.cpp)
  create_standard_test_cxx(NAME micm_c_api SOURCES micm_c_api.cpp)
endif()