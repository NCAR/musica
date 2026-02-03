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

# Install the CUDA plugin library alongside the Python module on Linux x86_64
if(UNIX AND NOT APPLE AND NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  if(TARGET musica_cuda)
    install(TARGETS musica_cuda LIBRARY DESTINATION musica)
  endif()
endif()
