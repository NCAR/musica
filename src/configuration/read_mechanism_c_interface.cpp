// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/configuration/read_mechanism_c_interface.hpp>

namespace musica
{
  Mechanism* ReadMechanismC(const char* config_path, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          Mechanism* mechanism = new Mechanism(ReadMechanism(std::string(config_path)));
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
          Mechanism* mechanism = new Mechanism(ReadMechanismFromString(std::string(config_string)));
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
