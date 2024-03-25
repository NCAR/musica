pybind11_add_module(musica_python wrapper.cpp)

set_target_properties(musica_python PROPERTIES OUTPUT_NAME musica)

target_include_directories(musica_python
    PUBLIC  
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/micm/include>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

target_link_libraries(musica_python PUBLIC musica::musica)

add_test(NAME musica_python_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR}/$<CONFIG>
  ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/micm_test.py)