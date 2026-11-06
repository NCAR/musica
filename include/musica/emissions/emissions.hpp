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

#include <string>
#include <vector>

namespace musica
{
  class EmissionsModel
  {
   public:
    /// @brief Build an emissions module from an already-translated musica::Emissions
    /// @param emissions Sources and regridding spec, e.g. produced by ConvertEmissions
    /// @param n_cells Number of horizontal grid cells
    /// @param n_vert_levels Number of vertical levels
    EmissionsModel(const Emissions& emissions, int n_cells, int n_vert_levels);

    /// @brief Convenience: parse a Mechanism's emissions config via ConvertEmissions and build
    /// @param mechanism Parsed mechanism configuration containing an emissions section
    /// @param n_cells Number of horizontal grid cells
    /// @param n_vert_levels Number of vertical levels
    static EmissionsModel FromMechanism(const mechanism_configuration::Mechanism& mechanism, int n_cells, int n_vert_levels);

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

    /// @brief Get the number of horizontal grid cells this model was built for
    int NumCells() const;

    /// @brief Get the flattened (n_species * n_cells) surface flux buffer from the most
    /// recent Run(), normalized to double precision (regardless of miem::Real's build-time
    /// precision) and laid out species-major (index = species_idx * NumCells() + cell_idx).
    /// Refreshed on every Run() call -- do not retain a reference/pointer across a Run() call.
    const std::vector<double>& SurfaceFluxData() const;

   private:
    miem::Emissions emissions_;
    miem::EmissionsState state_;
    std::vector<double> surface_flux_double_;
  };

}  // namespace musica
