################################################################################
# Python Extension Modules

# Define the list of Python extension module targets
set(PY_MODULES _musica)

# Add GPU module if CUDA is enabled
if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  list(APPEND PY_MODULES _musica_gpu)
endif()

# Loop over all Python extension modules
foreach(lib ${PY_MODULES})
  pybind11_add_module(
    ${lib} 
    musica.cpp
    mechanism_configuration.cpp
  )

  # Link to the correct object library
  if (${lib} STREQUAL "_musica_gpu")
    target_sources(${lib} PUBLIC $<TARGET_OBJECTS:musica_object_cuda> gpu_binding.cpp)
  else()
    target_sources(${lib} PUBLIC $<TARGET_OBJECTS:musica_object> cpu_binding.cpp)
  endif()

  target_include_directories(${lib}
    PUBLIC
      ${MUSICA_PROJECT_SRC_DIR}/include
  )

  target_link_libraries(${lib}
    PUBLIC
      musica::micm
      musica::mechanism_configuration
  )

  if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
      set_target_properties(micm_cuda PROPERTIES POSITION_INDEPENDENT_CODE ON)
      target_link_libraries(${lib}
        PUBLIC
          musica::micm_cuda
      )
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
