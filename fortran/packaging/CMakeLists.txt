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
    ${MUSICA_FORTRAN_MOD_DIR}/../include/
  DESTINATION
    ${MUSICA_INSTALL_INCLUDE_DIR}/musica/fortran
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
  "${MUSICA_PROJECT_SRC_DIR}/cmake/musicaConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/musica-fortranConfig.cmake"
  INSTALL_DESTINATION
    ${cmake_config_install_location}
)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/musica-fortranConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(IS_DEBUG "")
else()
  set(IS_DEBUG "#")
endif()
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
