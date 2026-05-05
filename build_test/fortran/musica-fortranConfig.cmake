
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was musicaConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

if("OFF" STREQUAL "ON" OR "OFF" STREQUAL "ON")
  find_dependency(PkgConfig)
  pkg_check_modules(netcdfc IMPORTED_TARGET REQUIRED netcdf)
  pkg_check_modules(netcdff IMPORTED_TARGET REQUIRED netcdf-fortran)
endif()

if("OFF" STREQUAL "ON")
  find_dependency(BLAS)
  find_dependency(LAPACK)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/musica-fortran_Exports.cmake")

check_required_components("musica-fortran")
