project(musica-python)
pybind11_add_module(musica-python ../src/micm/wrapper.cpp)

target_include_directories(musica-python
    PUBLIC  
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/micm/include>
      $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
  )

target_link_libraries(musica-python PUBLIC musica)
