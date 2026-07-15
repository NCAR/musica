// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the EmissionsModel class.
#include <musica/miem/emissions.hpp>

#include <miem/emissions_builder.hpp>

namespace musica
{
  namespace
  {
    miem::Emissions BuildEmissions(const Emissions& emissions, int n_cells, int n_vert_levels)
    {
      miem::EmissionsBuilder builder;
      builder.SetGridDimensions(n_cells, n_vert_levels);
      builder.SetRegridding(emissions.regridding);
      for (const auto& source : emissions.sources)
      {
        builder.AddSource(source);
      }
      return builder.Build();
    }
  }  // namespace

  EmissionsModel::EmissionsModel(const Emissions& emissions, int n_cells, int n_vert_levels)
      : emissions_(BuildEmissions(emissions, n_cells, n_vert_levels))
  {
  }

  EmissionsModel EmissionsModel::FromMechanism(
      const mechanism_configuration::Mechanism& mechanism,
      int n_cells,
      int n_vert_levels)
  {
    return EmissionsModel(ConvertEmissions(mechanism), n_cells, n_vert_levels);
  }

  const miem::EmissionsState& EmissionsModel::Run(double epoch_seconds, double dt_seconds)
  {
    state_ = emissions_.Run(epoch_seconds, dt_seconds);
    return state_;
  }

  int EmissionsModel::NumSpecies() const
  {
    return emissions_.NumSpecies();
  }

  const std::vector<std::string>& EmissionsModel::SpeciesNames() const
  {
    return emissions_.SpeciesNames();
  }

  double EmissionsModel::SurfaceFlux(int cell, const std::string& species) const
  {
    return static_cast<double>(state_.surface_flux_(cell, species));
  }

}  // namespace musica
