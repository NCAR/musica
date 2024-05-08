include(CMakePackageConfigHelpers)

install(
  TARGETS
    musica-fortran
  EXPORT
    musica_fortran_Exports
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# install the mod files
install(
  DIRECTORY
    ${MUSICA_MOD_DIR}/
    ${tuvx_BINARY_DIR}/include
  DESTINATION
    ${MUSICA_INSTALL_INCLUDE_DIR}/musica/fortran
  FILES_MATCHING PATTERN "*.mod"
)

# install the cmake config files
set(cmake_config_install_location "${CMAKE_INSTALL_LIBDIR}/cmake/musica")

install(
  EXPORT
    musica_fortran_Exports 
  DESTINATION
    ${cmake_config_install_location}
  NAMESPACE musica::
)

configure_package_config_file(
  "${MUSICA_PROJECT_SRC_DIR}/cmake/musicaConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/musica_fortranConfig.cmake"
  INSTALL_DESTINATION
    ${cmake_config_install_location}
)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/musica_fortranConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    ${PROJECT_BINARY_DIR}/musica_fortranConfig.cmake
    ${PROJECT_BINARY_DIR}/musica_fortranConfigVersion.cmake
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
