#include <musica/version.hpp>

#include <cstring>
#include <iostream>

int main()
{
  const char* version = musica::GetMusicaVersion();
  std::cout << "MUSICA version: " << version << std::endl;
  return (version != nullptr && std::strlen(version) > 0) ? 0 : 1;
}
