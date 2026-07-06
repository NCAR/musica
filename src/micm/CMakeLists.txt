target_link_libraries(musica PUBLIC musica::micm)

# Link against dl for dlopen/dlsym on Linux
# needed for cuda plugin loading and micm dynamic loading
if(UNIX AND NOT APPLE)
  target_link_libraries(musica PUBLIC ${CMAKE_DL_LIBS})
endif()


target_sources(musica PRIVATE
  cpu_solver.cpp
  cuda_availability.cpp
  cuda_loader.cpp
  lambda_callback.cpp
  micm.cpp
  micm_c_interface.cpp
  state.cpp
  state_c_interface.cpp
)