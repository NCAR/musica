# include(test_util)

# create_standard_test(NAME connect_to_musica_core SOURCES musica_core.F90 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# if (ENABLE_OPENMP)
#   create_standard_test(NAME connect_to_musica_core_openmp SOURCES musica_core_openmp.F90)
# endif()

# if (ENABLE_MPI)
#   create_standard_test(NAME connect_to_musica_core_mpi SOURCES musica_core_mpi.F90 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
# endif()

# if (ENABLE_TUVX)
#   create_standard_test(NAME connect_to_tuvx SOURCES tuvx.F90)
#   if (ENABLE_OPENMP)
#     create_standard_test(NAME connect_to_tuvx_openmp SOURCES tuvx_openmp.F90)
#   endif()
#   if (ENABLE_MPI)
#     create_standard_test(NAME connect_to_tuvx_mpi SOURCES tuvx_mpi.F90)
#   endif()
# endif()

# if (ENABLE_MICM)
#   create_standard_test_cxx(NAME connect_to_micm SOURCES micm.cpp)
# endif()


add_executable(connect_to_musica_core musica_core.F90)
target_link_libraries(connect_to_musica_core
  PUBLIC 
    musica::musica
)
add_test(
  NAME connect_to_musica_core
  COMMAND $<TARGET_FILE:connect_to_musica_core>
)

if (ENABLE_OPENMP)
  add_executable(connect_to_musica_core_openmp musica_core_openmp.F90)
  target_link_libraries(connect_to_musica_core_openmp
    PUBLIC 
      musica::musica
      OpenMP::OpenMP_Fortran
  )
  add_test(
    NAME connect_to_musica_core_openmp
    COMMAND $<TARGET_FILE:connect_to_musica_core_openmp>
  )
endif()

if(ENABLE_MPI)
  add_executable(connect_to_musica_core_mpi musica_core_mpi.F90)
  target_link_libraries(connect_to_musica_core_mpi
    PUBLIC 
      musica::musica
  )
  add_test(
    NAME connect_to_musica_core_mpi
    COMMAND mpirun -v -np 2 $<TARGET_FILE:connect_to_musica_core_mpi>
  )
endif()

if (ENABLE_MICM)
  add_executable(connect_to_micm micm.cpp)
  target_link_libraries(connect_to_micm
    PUBLIC 
      musica::musica
      GTest::gtest_main
  )
  add_test(
    NAME connect_to_micm
    COMMAND $<TARGET_FILE:connect_to_micm>
  )
endif()