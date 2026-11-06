// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the miem engine behind musica::EmissionsModel.
#include "miem_backend.hpp"

#include "convert_emissions.hpp"

#include <miem/emissions_builder.hpp>

#include <type_traits>

static_assert(std::is_same_v<miem::Real, double>, "musica requires miem built with MIEM_USE_DOUBLE=ON");

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

  MiemBackend::MiemBackend(const mechanism_configuration::Mechanism& mechanism, int n_cells, int n_vert_levels)
      : emissions_(BuildEmissions(ConvertEmissions(mechanism), n_cells, n_vert_levels)),
        surface_flux_(static_cast<std::size_t>(emissions_.NumSpecies()) * emissions_.NumCells(), 0.0)
  {
  }

  void MiemBackend::Run(double epoch_seconds, double dt_seconds)
  {
    state_ = emissions_.Run(epoch_seconds, dt_seconds);
    const auto& raw = state_.surface_flux_.raw();
    surface_flux_.assign(raw.begin(), raw.end());
  }

  const std::vector<std::string>& MiemBackend::SpeciesNames() const
  {
    return emissions_.SpeciesNames();
  }

  SurfaceFluxView MiemBackend::SurfaceFlux() const
  {
    const auto n_cells = static_cast<std::size_t>(emissions_.NumCells());
    return SurfaceFluxView{ .data = surface_flux_.data(),
                            .n_species = static_cast<std::size_t>(emissions_.NumSpecies()),
                            .n_cells = n_cells,
                            .species_stride = n_cells,
                            .cell_stride = 1 };
  }

}  // namespace musica
