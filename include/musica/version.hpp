// Copyright (C) 2023-2025 National Center for Atmospheric Research
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
