// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the IMicmSolver interface for type-erased MICM solvers.
// This enables runtime CUDA loading without compile-time ABI differences.
#pragma once

#include <musica/micm/chemistry.hpp>

#include <micm/solver/solver_result.hpp>
#include <micm/system/system.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <string>

namespace musica
{
  class IState;  // forward declaration

  /// @brief Abstract interface for type-erased MICM solvers
  /// This interface enables runtime polymorphism for different solver types
  /// (CPU and CUDA) without compile-time variant changes.
  class IMicmSolver
  {
   public:
    virtual ~IMicmSolver() = default;

    /// @brief Solve the chemical system for a given time step
    /// @param state The state object containing concentrations and conditions
    /// @param time_step Time [s] to advance the state by
    /// @return Solver result containing status and statistics
    virtual micm::SolverResult Solve(IState* state, double time_step) = 0;

    /// @brief Get the maximum number of grid cells this solver can handle
    /// @return Maximum number of grid cells per state
    virtual std::size_t MaximumNumberOfGridCells() const = 0;

    /// @brief Create a new state object compatible with this solver
    /// @param number_of_grid_cells Number of grid cells for the state
    /// @return Unique pointer to a new IState implementation
    virtual std::unique_ptr<IState> CreateState(std::size_t number_of_grid_cells) = 0;

    /// @brief Get the chemical system configuration
    /// @return The MICM System object
    virtual micm::System GetSystem() const = 0;

    /// @brief Get the species ordering map
    /// @return Map of species names to their indices
    virtual std::map<std::string, std::size_t> GetSpeciesOrdering() const = 0;

    /// @brief Get the rate parameter ordering map
    /// @return Map of rate parameter names to their indices
    virtual std::map<std::string, std::size_t> GetRateParameterOrdering() const = 0;

    /// @brief Get the vector size for this solver type
    /// @return Vector dimension for vector-ordered solvers, 1 for standard-ordered solvers
    virtual std::size_t GetVectorSize() const = 0;
  };

}  // namespace musica
