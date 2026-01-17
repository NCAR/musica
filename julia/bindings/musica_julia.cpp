#include "jlcxx/jlcxx.hpp"
#include "musica/version.hpp"

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
  mod.method("get_version", []() {
    return musica::GetMusicaVersion();
  });
}
