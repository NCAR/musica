include(CMakePackageConfigHelpers)

set(_musica_export_targets musica mechanism_configuration)

# If yaml-cpp was fetched (not found as a system package), include it in the
# export set so the generated targets file can reference it.  When yaml-cpp
# comes from find_package, the imported target cannot be re-exported; consumers
# will locate it via find_dependency in musicaConfig.cmake instead.
if(TARGET yaml-cpp AND NOT yaml-cpp_FOUND)
  list(APPEND _musica_export_targets yaml-cpp)
  set(MUSICA_YAML_CPP_BUNDLED TRUE)
else()
  set(MUSICA_YAML_CPP_BUNDLED FALSE)
endif()

install(
  TARGETS
    ${_musica_export_targets}
  EXPORT
    musica_Exports
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# install the musica header files
install(
  DIRECTORY
    ${MUSICA_PROJECT_SRC_DIR}/include/
  DESTINATION
    ${MUSICA_INSTALL_INCLUDE_DIR}
)

# install MICM
if(MUSICA_ENABLE_MICM)
  install(
    TARGETS
      micm
    EXPORT
      musica_Exports
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
    install (
      TARGETS
        micm_cuda
      EXPORT
        musica_Exports
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
  endif()

  install(
    DIRECTORY
      ${micm_SOURCE_DIR}/include/
    DESTINATION 
      ${MUSICA_INSTALL_INCLUDE_DIR}
  )
endif()

if (MUSICA_ENABLE_TUVX OR MUSICA_ENABLE_CARMA)
  # TUV-x and CARMA code is written in Fortran, so we need module files
  install(
    DIRECTORY
      ${MUSICA_MOD_DIR}/
    DESTINATION
      ${MUSICA_INSTALL_MOD_DIR}
    FILES_MATCHING PATTERN "*.mod"
  )
endif()

# install the cmake config files
set(cmake_config_install_location "${CMAKE_INSTALL_LIBDIR}/cmake/musica")

install(
  EXPORT
    musica_Exports 
  DESTINATION
    ${cmake_config_install_location}
  NAMESPACE musica::
)

configure_package_config_file(
  "${MUSICA_PROJECT_SRC_DIR}/cmake/musicaConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/musicaConfig.cmake"
  INSTALL_DESTINATION
    ${cmake_config_install_location}
)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/musicaConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

if("${CMAKE_BUILD_TYPE}" MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
  set(IS_DEBUG "")
else()
  set(IS_DEBUG "#")
endif()
set(CMAKE_INSTALL_PKGCONFIGDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
configure_file(
  "${MUSICA_PROJECT_SRC_DIR}/cmake/musica.pc.in"
  "${PROJECT_BINARY_DIR}/musica.pc"
  @ONLY
)

install(
  FILES
    "${PROJECT_BINARY_DIR}/musica.pc"
  DESTINATION
    "${CMAKE_INSTALL_PKGCONFIGDIR}"
)

install(
  FILES
    ${PROJECT_BINARY_DIR}/musicaConfig.cmake
    ${PROJECT_BINARY_DIR}/musicaConfigVersion.cmake
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

######################################################################
# Environment module

if (MUSICA_CREATE_ENVIRONMENT_MODULE)
  # Define the paths and environment variables for the module
  set(MODULE_NAME ${PROJECT_VERSION}.lua)
  set(MODULE_FILE_PATH ${CMAKE_BINARY_DIR}/modulefiles/${PROJECT_NAME}/${MODULE_NAME})
  set(MODULE_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_PREFIX})

  message(STATUS "MODULE NAME ${MODULE_NAME}")
  message(STATUS "MODULE PATH ${MODULE_FILE_PATH}")

  # Generate the module file using the template
  configure_file(${MUSICA_PROJECT_SRC_DIR}/src/packaging/modulefile.lua.in
                 ${MODULE_FILE_PATH}
                @ONLY)

  if(MUSICA_INSTALL_MODULE_FILE_PATH)
    # Create a custom target for installing the module file
    add_custom_target(install_lmod_module
                      COMMAND ${CMAKE_COMMAND} -E copy
                              ${MODULE_FILE_PATH}
                              ${MUSICA_INSTALL_MODULE_FILE_PATH}/${PROJECT_NAME}/${MODULE_NAME})

    # Add the install_lmod_module target to the "install" target
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install_lmod_module)")

    # Add a message to inform users how to trigger the installation
    message(STATUS "To install the Lmod module, run: make install")
  endif()
endif()
