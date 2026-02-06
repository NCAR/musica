// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the IState interface for type-erased MICM states.
// This enables runtime CUDA loading without compile-time ABI differences.
#pragma once

#include <micm/solver/state.hpp>

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace musica
{
  /// @brief Abstract interface for type-erased MICM states
  /// This interface enables runtime polymorphism for different state types
  /// (CPU vector, CPU standard, and CUDA) without compile-time variant changes.
  class IState
  {
   public:
    virtual ~IState() = default;

    /// @brief Get the number of grid cells
    /// @return Number of grid cells
    virtual std::size_t NumberOfGridCells() const = 0;

    /// @brief Get the number of species
    /// @return Number of species
    virtual std::size_t NumberOfSpecies() const = 0;

    /// @brief Get the number of user-defined rate parameters
    /// @return Number of user-defined rate parameters
    virtual std::size_t NumberOfUserDefinedRateParameters() const = 0;

    /// @brief Get the vector of conditions
    /// @return Reference to vector of conditions
    virtual std::vector<micm::Conditions>& GetConditions() = 0;

    /// @brief Get the vector of conditions (const version)
    /// @return Const reference to vector of conditions
    virtual const std::vector<micm::Conditions>& GetConditions() const = 0;

    /// @brief Get the ordered concentrations vector
    /// @return Reference to the concentrations vector
    virtual std::vector<double>& GetOrderedConcentrations() = 0;

    /// @brief Get the ordered concentrations vector (const version)
    /// @return Const reference to the concentrations vector
    virtual const std::vector<double>& GetOrderedConcentrations() const = 0;

    /// @brief Get the ordered rate parameters vector
    /// @return Reference to the rate parameters vector
    virtual std::vector<double>& GetOrderedRateParameters() = 0;

    /// @brief Get the ordered rate parameters vector (const version)
    /// @return Const reference to the rate parameters vector
    virtual const std::vector<double>& GetOrderedRateParameters() const = 0;

    /// @brief Get the strides for the concentration matrix
    /// @return Pair of (row_stride, column_stride)
    virtual std::pair<std::size_t, std::size_t> GetConcentrationsStrides() const = 0;

    /// @brief Get the strides for the rate parameter matrix
    /// @return Pair of (row_stride, column_stride)
    virtual std::pair<std::size_t, std::size_t> GetRateParameterStrides() const = 0;

    /// @brief Get the variable (species) ordering map
    /// @return Map of species names to their indices
    virtual std::map<std::string, std::size_t> GetVariableMap() const = 0;

    /// @brief Get the rate parameter ordering map
    /// @return Map of rate parameter names to their indices
    virtual std::map<std::string, std::size_t> GetRateParameterMap() const = 0;
  };

}  // namespace musica
