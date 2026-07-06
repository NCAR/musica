target_link_libraries(musica PUBLIC musica::miem)

target_sources(musica PRIVATE
  convert.cpp
  parse.cpp
)
