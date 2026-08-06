// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the definition of the EmissionsModel class, a musica-level
// convenience wrapper around miem::EmissionsBuilder and miem::Emissions, so that
// C/Python/Fortran bindings can target one stable interface instead of miem directly.
#pragma once

#include <musica/configuration/emissions.hpp>

#include <mechanism_configuration/mechanism.hpp>

#include <miem/emissions.hpp>
#include <miem/emissions_state.hpp>

#include <map>
#include <string>
#include <vector>

namespace musica
{
  /// @brief Host-grid and diagnostic options for rank-local MIEM construction.
  ///
  /// selected_global_cell_ids are one-based inventory slots in the exact
  /// order expected by the caller. An empty selection preserves the legacy
  /// full-grid behavior.
  struct EmissionsOptions
  {
    int global_n_cells = 0;
    int n_vert_levels = 0;
    std::vector<int> selected_global_cell_ids;
    miem::DiagnosticSelection diagnostics;
  };

  class EmissionsModel
  {
   public:
    /// @brief Build an emissions module from an already-translated musica::Emissions
    /// @param emissions Sources and regridding spec, e.g. produced by ConvertEmissions
    /// @param n_cells Number of horizontal grid cells
    /// @param n_vert_levels Number of vertical levels
    EmissionsModel(const Emissions& emissions, int n_cells, int n_vert_levels);

    /// @brief Build a rank-local emissions module with bounded diagnostics.
    EmissionsModel(const Emissions& emissions, const EmissionsOptions& options);

    /// @brief Convenience: parse a Mechanism's emissions config via ConvertEmissions and build
    /// @param mechanism Parsed mechanism configuration containing an emissions section
    /// @param n_cells Number of horizontal grid cells
    /// @param n_vert_levels Number of vertical levels
    static EmissionsModel FromMechanism(const mechanism_configuration::Mechanism& mechanism, int n_cells, int n_vert_levels);

    /// @brief Parse a Mechanism and build with selected-cell options.
    static EmissionsModel
    FromMechanism(const mechanism_configuration::Mechanism& mechanism, const EmissionsOptions& options);

    /// @brief Advance one time step and return the resulting state
    /// @param epoch_seconds Simulation time as seconds since epoch
    /// @param dt_seconds Time step [s]
    /// @return The emissions state computed for this time step
    const miem::EmissionsState& Run(double epoch_seconds, double dt_seconds);

    /// @brief Get the number of aggregated mechanism species across all sources
    int NumSpecies() const;

    /// @brief Get the aggregated mechanism species names across all sources
    const std::vector<std::string>& SpeciesNames() const;

    /// @brief Get the surface flux for a given cell and species from the most recent Run()
    /// @param cell Grid cell index
    /// @param species Mechanism species name
    double SurfaceFlux(int cell, const std::string& species) const;

    /// @brief Get the vertically distributed flux for one selected cell,
    /// level, and species from the most recent Run().
    double LayerFlux(int cell, int level, const std::string& species) const;

    /// @brief Get the number of horizontal grid cells this model was built for
    int NumCells() const;

    /// @brief Get the full inventory cell count supplied at construction.
    int NumGlobalCells() const;

    /// @brief Get the number of vertical levels supplied at construction.
    int NumVertLevels() const;

    /// @brief Get one-based selected global inventory cell IDs in output order.
    const std::vector<int>& SelectedGlobalCellIds() const;

    /// @brief Get the flattened (n_species * n_cells) surface flux buffer from the most
    /// recent Run(), normalized to double precision (regardless of miem::Real's build-time
    /// precision) and laid out species-major (index = species_idx * NumCells() + cell_idx).
    /// Refreshed on every Run() call -- do not retain a reference/pointer across a Run() call.
    const std::vector<double>& SurfaceFluxData() const;

    /// @brief Get the flattened species/level/cell layer flux buffer.
    /// Layout: species * NumVertLevels() * NumCells() + level * NumCells() + cell.
    const std::vector<double>& LayerFluxData() const;

    /// @brief Names/IDs selected for bounded diagnostics, in request order.
    const std::vector<std::string>& DiagnosticSectorNames() const;
    const std::vector<int>& DiagnosticCategoryIds() const;
    bool LayeredDiagnosticsEnabled() const;

    /// @brief Get selected diagnostic buffers from the most recent Run().
    const std::vector<double>& SectorFluxData(const std::string& sector) const;
    const std::vector<double>& CategoryFluxData(int category) const;
    const std::vector<double>& SectorLayerFluxData(const std::string& sector) const;
    const std::vector<double>& CategoryLayerFluxData(int category) const;

    /// @brief Exact inventory metadata, available after a successful selected run.
    bool HasInventoryGridMetadata() const;
    const miem::InventoryGridMetadata& GridMetadata() const;

   private:
    miem::Emissions emissions_;
    miem::EmissionsState state_;
    miem::DiagnosticSelection diagnostic_selection_;
    std::vector<double> surface_flux_double_;
    std::vector<double> layer_flux_double_;
    std::map<std::string, std::vector<double>> sector_flux_double_;
    std::map<int, std::vector<double>> category_flux_double_;
    std::map<std::string, std::vector<double>> sector_layer_flux_double_;
    std::map<int, std::vector<double>> category_layer_flux_double_;
  };

}  // namespace musica
