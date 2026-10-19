// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/configuration/read_mechanism_c_interface.hpp>

#include <utility>

namespace musica
{
  Mechanism* ReadMechanismC(const char* config_path, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          // Parse into a named object first: nvc++ 26.5 crashes when the parse result is
          // a temporary in the new-expression (internal error in lower_dynamic_init).
          Mechanism parsed = ReadMechanism(std::string(config_path));
          Mechanism* mechanism = new Mechanism(std::move(parsed));
          NoError(error);
          return mechanism;
        },
        error);
  }

  Mechanism* ReadMechanismFromStringC(const char* config_string, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          Mechanism parsed = ReadMechanismFromString(std::string(config_string));
          Mechanism* mechanism = new Mechanism(std::move(parsed));
          NoError(error);
          return mechanism;
        },
        error);
  }

  void DeleteMechanism(Mechanism* mechanism, Error* error)
  {
    HandleErrors(
        [&]()
        {
          delete mechanism;
          NoError(error);
        },
        error);
  }
}  // namespace musica
