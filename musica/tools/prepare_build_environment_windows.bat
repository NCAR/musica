@echo off
REM Windows build environment preparation script for MUSICA

REM Ensure MSYS2/MinGW64 is installed and in PATH
echo Setting up MinGW64 build environment...

REM Add MSYS2 MinGW64 to PATH if not already there
set "MINGW_PATH=C:\msys64\mingw64\bin"
echo %PATH% | find /i "%MINGW_PATH%" >nul || set "PATH=%MINGW_PATH%;%PATH%"

REM Verify MinGW64 compilers are available
where gcc || (echo "ERROR: gcc not found in PATH" && exit /b 1)
where g++ || (echo "ERROR: g++ not found in PATH" && exit /b 1) 
where gfortran || (echo "ERROR: gfortran not found in PATH" && exit /b 1)

REM Display compiler versions for debugging
echo GCC version:
gcc --version

echo G++ version:
g++ --version

echo GFortran version:
gfortran --version

REM Set CMake to use MinGW Makefiles generator
set CMAKE_GENERATOR=MSYS Makefiles

echo MinGW64 build environment setup complete.
