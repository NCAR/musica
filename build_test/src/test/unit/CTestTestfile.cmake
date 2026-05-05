# CMake generated Testfile for 
# Source directory: /home/runner/work/musica/musica/src/test/unit
# Build directory: /home/runner/work/musica/musica/build_test/src/test/unit
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[util]=] "/home/runner/work/musica/musica/build_test/test_util")
set_tests_properties([=[util]=] PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "/home/runner/work/musica/musica/build_test" _BACKTRACE_TRIPLES "/home/runner/work/musica/musica/cmake/test_util.cmake;77;add_test;/home/runner/work/musica/musica/cmake/test_util.cmake;65;add_musica_test;/home/runner/work/musica/musica/src/test/unit/CMakeLists.txt;9;create_standard_test_cxx;/home/runner/work/musica/musica/src/test/unit/CMakeLists.txt;0;")
subdirs("micm")
