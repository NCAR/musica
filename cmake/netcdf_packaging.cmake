# Decide how consumers get netCDF at link time.
#
# Two separate netCDF dependencies:
#  - TUV-x/CARMA: use netCDF Fortran → `-lnetcdff` (which also requires `-lnetcdf`).
#  - MIEM: uses netCDF C (`<netcdf.h>`) → `-lnetcdf`.
# - `libmusica` is static, so `pkg-config --libs --static musica` must expose the
#    required netCDF libraries; otherwise consumers get `undefined nf90_* / nc_* symbols`.
# - `dependencies.cmake` finds netCDF via `pkg_check_modules`, but only for TUVX/CARMA.
#    In a MIEM-only build, those pkg-config paths aren't available because MIEM uses
#   `find_package(netCDF)` instead.
# - Spack/Homebrew install netCDF outside the default linker search paths, so the linker
#   may not find it.
# - Run `pkg-config` without `REQUIRED` solely to obtain the netCDF library directory.
#   This supplements MIEM's existing `find_package` logic.
#
# Sets, for use in the *.pc.in templates:
#   MUSICA_PC_LIBS_PRIVATE     Libs.private line, or empty
macro(musica_set_netcdf_packaging_vars)
  set(MUSICA_PC_LIBS_PRIVATE "")

  set(_musica_netcdf_libs "")
  if(MUSICA_ENABLE_TUVX OR MUSICA_ENABLE_CARMA)
    set(_musica_netcdf_libs "-lnetcdff -lnetcdf")
  elseif(MUSICA_ENABLE_MIEM)
    set(_musica_netcdf_libs "-lnetcdf")
  endif()

  if(NOT _musica_netcdf_libs STREQUAL "")
    if(NOT netcdfc_LIBRARY_DIRS AND NOT netcdff_LIBRARY_DIRS)
      find_package(PkgConfig QUIET)
      if(PkgConfig_FOUND)
        pkg_check_modules(musica_pc_netcdf QUIET netcdf)
      endif()
    endif()

    set(_musica_netcdf_dirs "")
    foreach(_dir IN LISTS netcdff_LIBRARY_DIRS netcdfc_LIBRARY_DIRS
                          musica_pc_netcdf_LIBRARY_DIRS)
      if(_dir AND NOT _dir IN_LIST _musica_netcdf_dirs)
        list(APPEND _musica_netcdf_dirs "${_dir}")
      endif()
    endforeach()
    set(MUSICA_PC_LIBS_PRIVATE "Libs.private:")
    foreach(_dir IN LISTS _musica_netcdf_dirs)
      string(APPEND MUSICA_PC_LIBS_PRIVATE " -L${_dir}")
    endforeach()
    string(APPEND MUSICA_PC_LIBS_PRIVATE " ${_musica_netcdf_libs}")
    unset(_musica_netcdf_dirs)
  endif()
  unset(_musica_netcdf_libs)
endmacro()
