################################################################################
# Python Extension Modules

include(setup_musica_target)

# Define the list of Python extension module targets
set(PY_MODULES _musica)

set(MUSICA_PYTHON_SOURCES ${MUSICA_SOURCES})

# Add subdirectory for bindings
add_subdirectory(bindings)
