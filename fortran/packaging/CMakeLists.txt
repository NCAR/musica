include(CMakePackageConfigHelpers)

install(
  TARGETS
    musica-fortran
  EXPORT
    musica-fortran_Exports
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY
    ${MUSICA_MOD_DIR}/
  DESTINATION
    ${MUSICA_INSTALL_MOD_DIR}
  FILES_MATCHING PATTERN "*.mod"
)

# install the cmake config files
set(cmake_config_install_location "${CMAKE_INSTALL_LIBDIR}/cmake/musica")

install(
  EXPORT
    musica-fortran_Exports 
  DESTINATION
    ${cmake_config_install_location}
  NAMESPACE musica::
)

configure_package_config_file(
  "${MUSICA_PROJECT_SRC_DIR}/cmake/musica-fortranConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/musica-fortranConfig.cmake"
  INSTALL_DESTINATION
    ${cmake_config_install_location}
)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/musica-fortranConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

if("${CMAKE_BUILD_TYPE}" MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
  set(IS_DEBUG "")
else()
  set(IS_DEBUG "#")
endif()

# musica-fortran is a static archive that is normally linked by a Fortran
# driver.  Its pkg-config metadata must therefore carry the enabled C/C++
# component archives and their non-Fortran link dependencies.  Keep these
# values derived from the configured targets/options rather than relying on a
# caller-provided CMAKE_CXX_STANDARD_LIBRARIES override.
set(MUSICA_FORTRAN_PKGCONFIG_REQUIRES "")
if(MUSICA_ENABLE_TUVX OR MUSICA_ENABLE_CARMA)
  # Some netcdf-fortran.pc files list NetCDF-C only in Requires.private,
  # which pkg-config omits without --static.  Name both packages explicitly
  # because musica-fortran's public metadata is used to link static archives.
  list(APPEND MUSICA_FORTRAN_PKGCONFIG_REQUIRES netcdf-fortran netcdf)
elseif(MUSICA_ENABLE_MIEM)
  list(APPEND MUSICA_FORTRAN_PKGCONFIG_REQUIRES netcdf)
endif()
list(REMOVE_DUPLICATES MUSICA_FORTRAN_PKGCONFIG_REQUIRES)
string(JOIN " " MUSICA_FORTRAN_PKGCONFIG_REQUIRES ${MUSICA_FORTRAN_PKGCONFIG_REQUIRES})

set(MUSICA_FORTRAN_COMPONENT_LIBS "")
if(MUSICA_ENABLE_MIEM)
  list(APPEND MUSICA_FORTRAN_COMPONENT_LIBS -lmiem)
endif()
string(JOIN " " MUSICA_FORTRAN_COMPONENT_LIBS ${MUSICA_FORTRAN_COMPONENT_LIBS})

set(MUSICA_FORTRAN_OPTIONAL_LIBS "")
if(MUSICA_USE_FMT AND TARGET fmt)
  list(APPEND MUSICA_FORTRAN_OPTIONAL_LIBS -lfmt)
endif()

if(MUSICA_ENABLE_TUVX AND TUVX_ENABLE_LAPACK)
  # TUV-x discovers these inside its own directory scope, so rediscover them
  # here to serialize the same external link interface into pkg-config.
  find_package(LAPACK REQUIRED)
  find_package(BLAS REQUIRED)
  list(APPEND MUSICA_FORTRAN_OPTIONAL_LIBS ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
elseif(MUSICA_ENABLE_CARMA)
  find_package(LAPACK REQUIRED)
  find_package(BLAS REQUIRED)
  list(APPEND MUSICA_FORTRAN_OPTIONAL_LIBS ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
endif()

foreach(link_library IN LISTS CMAKE_DL_LIBS)
  if(IS_ABSOLUTE "${link_library}" OR link_library MATCHES "^-")
    list(APPEND MUSICA_FORTRAN_OPTIONAL_LIBS "${link_library}")
  else()
    list(APPEND MUSICA_FORTRAN_OPTIONAL_LIBS "-l${link_library}")
  endif()
endforeach()

# A Fortran linker does not add the C++ runtime automatically.  CMake's
# detected implicit C++ link interface is portable across libstdc++ and libc++
# toolchains, unlike hard-coding either runtime here.
foreach(link_library IN LISTS CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
  if(link_library STREQUAL "stdc++" OR link_library STREQUAL "c++")
    list(APPEND MUSICA_FORTRAN_OPTIONAL_LIBS "-l${link_library}")
  endif()
endforeach()

list(REMOVE_DUPLICATES MUSICA_FORTRAN_OPTIONAL_LIBS)
string(JOIN " " MUSICA_FORTRAN_OPTIONAL_LIBS ${MUSICA_FORTRAN_OPTIONAL_LIBS})

set(CMAKE_INSTALL_PKGCONFIGDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
configure_file(
  "${MUSICA_PROJECT_SRC_DIR}/cmake/musica-fortran.pc.in"
  "${PROJECT_BINARY_DIR}/musica-fortran.pc"
  @ONLY
)

install(
  FILES
    "${PROJECT_BINARY_DIR}/musica-fortran.pc"
  DESTINATION
    "${CMAKE_INSTALL_PKGCONFIGDIR}"
)

install(
  FILES
    ${PROJECT_BINARY_DIR}/musica-fortranConfig.cmake
    ${PROJECT_BINARY_DIR}/musica-fortranConfigVersion.cmake
  DESTINATION
    ${cmake_config_install_location}
)

######################################################################
# uninstall target

# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
if(NOT TARGET uninstall)
  configure_file(
    "${MUSICA_PROJECT_SRC_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
