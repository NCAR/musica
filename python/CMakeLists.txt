pybind11_add_module(musica-python wrapper.cpp)

target_include_directories(musica-python
    PUBLIC  
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/micm/include>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

target_link_libraries(musica-python PUBLIC musica)

set_target_properties(musica-python PROPERTIES
  OUTPUT_NAME musica
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)