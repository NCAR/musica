// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Builder function that converts MIAM configuration structs into a
// miam::Model object and creates a MICM solver with it as an external model.
#pragma once

#include <musica/miam/miam_types.hpp>
#include <musica/micm/chemistry.hpp>
#include <musica/micm/micm.hpp>
#include <musica/util.hpp>

namespace musica
{
  /// @brief Create a MICM solver with a MIAM external model attached
  /// @param chemistry Gas-phase chemistry (system + processes)
  /// @param solver_type The solver variant to use
  /// @param miam_config Complete MIAM model configuration
  /// @param error Error output
  /// @return Pointer to an initialized MICM solver (caller owns)
  MICM* CreateMicmWithMiam(
      const Chemistry& chemistry,
      MICMSolver solver_type,
      const miam_config::ModelConfig& miam_config,
      Error* error);
}  // namespace musica
