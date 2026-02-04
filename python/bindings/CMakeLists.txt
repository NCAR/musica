################################################################################
# Python Extension Module

# Create the Python module
pybind11_add_module(_musica)

target_sources(_musica PRIVATE
  cpu.cpp
  common.cpp
  cuda.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mechanism_configuration/mechanism_configuration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/conditions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/micm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/solver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/state.cpp
)

if (MUSICA_ENABLE_TUVX)
  target_sources(_musica PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/grid_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/grid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/profile_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/profile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/radiator_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/radiator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tuvx/tuvx.cpp
  )
endif()

if (MUSICA_ENABLE_CARMA)
  target_sources(_musica PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/carma/carma.cpp
  )
endif()


# Link against the musica library (instead of compiling sources directly)
target_link_libraries(_musica PRIVATE musica::musica)

# Include directories for binding headers
target_include_directories(_musica PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set properties
set_target_properties(_musica PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

if (APPLE)
  set_target_properties(_musica PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
elseif(UNIX)
  set_target_properties(_musica PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

# Install the Python module
install(TARGETS _musica LIBRARY DESTINATION musica)

# Install the configs directory
install(DIRECTORY ${CMAKE_SOURCE_DIR}/configs DESTINATION musica)

function(install_prebuilt_so prefix libname dest)
  find_library(_lib ${libname} HINTS "${prefix}/lib" "${prefix}/lib64")
  if(_lib)
    get_filename_component(_dir "${_lib}" DIRECTORY)
    file(GLOB _libs "${_dir}/lib${libname}.so*")
    install(FILES ${_libs} DESTINATION ${dest})
  endif()
endfunction()

if(MUSICA_USE_PREBUILT)
  install_prebuilt_so("${CMAKE_PREFIX_PATH}" musica musica)
  install_prebuilt_so("${CMAKE_PREFIX_PATH}" mechanism_configuration musica)
  install_prebuilt_so("${CMAKE_PREFIX_PATH}" musica_cuda musica)
endif()

# Install the musica library alongside the Python module
if(MUSICA_USE_PREBUILT)
  # When using prebuilt, copy the musica library (including soversioned symlinks) from the prebuilt installation
  find_library(MUSICA_LIBRARY musica HINTS "${CMAKE_PREFIX_PATH}/lib")
  if(MUSICA_LIBRARY)
    message(STATUS "Installing musica library: ${MUSICA_LIBRARY}")
    get_filename_component(MUSICA_LIBRARY_DIR "${MUSICA_LIBRARY}" DIRECTORY)
    file(GLOB MUSICA_LIBRARY_FILES "${MUSICA_LIBRARY_DIR}/libmusica.so*")
    install(FILES ${MUSICA_LIBRARY_FILES} DESTINATION musica)
  endif()
elseif(TARGET musica)
  if(WIN32)
    install(TARGETS musica RUNTIME DESTINATION musica)
  else()
    install(TARGETS musica LIBRARY DESTINATION musica)
  endif()
endif()

# Install mechanism_configuration library alongside the Python module
iif(MUSICA_USE_PREBUILT)
  find_library(MECHANISM_CONFIG_LIBRARY mechanism_configuration
               HINTS "${CMAKE_PREFIX_PATH}/lib")
  if(MECHANISM_CONFIG_LIBRARY)
    get_filename_component(MC_LIBDIR "${MECHANISM_CONFIG_LIBRARY}" DIRECTORY)
    file(GLOB MC_LIBS "${MC_LIBDIR}/libmechanism_configuration.so*")
    install(FILES ${MC_LIBS} DESTINATION musica)
  endif()
endif()

# Install the CUDA plugin library alongside the Python module on Linux x86_64
if(UNIX AND NOT APPLE AND NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  if(MUSICA_USE_PREBUILT)
    # When using prebuilt, copy the CUDA plugin from the prebuilt installation
    find_library(MUSICA_CUDA_LIBRARY musica_cuda HINTS "${CMAKE_PREFIX_PATH}/lib")
    if(MUSICA_CUDA_LIBRARY)
      install(FILES ${MUSICA_CUDA_LIBRARY} DESTINATION musica)
    endif()
  elseif(TARGET musica_cuda)
    install(TARGETS musica_cuda LIBRARY DESTINATION musica)
  endif()
endif()
