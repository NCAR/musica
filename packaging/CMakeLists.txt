include(CMakePackageConfigHelpers)

install(
  TARGETS 
    musica
  EXPORT 
    musica_Exports
  LIBRARY DESTINATION ${INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY 
    ${MUSICA_MOD_DIR}/
  DESTINATION 
    ${INSTALL_MOD_DIR}
  FILES_MATCHING PATTERN "*.mod"
)

set(cmake_config_install_location ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/musica-${PROJECT_VERSION})

install(
  EXPORT 
    musica_Exports
  DESTINATION 
    ${cmake_config_install_location}
  NAMESPACE musica::
)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/musicaConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/musicaConfig.cmake"
  INSTALL_DESTINATION
    ${cmake_config_install_location}
)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/musicaConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    ${PROJECT_BINARY_DIR}/musicaConfig.cmake
    ${PROJECT_BINARY_DIR}/musicaConfigVersion.cmake
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/musica-${PROJECT_VERSION}
)

######################################################################
# uninstall target

# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
if(NOT TARGET uninstall)
  configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()