// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Builder function that converts a mechanism_configuration Mechanism (its
// species, phases, and aerosol section) into a miam::Model object and creates a
// MICM solver with it attached as an external model.
#pragma once

#include <musica/configuration/chemistry.hpp>
#include <musica/micm/micm.hpp>
#include <musica/utils/util.hpp>

#include <mechanism_configuration/mechanism.hpp>

namespace musica
{
  /// @brief Create a MICM solver with a MIAM external model attached
  /// @param mechanism Parsed mechanism carrying the species, phases, and aerosol
  ///                  section that define the MIAM model.
  /// @param solver_type The solver variant to use
  /// @param error Error output
  /// @return Pointer to an initialized MICM solver (caller owns)
  MICM* CreateMicmWithMiam(
      const mechanism_configuration::Mechanism& mechanism,
      MICMSolver solver_type,
      Error* error);
}  // namespace musica