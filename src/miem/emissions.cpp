// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the EmissionsModel class.
#include <musica/miem/emissions.hpp>

#include <miem/emissions_builder.hpp>

#include <algorithm>

namespace musica
{
  namespace
  {
    EmissionsOptions FullGridOptions(int n_cells, int n_vert_levels)
    {
      EmissionsOptions options;
      options.global_n_cells = n_cells;
      options.n_vert_levels = n_vert_levels;
      return options;
    }

    miem::Emissions BuildEmissions(const Emissions& emissions, const EmissionsOptions& options)
    {
      miem::EmissionsBuilder builder;
      builder.SetGridDimensions(options.global_n_cells, options.n_vert_levels);
      builder.SetCellSelection(options.selected_global_cell_ids);
      builder.SetDiagnosticSelection(options.diagnostics);
      builder.SetRegridding(emissions.regridding);
      for (const auto& source : emissions.sources)
      {
        builder.AddSource(source);
      }
      return builder.Build();
    }

    template<typename Input>
    std::vector<double> ToDouble(const Input& input)
    {
      return std::vector<double>(input.begin(), input.end());
    }
  }  // namespace

  EmissionsModel::EmissionsModel(const Emissions& emissions, int n_cells, int n_vert_levels)
      : EmissionsModel(emissions, FullGridOptions(n_cells, n_vert_levels))
  {
  }

  EmissionsModel::EmissionsModel(const Emissions& emissions, const EmissionsOptions& options)
      : emissions_(BuildEmissions(emissions, options)),
        diagnostic_selection_(options.diagnostics)
  {
  }

  EmissionsModel
  EmissionsModel::FromMechanism(const mechanism_configuration::Mechanism& mechanism, int n_cells, int n_vert_levels)
  {
    return EmissionsModel(ConvertEmissions(mechanism), n_cells, n_vert_levels);
  }

  EmissionsModel
  EmissionsModel::FromMechanism(const mechanism_configuration::Mechanism& mechanism, const EmissionsOptions& options)
  {
    return EmissionsModel(ConvertEmissions(mechanism), options);
  }

  const miem::EmissionsState& EmissionsModel::Run(double epoch_seconds, double dt_seconds)
  {
    state_ = emissions_.Run(epoch_seconds, dt_seconds);
    const auto& raw = state_.surface_flux_.raw();
    surface_flux_double_.assign(raw.begin(), raw.end());
    layer_flux_double_ = ToDouble(state_.layer_flux_);

    sector_flux_double_.clear();
    category_flux_double_.clear();
    sector_layer_flux_double_.clear();
    category_layer_flux_double_.clear();
    for (const auto& sector : state_.sector_names_)
    {
      sector_flux_double_.emplace(sector, ToDouble(state_.sector_fluxes_.at(sector).raw()));
      if (const auto* layered = state_.GetSectorLayerFlux(sector))
      {
        sector_layer_flux_double_.emplace(sector, ToDouble(*layered));
      }
    }
    for (const int category : state_.category_ids_)
    {
      category_flux_double_.emplace(category, ToDouble(state_.category_fluxes_.at(category).raw()));
      if (const auto* layered = state_.GetCategoryLayerFlux(category))
      {
        category_layer_flux_double_.emplace(category, ToDouble(*layered));
      }
    }
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

  double EmissionsModel::LayerFlux(int cell, int level, const std::string& species) const
  {
    const auto& names = emissions_.SpeciesNames();
    const auto species_it = std::find(names.begin(), names.end(), species);
    if (species_it == names.end() || cell < 0 || cell >= NumCells() || level < 0 || level >= NumVertLevels())
    {
      return 0.0;
    }
    const auto species_index = static_cast<std::size_t>(std::distance(names.begin(), species_it));
    const auto index = species_index * static_cast<std::size_t>(NumVertLevels()) * NumCells() +
                       static_cast<std::size_t>(level) * NumCells() + cell;
    return layer_flux_double_.at(index);
  }

  int EmissionsModel::NumCells() const
  {
    return emissions_.NumCells();
  }

  int EmissionsModel::NumGlobalCells() const
  {
    return emissions_.NumGlobalCells();
  }

  int EmissionsModel::NumVertLevels() const
  {
    return emissions_.NumVertLevels();
  }

  const std::vector<int>& EmissionsModel::SelectedGlobalCellIds() const
  {
    return emissions_.SelectedGlobalCellIds();
  }

  const std::vector<double>& EmissionsModel::SurfaceFluxData() const
  {
    return surface_flux_double_;
  }

  const std::vector<double>& EmissionsModel::LayerFluxData() const
  {
    return layer_flux_double_;
  }

  const std::vector<std::string>& EmissionsModel::DiagnosticSectorNames() const
  {
    return diagnostic_selection_.sectors_;
  }

  const std::vector<int>& EmissionsModel::DiagnosticCategoryIds() const
  {
    return diagnostic_selection_.categories_;
  }

  bool EmissionsModel::LayeredDiagnosticsEnabled() const
  {
    return diagnostic_selection_.layered_output_;
  }

  const std::vector<double>& EmissionsModel::SectorFluxData(const std::string& sector) const
  {
    return sector_flux_double_.at(sector);
  }

  const std::vector<double>& EmissionsModel::CategoryFluxData(int category) const
  {
    return category_flux_double_.at(category);
  }

  const std::vector<double>& EmissionsModel::SectorLayerFluxData(const std::string& sector) const
  {
    return sector_layer_flux_double_.at(sector);
  }

  const std::vector<double>& EmissionsModel::CategoryLayerFluxData(int category) const
  {
    return category_layer_flux_double_.at(category);
  }

  bool EmissionsModel::HasInventoryGridMetadata() const
  {
    return emissions_.HasInventoryGridMetadata();
  }

  const miem::InventoryGridMetadata& EmissionsModel::GridMetadata() const
  {
    return emissions_.GridMetadata();
  }

}  // namespace musica
