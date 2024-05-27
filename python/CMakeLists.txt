pybind11_add_module(musica_python 
  wrapper.cpp
  ${PROJECT_SOURCE_DIR}/src/micm/micm.cpp
  ${PROJECT_SOURCE_DIR}/src/component_versions.cpp
  ${PROJECT_SOURCE_DIR}/src/util.cpp
  ${CMAKE_BINARY_DIR}/version.cpp
)

target_link_libraries(musica_python PRIVATE nlohmann_json::nlohmann_json)

target_compile_features(musica_python PUBLIC cxx_std_20)

set_target_properties(musica_python PROPERTIES OUTPUT_NAME musica)

target_include_directories(musica_python
  PUBLIC  
    $<BUILD_INTERFACE:${micm_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

target_include_directories(musica_python
  PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${MUSICA_INSTALL_INCLUDE_DIR}>
)

install(TARGETS musica_python
  LIBRARY DESTINATION .
)

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


add_subdirectory(test)