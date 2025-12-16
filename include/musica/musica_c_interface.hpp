// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#ifdef __cplusplus
namespace musica
{
  extern "C"
  {
#endif

    /// @brief Get the MUSICA version
    /// @param musica_version MUSICA version [output]
    void MusicaVersion(String *musica_version);

#ifdef __cplusplus
  }
}
#endif
