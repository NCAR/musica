// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines functions to manage and retrieve versions of components in a software system.
// It provides functionality to compare versions, check compatibility, and retrieve version information.
#include <musica/component_versions.hpp>
#include <musica/version.hpp>

#include <cstdlib>
#include <cstring>
#include <string>

#ifdef MUSICA_USE_MICM
  #include <micm/version.hpp>
#endif
#ifdef MUSICA_USE_TUVX
  #include <musica/tuvx/tuvx.hpp>
#endif
#ifdef MUSICA_USE_CARMA
  #include <musica/carma/carma.hpp>
#endif

namespace musica
{
  char* GetAllComponentVersions()
  {
    std::string result;

    result += "musica: ";
    result += musica::GetMusicaVersion();
    result += "\n";

#ifdef MUSICA_USE_MICM
    result += "micm: ";
    result += micm::GetMicmVersion();
    result += "\n";
#endif

#ifdef MUSICA_USE_TUVX
    result += "tuvx: ";
    result += musica::TUVX::GetVersion();
    result += "\n";
#endif

#ifdef MUSICA_USE_CARMA
    result += "carma: ";
    result += musica::CARMA::GetVersion();
    result += "\n";
#endif

    char* buf = static_cast<char*>(std::malloc(result.size() + 1));
    if (buf)
    {
      std::memcpy(buf, result.c_str(), result.size() + 1);
    }
    return buf;
  }
}  // namespace musica