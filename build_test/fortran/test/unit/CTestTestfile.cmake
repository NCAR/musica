# CMake generated Testfile for 
# Source directory: /home/runner/work/musica/musica/fortran/test/unit
# Build directory: /home/runner/work/musica/musica/build_test/fortran/test/unit
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[musica_version]=] "/home/runner/work/musica/musica/build_test/test_musica_version")
set_tests_properties([=[musica_version]=] PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "/home/runner/work/musica/musica/build_test" _BACKTRACE_TRIPLES "/home/runner/work/musica/musica/cmake/test_util.cmake;77;add_test;/home/runner/work/musica/musica/cmake/test_util.cmake;47;add_musica_test;/home/runner/work/musica/musica/fortran/test/unit/CMakeLists.txt;3;create_standard_test_fortran;/home/runner/work/musica/musica/fortran/test/unit/CMakeLists.txt;0;")
add_test([=[fortran_util]=] "/home/runner/work/musica/musica/build_test/test_fortran_util")
set_tests_properties([=[fortran_util]=] PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "/home/runner/work/musica/musica/build_test" _BACKTRACE_TRIPLES "/home/runner/work/musica/musica/cmake/test_util.cmake;77;add_test;/home/runner/work/musica/musica/cmake/test_util.cmake;47;add_musica_test;/home/runner/work/musica/musica/fortran/test/unit/CMakeLists.txt;4;create_standard_test_fortran;/home/runner/work/musica/musica/fortran/test/unit/CMakeLists.txt;0;")
add_test([=[micm_version]=] "/home/runner/work/musica/musica/build_test/test_micm_version")
set_tests_properties([=[micm_version]=] PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "/home/runner/work/musica/musica/build_test" _BACKTRACE_TRIPLES "/home/runner/work/musica/musica/cmake/test_util.cmake;77;add_test;/home/runner/work/musica/musica/cmake/test_util.cmake;47;add_musica_test;/home/runner/work/musica/musica/fortran/test/unit/CMakeLists.txt;7;create_standard_test_fortran;/home/runner/work/musica/musica/fortran/test/unit/CMakeLists.txt;0;")
