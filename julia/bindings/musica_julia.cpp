// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Entry point for the MUSICA Julia bindings. Defines the binding registry and
// the single JLCXX module. Component bindings live in their own translation
// units (micm.cpp, tuvx.cpp, ...) and register themselves; this file only binds
// the always-available MUSICA version and then runs every registered component.
#include "jlcxx/jlcxx.hpp"
#include "registration.hpp"

#include <musica/version.hpp>

#include <string>
#include <vector>

namespace musica_julia
{
  std::vector<BindingRegistration>& binding_registry()
  {
    static std::vector<BindingRegistration> registry;
    return registry;
  }
}  // namespace musica_julia

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
  // Core MUSICA version (always available).
  mod.method("get_musica_version", []() { return std::string(musica::GetMusicaVersion()); });

  // Component bindings (MICM, TUV-x, ...) registered themselves at static-init
  // time; run each. A component is present only if its .cpp was compiled in
  // (decided in CMake), so no preprocessor guards are needed here.
  for (const auto& register_bindings : musica_julia::binding_registry())
    register_bindings(mod);
}
