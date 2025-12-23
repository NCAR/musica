// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// WASM bindings for MUSICA using Emscripten

#include <emscripten/bind.h>
#include <string>

#include <musica/version.hpp>
#include <micm/version.hpp>

using namespace emscripten;

// Wrapper functions to return std::string instead of const char*
std::string GetVersion()
{
  return std::string(musica::GetMusicaVersion());
}

std::string GetMicmVersion()
{
  return std::string(micm::GetMicmVersion());
}

EMSCRIPTEN_BINDINGS(musica_module)
{
  function("getVersion", &GetVersion);
  function("getMicmVersion", &GetMicmVersion);
}
