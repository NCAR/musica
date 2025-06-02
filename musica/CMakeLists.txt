################################################################################
# Python Extension Modules

# Define common sources for both Python bindings
set(MUSICA_PYBIND_SOURCES
  binding.cpp
  musica.cpp
  mechanism_configuration.cpp
)

# Define the list of Python extension module targets
set(PY_MODULES _musica)

# Add GPU module if CUDA is enabled
if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None" AND NOT APPLE)
  list(APPEND PY_MODULES _musica_gpu)
endif()

# Loop over all Python extension modules
foreach(lib ${PY_MODULES})
  pybind11_add_module(${lib} ${MUSICA_PYBIND_SOURCES})

  # Common include paths
  target_include_directories(${lib}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
  )

  # Common linking (to core MUSICA object library)
  target_link_libraries(${lib}
    PRIVATE
      musica::musica
  )

  # Silence warnings
  include(silence_warnings)
  silence_warnings(${lib})

  # GPU-specific options
  if (${lib} STREQUAL "_musica_gpu")
    target_compile_definitions(${lib} PRIVATE MUSICA_ENABLE_CUDA)
    target_link_libraries(${lib} PRIVATE musica::micm_cuda)
  endif()

  # RPATH handling (Linux/macOS)
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

  # Python module install path
  install(TARGETS ${lib} LIBRARY DESTINATION musica)
endforeach()
