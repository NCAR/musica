// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// C-visible handle for a parsed mechanism_configuration::Mechanism, so that
// C/Fortran callers can obtain a Mechanism* to pass to CreateEmissions.
#pragma once

#include <musica/configuration/read_mechanism.hpp>
#include <musica/utils/util.hpp>

#include <mechanism_configuration/mechanism.hpp>

namespace musica
{
  using Mechanism = mechanism_configuration::Mechanism;

#ifdef __cplusplus
  extern "C"
  {
#endif
    /// @brief Parse a Mechanism from a configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to a heap-allocated Mechanism object (caller owns; free with DeleteMechanism)
    Mechanism* ReadMechanismC(const char* config_path, Error* error);

    /// @brief Parse a Mechanism from a JSON or YAML configuration string
    /// @param config_string JSON or YAML configuration string
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to a heap-allocated Mechanism object (caller owns; free with DeleteMechanism)
    Mechanism* ReadMechanismFromStringC(const char* config_string, Error* error);

    /// @brief Deletes a Mechanism object created by ReadMechanismC / ReadMechanismFromStringC
    /// @param mechanism Pointer to Mechanism object
    /// @param error Error struct to indicate success or failure
    void DeleteMechanism(Mechanism* mechanism, Error* error);
#ifdef __cplusplus
  }
#endif
}  // namespace musica
