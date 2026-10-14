#include <musica/version.hpp>

#include <cstring>
#include <iostream>

#ifdef MUSICA_TEST_MIEM
  #include <musica/miem/emissions_c_interface.hpp>
#endif

int main()
{
  const char* version = musica::GetMusicaVersion();
  std::cout << "MUSICA version: " << version << std::endl;

#ifdef MUSICA_TEST_MIEM
  // MIEM is installed as its own archive rather than absorbed into libmusica,
  // and it pulls in netCDF-C. Referencing a symbol that lives in musica's MIEM
  // objects makes the linker resolve that whole chain, so a .pc or CMake export
  // that forgets -lmiem or -lnetcdf fails here. This is a link-time check, so
  // the entry point is never called.
  std::cout << "MIEM entry point: " << reinterpret_cast<const void*>(&musica::DeleteEmissions) << std::endl;
#endif

  return (version != nullptr && std::strlen(version) > 0) ? 0 : 1;
}
