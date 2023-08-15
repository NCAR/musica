include(test_util)

create_standard_test(NAME connect_to_musica_core SOURCES musica_core.F90)

if (ENABLE_OPENMP)
  create_standard_test(NAME connect_to_musica_core_openmp SOURCES musica_core_openmp.F90)
endif()
if (ENABLE_MPI)
  create_standard_test(NAME connect_to_musica_core_mpi SOURCES musica_core_mpi.F90)
endif()

if (ENABLE_TUVX)
  create_standard_test(NAME connect_to_tuvx SOURCES tuvx.F90)
  if (ENABLE_OPENMP)
    create_standard_test(NAME connect_to_tuvx_openmp SOURCES tuvx_openmp.F90)
  endif()
  if (ENABLE_MPI)
    create_standard_test(NAME connect_to_tuvx_mpi SOURCES tuvx_mpi.F90)
  endif()
endif()

if (ENABLE_MICM)
  create_standard_test_cxx(NAME connect_to_micm SOURCES micm.cpp)
endif()