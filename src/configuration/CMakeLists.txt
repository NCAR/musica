target_sources(musica PRIVATE
  read_mechanism.cpp
)

if(MUSICA_ENABLE_MICM)
  target_sources(musica PRIVATE parse.cpp)
endif()

if(MUSICA_ENABLE_MIEM)
  target_sources(musica PRIVATE emissions.cpp)
  target_link_libraries(musica PUBLIC musica::miem)
endif()