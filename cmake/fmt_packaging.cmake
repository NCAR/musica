# Decide how consumers get fmt at link time.
#
# mechanism_configuration links fmt, so anything linking musica needs it too.
# If we built fmt ourselves (TARGET fmt) it is installed next to the libraries
# and exported as musica::fmt; otherwise it came from the system (e.g. Spack)
# and consumers must find it themselves.
#
# Sets, for use in the *.pc.in and *Config.cmake.in templates:
#   MUSICA_FMT_EXTERNAL            ON if consumers must find_dependency(fmt)
#   MUSICA_PC_FMT_LIBS             extra Libs entry for a bundled fmt
#   MUSICA_PC_REQUIRES_PRIVATE     Requires.private line for an external fmt
macro(musica_set_fmt_packaging_vars)
  set(MUSICA_FMT_EXTERNAL OFF)
  set(MUSICA_PC_FMT_LIBS "")
  set(MUSICA_PC_REQUIRES_PRIVATE "")
  # With a prebuilt musica we cannot tell how its fmt was provided
  if(MUSICA_USE_FMT AND NOT MUSICA_USE_PREBUILT)
    if(TARGET fmt)
      set(MUSICA_PC_FMT_LIBS " -l\${fmt_lib}")
    else()
      set(MUSICA_FMT_EXTERNAL ON)
      set(MUSICA_PC_REQUIRES_PRIVATE "Requires.private: fmt")
    endif()
  endif()
endmacro()
