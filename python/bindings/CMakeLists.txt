include(setup_musica_target)

################################################################################
# Python Extension Modules

set(PY_MODULES _musica)

set(MUSICA_PYTHON_SOURCES
  common.cpp
  cuda.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mechanism_configuration/mechanism_configuration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/conditions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/micm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/solver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/micm/state.cpp
  ${MUSICA_SOURCES}
)

if (MUSICA_ENABLE_TUVX)
  list(APPEND MUSICA_PYTHON_SOURCES
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
  list(APPEND MUSICA_PYTHON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/carma/carma.cpp
  )
endif()

# Single Python module (no separate _musica_gpu module)
# CUDA support is detected at runtime via CudaLoader
pybind11_add_module(_musica
  cpu.cpp
  ${MUSICA_PYTHON_SOURCES}
)
musica_setup_target(_musica MODE CPU)

# Set separate Fortran module directory for _musica to avoid conflicts
if (MUSICA_ENABLE_TUVX OR MUSICA_ENABLE_CARMA)
  set_target_properties(_musica PROPERTIES
    Fortran_MODULE_DIRECTORY ${PROJECT_BINARY_DIR}/musica_python_cpu_modules
  )
endif()

foreach(lib ${PY_MODULES})
  if (APPLE)
    set_target_properties(${lib} PROPERTIES
      INSTALL_RPATH "@loader_path"
      BUILD_WITH_INSTALL_RPATH TRUE
    )
  elseif(UNIX)
    set_target_properties(${lib} PROPERTIES
      INSTALL_RPATH "$ORIGIN"
      BUILD_WITH_INSTALL_RPATH TRUE
    )
  endif()

  install(TARGETS ${lib} LIBRARY DESTINATION musica)

  # Install the configs directory
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/configs DESTINATION musica)
endforeach()

# Install the CUDA plugin library alongside the Python module on Linux x86_64
if(UNIX AND NOT APPLE AND NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  # Build the CUDA plugin for Python if it exists
  if(TARGET musica_cuda)
    install(TARGETS musica_cuda LIBRARY DESTINATION musica)
  endif()
endif()
