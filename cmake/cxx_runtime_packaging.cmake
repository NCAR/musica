# This CMake macro detects the C++ runtime library used by the compiler and
# prepares it for inclusion in `pkg-config` files.

#  - Checks `CMAKE_CXX_IMPLICIT_LINK_LIBRARIES` for:
#   - `libstdc++`
#   - `libc++`
#   - `libc++abi`
# Builds a linker flag and stores the result in `MUSICA_PC_CXX_RUNTIME`.
macro(musica_set_cxx_runtime_packaging_vars)
  set(MUSICA_PC_CXX_RUNTIME "")
  foreach(_lib IN LISTS CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
    if(_lib MATCHES "^(stdc\\+\\+|c\\+\\+|c\\+\\+abi)$")
      string(APPEND MUSICA_PC_CXX_RUNTIME " -l${_lib}")
    endif()
  endforeach()
  if(MUSICA_PC_CXX_RUNTIME STREQUAL "")
    message(WARNING
      "Could not determine the C++ runtime for the pkg-config files from "
      "CMAKE_CXX_IMPLICIT_LINK_LIBRARIES (${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES}). "
      "Linking musica through pkg-config with a C or Fortran driver may fail on "
      "undefined std:: symbols.")
  endif()
endmacro()
