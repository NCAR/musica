// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#ifdef __cplusplus
namespace musica
{
  extern "C"
  {
#endif

    const char* GetMusicaVersion();
    unsigned GetMusicaVersionMajor();
    unsigned GetMusicaVersionMinor();
    unsigned GetMusicaVersionPatch();
    unsigned GetMusicaVersionTweak();

#ifdef __cplusplus
  }
}
#endif
