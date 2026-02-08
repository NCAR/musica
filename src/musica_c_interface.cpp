#include <musica/version.hpp>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif

    void MusicaVersion(String *musica_version)
    {
      CreateString(GetMusicaVersion(), musica_version);
    }

#ifdef __cplusplus
  }
#endif

}  // namespace musica