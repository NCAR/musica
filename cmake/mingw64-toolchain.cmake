# CMake toolchain file for MinGW64 on Windows
# This helps CMake find and use the correct MinGW compilers

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Set the compilers
set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe") 
set(CMAKE_Fortran_COMPILER "C:/msys64/mingw64/bin/gfortran.exe")

# Set the resource compiler
set(CMAKE_RC_COMPILER "C:/msys64/mingw64/bin/windres.exe")

# Set the target environment
set(CMAKE_FIND_ROOT_PATH "C:/msys64/mingw64")

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment only
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment only
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Make sure we use MinGW specific flags
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
