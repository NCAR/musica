target_link_libraries(musica PUBLIC musica::micm)

# Link against dl for dlopen/dlsym on Linux
# needed for cuda plugin loading and micm dynamic loading
if(UNIX AND NOT APPLE)
  target_link_libraries(musica PUBLIC ${CMAKE_DL_LIBS})
endif()


target_sources(musica PRIVATE
  convert_v0_to_v1.cpp
  cpu_solver.cpp
  cuda_availability.cpp
  cuda_loader.cpp
  micm.cpp
  micm_c_interface.cpp
  parse.cpp
  state.cpp
  state_c_interface.cpp
  v0_parse.cpp
  v1_parse.cpp
)