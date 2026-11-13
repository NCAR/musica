// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the definition of the EmissionsModel class, a musica-level
// interface to an emissions engine, so that C/Python/Fortran bindings can target
// one stable interface instead of a specific engine.
#pragma once

#include <mechanism_configuration/mechanism.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace musica
{
  /// @brief Read-only view of the surface flux [kg m-2 s-1] from the most recent Run().
  /// Element (species, cell) is data[species * species_stride + cell * cell_stride].
  /// The data is zero before the first Run().
  struct SurfaceFluxView
  {
    const double* data;
    std::size_t n_species;
    std::size_t n_cells;
    std::size_t species_stride;
    std::size_t cell_stride;
  };

  /// @brief Read-only view of the flux on each vertical level [kg m-2 s-1] from the most recent Run().
  /// Level 0 is the surface.
  /// Element (species, level, cell) is data[species * species_stride + level * level_stride + cell * cell_stride].
  /// The data is zero before the first Run().
  struct Flux3DView
  {
    const double* data;
    std::size_t n_species;
    std::size_t n_levels;
    std::size_t n_cells;
    std::size_t species_stride;
    std::size_t level_stride;
    std::size_t cell_stride;
  };

  /// @brief Writable views of the environment buffers that the model owns.
  /// The host writes the environment before Run(). The model keeps the values between runs.
  /// Each field is flat (level, cell) or (cell), and the cell index changes fastest.
  /// The struct has only the fields that a supported source needs.
  struct EmissionsEnvironment
  {
  };

  class EmissionsModel
  {
   public:
    /// @brief Build an emissions model from a Mechanism's emissions section
    /// @param mechanism Parsed mechanism configuration containing an emissions section
    /// @param n_cells Number of horizontal grid cells
    /// @param n_vert_levels Number of vertical levels
    EmissionsModel(const mechanism_configuration::Mechanism& mechanism, int n_cells, int n_vert_levels);
    ~EmissionsModel();
    EmissionsModel(EmissionsModel&&) noexcept;
    EmissionsModel& operator=(EmissionsModel&&) noexcept;

    /// @brief Advance one time step
    /// @param epoch_seconds Simulation time as seconds since epoch
    /// @param dt_seconds Time step [s]
    void Run(double epoch_seconds, double dt_seconds);

    /// @brief Get the aggregated mechanism species names, in surface flux order
    const std::vector<std::string>& SpeciesNames() const;

    /// @brief Get the surface flux from the most recent Run()
    SurfaceFluxView SurfaceFlux() const;

    /// @brief Get the flux on each vertical level from the most recent Run()
    Flux3DView Flux3D() const;

    /// @brief Get the environment buffers, so that the host can write them before Run()
    EmissionsEnvironment Environment();

   private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
  };

}  // namespace musica
