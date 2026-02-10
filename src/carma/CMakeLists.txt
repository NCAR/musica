target_sources(musica PRIVATE $<TARGET_OBJECTS:carma_object>)
target_include_directories(musica
  PUBLIC
    $<BUILD_INTERFACE:${MUSICA_MOD_DIR}>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_MOD_DIR}>
)

# Link CARMA dependencies
get_target_property(_carma_links carma_object INTERFACE_LINK_LIBRARIES)
if (_carma_links)
  message(STATUS "Linking CARMA dependencies: ${_carma_links}")
  target_link_libraries(musica PUBLIC ${_carma_links})
endif()

if (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
  target_compile_options(musica PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-ffree-line-length-none>)
  target_compile_options(carma_object PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-ffree-line-length-none>)
endif()

target_sources(musica PRIVATE
  carma.cpp
  carma_state.cpp
  carma_c_interface.cpp
  interface.F90
  carma_parameters.F90
)