/**
 * This file defines functions to manage and retrieve versions of components in a software system.
 * It provides functionality to compare versions, check compatibility, and retrieve version information.
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 * 
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <musica/version.h>
#ifdef MUSICA_USE_MICM
#include <micm/version.hpp>
#endif

char* add_name_and_version(char* pos, const char* name, const char* version, const char* sep) {
  size_t name_length = strlen(name);
  size_t version_length = strlen(version);
  size_t sep_length = strlen(sep);

  memcpy(pos, name, name_length);
  pos += name_length;

  memcpy(pos, version, version_length);
  pos += version_length;

  memcpy(pos, sep, sep_length);
  pos += sep_length;

  return pos;
}

char* getAllComponentVersions() {
  const char* sep = "\n";
  size_t sep_size = strlen(sep);
  size_t buf_size = 0;

  const char* musica_name = "musica: ";
  const char* musica_version = getMusicaVersion();
  buf_size += strlen(musica_name) + strlen(musica_version) + sep_size;

#ifdef MUSICA_USE_MICM
  const char* micm_name = "micm: ";
  const char* micm_version = getMicmVersion();
  buf_size += strlen(micm_name) + strlen(micm_name) + sep_size;
#endif

  char* buf = (char*)malloc(sizeof(char) * (buf_size + 1));

  if (buf) {
    char* pos = buf;
    
    pos = add_name_and_version(pos, musica_name, musica_version, sep);
#ifdef MUSICA_USE_MICM
    pos = add_name_and_version(pos, micm_name, micm_version, sep);
#endif

    *pos = '\0';
  }

  return buf;
}