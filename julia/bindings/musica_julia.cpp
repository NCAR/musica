#include "jlcxx/jlcxx.hpp"
#include "musica/version.hpp"

#include <string>

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
  mod.method("get_version", []() { return std::string(musica::GetMusicaVersion()); });
}
