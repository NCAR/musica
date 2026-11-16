// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the miem engine behind musica::EmissionsModel.
#pragma once

#include <musica/emissions/emissions.hpp>

#include <mechanism_configuration/mechanism.hpp>

#include <miem/emissions.hpp>
#include <miem/emissions_state.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace musica
{
  class MiemBackend
  {
   public:
    MiemBackend(const mechanism_configuration::Mechanism& mechanism, int n_cells, int n_vert_levels);

    void Run(double epoch_seconds, double dt_seconds);
    const std::vector<std::string>& SpeciesNames() const;
    SurfaceFluxView SurfaceFlux() const;
    Flux3DView Flux3D() const;
    EmissionsEnvironment Environment();

   private:
    miem::Emissions emissions_;
    miem::EmissionsState state_;
    std::size_t n_vert_levels_;
    std::vector<double> surface_flux_;
    std::vector<double> flux_3d_;
  };
}  // namespace musica
