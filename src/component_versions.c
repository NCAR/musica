#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <musica/version.h>
#ifdef MUSICA_USE_MICM
#include <micm/version.hpp>
#endif

char* add_name_and_version(char* buf, const char* name, const char* version, const char* sep) {
  char* pos = buf;

  strcpy(pos, name);
  pos += strlen(name);

  strcpy(pos, version);
  pos += strlen(version);

  strcpy(pos, sep);
  pos += strlen(sep);

  return pos;
}

char* getAllComponentVersions() {
  const char* sep = "\n";  // Changed single quotes to double quotes
  size_t sep_size = strlen(sep);
  size_t buf_size = 0;

  const char* musica_name = "musica: ";
  const char* musica_version = getMusicaVersion();
  buf_size += strlen(musica_name) + strlen(musica_version) + sep_size;

#ifdef MUSICA_USE_MICM
  const char* micm_name = "micm: ";
  const char* micm_version = getMicmVersion();
  buf_size += strlen(micm_name) + sep_size;
#endif

  char* buf = (char*)malloc(sizeof(char) * (buf_size + 1));

  if (buf) {
    char* pos = add_name_and_version(buf, musica_name, musica_version, sep);
#ifdef MUSICA_USE_MICM
    pos = add_name_and_version(pos, musica_name, micm_version, sep);
#endif
  }
  else {
    buf = NULL;
  }

  return buf;
}