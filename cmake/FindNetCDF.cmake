# FindNetCDF.cmake
# Find NetCDF C and Fortran libraries.
#
# Prefers cmake config files (spack-stack, Homebrew, Fedora dnf).
# Falls back to pkg-config when config files are absent (Ubuntu apt libnetcdff-dev
# does not ship netCDF-FortranConfig.cmake).
#
# In both cases the same targets are guaranteed after this module runs:
#   netCDF::netcdf  - NetCDF C library
#   netCDF::netcdff - NetCDF Fortran library
#
# Result variables:
#   NetCDF_FOUND   - True if both targets were found/created
#   NetCDF_VERSION - Version of netcdf-c (set when config files are available)

# If the C target already exists we are either fully done (both targets present)
# or being called recursively from find_dependency(netCDF) inside
# netCDF-FortranConfig.cmake. In both cases just return: the Fortran target is
# either already there or will be created by the config file that triggered this
# recursive call.
if(TARGET netCDF::netcdf)
  set(NetCDF_FOUND TRUE)
  return()
endif()

# ---- Try cmake config files first ----

find_package(netCDF CONFIG QUIET)
find_package(netCDF-Fortran CONFIG QUIET)

if(TARGET netCDF::netcdf AND netCDF_VERSION)
  set(NetCDF_VERSION "${netCDF_VERSION}")
endif()

# ---- pkg-config fallback for anything not yet found ----

if(NOT TARGET netCDF::netcdf OR NOT TARGET netCDF::netcdff)
  find_package(PkgConfig REQUIRED)

  if(NOT TARGET netCDF::netcdf)
    pkg_check_modules(_netcdfc REQUIRED IMPORTED_TARGET netcdf)
    add_library(netCDF::netcdf INTERFACE IMPORTED GLOBAL)
    target_link_libraries(netCDF::netcdf INTERFACE PkgConfig::_netcdfc)
  endif()

  if(NOT TARGET netCDF::netcdff)
    pkg_check_modules(_netcdff REQUIRED IMPORTED_TARGET netcdf-fortran)
    add_library(netCDF::netcdff INTERFACE IMPORTED GLOBAL)
    target_link_libraries(netCDF::netcdff INTERFACE PkgConfig::_netcdff)
  endif()
endif()

if(TARGET netCDF::netcdf AND TARGET netCDF::netcdff)
  set(NetCDF_FOUND TRUE)
else()
  set(NetCDF_FOUND FALSE)
  if(NetCDF_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find NetCDF (tried cmake config files and pkg-config)")
  endif()
endif()
