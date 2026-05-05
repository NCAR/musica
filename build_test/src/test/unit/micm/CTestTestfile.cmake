# CMake generated Testfile for 
# Source directory: /home/runner/work/musica/musica/src/test/unit/micm
# Build directory: /home/runner/work/musica/musica/build_test/src/test/unit/micm
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[micm_wrapper]=] "/home/runner/work/musica/musica/build_test/test_micm_wrapper")
set_tests_properties([=[micm_wrapper]=] PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "/home/runner/work/musica/musica/build_test" _BACKTRACE_TRIPLES "/home/runner/work/musica/musica/cmake/test_util.cmake;77;add_test;/home/runner/work/musica/musica/cmake/test_util.cmake;65;add_musica_test;/home/runner/work/musica/musica/src/test/unit/micm/CMakeLists.txt;3;create_standard_test_cxx;/home/runner/work/musica/musica/src/test/unit/micm/CMakeLists.txt;0;")
add_test([=[micm_c_api]=] "/home/runner/work/musica/musica/build_test/test_micm_c_api")
set_tests_properties([=[micm_c_api]=] PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "/home/runner/work/musica/musica/build_test" _BACKTRACE_TRIPLES "/home/runner/work/musica/musica/cmake/test_util.cmake;77;add_test;/home/runner/work/musica/musica/cmake/test_util.cmake;65;add_musica_test;/home/runner/work/musica/musica/src/test/unit/micm/CMakeLists.txt;4;create_standard_test_cxx;/home/runner/work/musica/musica/src/test/unit/micm/CMakeLists.txt;0;")
add_test([=[parser]=] "/home/runner/work/musica/musica/build_test/test_parser")
set_tests_properties([=[parser]=] PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "/home/runner/work/musica/musica/build_test" _BACKTRACE_TRIPLES "/home/runner/work/musica/musica/cmake/test_util.cmake;77;add_test;/home/runner/work/musica/musica/cmake/test_util.cmake;65;add_musica_test;/home/runner/work/musica/musica/src/test/unit/micm/CMakeLists.txt;5;create_standard_test_cxx;/home/runner/work/musica/musica/src/test/unit/micm/CMakeLists.txt;0;")
