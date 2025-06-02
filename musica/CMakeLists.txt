# GPUs are supported only on Linux
# But, to make the import simpler, we need to compile two versions of the pybind11 module:
# one with CUDA support and one without.
#
# Then, after the wheels are built, we have to modify the wheel file so that we don't upload the CUDA libraries to pypi
# and edit the rpath to search for sumbols using a specific cuda library version.

set(pybind11_libraries _musica)

if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None" AND NOT APPLE)
  list(APPEND pybind11_libraries _musica_gpu )
endif()

foreach(lib ${pybind11_libraries})
  pybind11_add_module(${lib}
    binding.cpp
    musica.cpp
    mechanism_configuration.cpp
  )

  target_link_libraries(${lib}
    PRIVATE
      musica::musica
  )

  include(silence_warnings)
  silence_warnings(${lib})

  if (APPLE)
    # set the rpath for the shared library
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

  if(WIN32)
    # makefiles on windows don't need the config directory
    if (${CMAKE_GENERATOR} MATCHES "MinGW Makefiles")
        set(PYTHON_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
    else()
    # but visual studio does
        set(PYTHON_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    endif()
  else()
    set(PYTHON_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
  endif()

  install(TARGETS ${lib} LIBRARY DESTINATION musica)

  install(
    TARGETS 
      yaml-cpp musica 
      LIBRARY DESTINATION musica/lib
      ARCHIVE DESTINATION musica/lib
  )
endforeach()