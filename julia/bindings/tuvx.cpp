// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// TUV-x bindings for the Julia interface. This translation unit is compiled into
// libmusica_julia only when MUSICA is built with TUV-x enabled (see
// julia/CMakeLists.txt), so it needs no preprocessor guards — it registers
// itself with the module via the self-registration registry (registration.hpp).
#include "musica/tuvx/tuvx.hpp"

#include "jlcxx/jlcxx.hpp"
#include "registration.hpp"

namespace
{
  void register_tuvx(jlcxx::Module& mod)
  {
    // Calls into the TUV-x Fortran library.
    mod.method("get_tuvx_version", []() { return musica::TUVX::GetVersion(); });
  }

  [[maybe_unused]] const musica_julia::Registrar registrar(register_tuvx);
}  // namespace
