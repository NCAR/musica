################################################################################
# Utility functions for creating tests

if(MUSICA_ENABLE_MEMCHECK)
  find_program(MEMORYCHECK_COMMAND "valgrind")
endif()

################################################################################
# build and add a standard test linked to musica

function(create_standard_test_fortran)
  set(prefix TEST)
  set(singleValues NAME WORKING_DIRECTORY)
  set(multiValues SOURCES LIBRARIES)
  include(CMakeParseArguments)
  cmake_parse_arguments(${prefix} " " "${singleValues}" "${multiValues}" ${ARGN})

  add_executable(test_${TEST_NAME} ${TEST_SOURCES})
  target_link_libraries(test_${TEST_NAME} PUBLIC musica::musica-fortran)
  if (${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel" OR ${CMAKE_Fortran_COMPILER_ID} MATCHES "NVHPC")
    set_target_properties(test_${TEST_NAME} PROPERTIES LINKER_LANGUAGE Fortran)
  endif()

  if (${CMAKE_Fortran_COMPILER_ID} STREQUAL "NVHPC")
    # for some reason, the NVHPC compiler does not realize that the fortran
    # tests are written in fortran, so we need to set the linker language
    # explicitly
    set_target_properties(test_${TEST_NAME}
      PROPERTIES
        LINKER_LANGUAGE Fortran
    )
  endif()

  # link additional libraries
  foreach(library ${TEST_LIBRARIES})
    target_link_libraries(test_${TEST_NAME} PUBLIC ${library})
  endforeach()

  if(MUSICA_ENABLE_OPENMP)
    target_link_libraries(test_${TEST_NAME} PUBLIC OpenMP::OpenMP_Fortran)
  endif()

  if(NOT DEFINED TEST_WORKING_DIRECTORY)
    set(TEST_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
  endif()

  add_musica_test(${TEST_NAME} test_${TEST_NAME} "" ${TEST_WORKING_DIRECTORY})
endfunction(create_standard_test_fortran)

function(create_standard_test_cxx)
  set(prefix TEST)
  set(singleValues NAME WORKING_DIRECTORY)
  set(multiValues SOURCES)
  include(CMakeParseArguments)
  cmake_parse_arguments(${prefix} " " "${singleValues}" "${multiValues}" ${ARGN})
  add_executable(test_${TEST_NAME} ${TEST_SOURCES})
  target_link_libraries(test_${TEST_NAME} PUBLIC musica::musica GTest::gtest_main)

  include(silence_warnings)
  silence_warnings(test_${TEST_NAME})

  if(MUSICA_ENABLE_OPENMP)
    target_link_libraries(test_${TEST_NAME} PUBLIC OpenMP::OpenMP_CXX)
  endif()
  if(NOT DEFINED TEST_WORKING_DIRECTORY)
    set(TEST_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
  endif()
  add_musica_test(${TEST_NAME} test_${TEST_NAME} "" ${TEST_WORKING_DIRECTORY})
endfunction(create_standard_test_cxx)

################################################################################
# Add a test

function(add_musica_test test_name test_binary test_args working_dir)
  if(MUSICA_ENABLE_MPI)
    add_test(NAME ${test_name}
      COMMAND mpirun -v -np 2 ${CMAKE_BINARY_DIR}/${test_binary} ${test_args}
             WORKING_DIRECTORY ${working_dir})
  else()
    add_test(NAME ${test_name}
             COMMAND ${test_binary} ${test_args}
             WORKING_DIRECTORY ${working_dir})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 20)
  endif()
  set(MEMORYCHECK_COMMAND_OPTIONS "--error-exitcode=1 --trace-children=yes --leak-check=full --gen-suppressions=all ${MEMCHECK_SUPPRESS}")
  set(memcheck "${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS}")
  separate_arguments(memcheck)
  if(MUSICA_ENABLE_MPI AND MEMORYCHECK_COMMAND AND MUSICA_ENABLE_MEMCHECK)
    add_test(NAME memcheck_${test_name}
      COMMAND mpirun -v -np 2 ${memcheck} ${CMAKE_BINARY_DIR}/${test_binary} ${test_args}
             WORKING_DIRECTORY ${working_dir})
    
    # add dependency between memcheck and previous test
    # https://stackoverflow.com/a/66931930/5217293
    set_tests_properties(${test_name} PROPERTIES FIXTURES_SETUP f_${test_name})
    set_tests_properties(memcheck_${test_name} PROPERTIES FIXTURES_REQUIRED f_${test_name})
  elseif(MEMORYCHECK_COMMAND AND MUSICA_ENABLE_MEMCHECK)
    add_test(NAME memcheck_${test_name}
             COMMAND ${memcheck} ${CMAKE_BINARY_DIR}/${test_binary} ${test_args}
             WORKING_DIRECTORY ${working_dir})
  endif()
endfunction(add_musica_test)

################################################################################
