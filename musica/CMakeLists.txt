################################################################################
# Python Extension Modules

#include(setup_muisca_target)

# Define the list of Python extension module targets
set(PY_MODULES _musica)

set(
  MUSICA_PYTHON_SOURCES 
    binding_common.cpp
    cuda.cpp 
    mechanism_configuration.cpp 
    musica.cpp 

    ${MUSICA_SOURCES}
)

pybind11_add_module(_musica cpu_binding.cpp ${MUSICA_PYTHON_SOURCES})
musica_setup_target(_musica MODE CPU)

if (NOT ${MUSICA_GPU_TYPE} STREQUAL "None")
  pybind11_add_module(_musica_gpu gpu_binding.cpp ${MUSICA_PYTHON_SOURCES}) 
  musica_setup_target(_musica_gpu MODE GPU)
  list(APPEND PY_MODULES _musica_gpu)
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
endforeach()
