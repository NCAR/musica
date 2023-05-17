#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <musica/version.h>
#ifdef MUSICA_USE_MICM
#include <micm/version.hpp>
#endif

char* getAllComponentVersions() {
  const char* sep = "\n";  // Changed single quotes to double quotes
  size_t sep_size = strlen(sep);
  size_t buf_size = 0;

  const char* musica_version = getMusicaVersion();
  buf_size += strlen(musica_version) + sep_size;

#ifdef MUSICA_USE_MICM
  const char* micm_version = getMicmVersion();
  buf_size += strlen(micm_version) + sep_size;
#endif

  char* buf = (char*)malloc(sizeof(char) * (buf_size + 1));

  if (buf) {
    char* pos = buf;
    strcpy(pos, musica_version);

    pos += strlen(musica_version);
    strcpy(pos, sep);
    pos += sep_size;

#ifdef MUSICA_USE_MICM
    strcpy(pos, micm_version);

    pos += strlen(micm_version);
    strcpy(pos, sep);
    pos += sep_size;
#endif
  }
  else {
    buf = NULL;
  }

  return buf;
}