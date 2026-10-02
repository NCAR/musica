# Decide how consumers get netCDF at link time.
#
# TUV-x and CARMA pull netCDF into libmusica, which is static by default, so
# `pkg-config --libs --static musica` has to name it or a consumer that reaches
# a TUV-x entry point fails on undefined nf90_* symbols.
#
# dependencies.cmake sets netcdfc_*/netcdff_* via pkg_check_modules; those are
# the non-static results, so they hold the clean `Libs:` entries. When MUSICA is
# a subproject reusing a parent's netCDF targets they are unset, and we fall
# back to bare -l flags.
#
# Sets, for use in the *.pc.in templates:
#   MUSICA_PC_LIBS_PRIVATE     Libs.private line, or empty
macro(musica_set_netcdf_packaging_vars)
  set(MUSICA_PC_LIBS_PRIVATE "")
  if(MUSICA_ENABLE_TUVX OR MUSICA_ENABLE_CARMA)
    set(_musica_netcdf_dirs "")
    foreach(_dir IN LISTS netcdff_LIBRARY_DIRS netcdfc_LIBRARY_DIRS)
      if(_dir AND NOT _dir IN_LIST _musica_netcdf_dirs)
        list(APPEND _musica_netcdf_dirs "${_dir}")
      endif()
    endforeach()
    set(MUSICA_PC_LIBS_PRIVATE "Libs.private:")
    foreach(_dir IN LISTS _musica_netcdf_dirs)
      string(APPEND MUSICA_PC_LIBS_PRIVATE " -L${_dir}")
    endforeach()
    string(APPEND MUSICA_PC_LIBS_PRIVATE " -lnetcdff -lnetcdf")
    unset(_musica_netcdf_dirs)
  endif()
endmacro()
