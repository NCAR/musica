// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#pragma once

#include <musica/micm/micm.hpp>

#include <micm/CPU.hpp>
#ifdef MUSICA_ENABLE_CUDA
  #include <micm/GPU.hpp>
#endif

#include <any>
#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace musica
{
  class MICM;

  class State
  {
   public:
    State() = default;

    State(const musica::MICM& micm, std::size_t number_of_grid_cells);

    /// @brief Define the variant that holds all state types
    using StateVariant = std::variant<
        micm::VectorState,
        micm::StandardState
#ifdef MUSICA_ENABLE_CUDA
        ,
        micm::GpuState
#endif
        >;

    /// @brief Get the number of grid cells
    /// @return Number of grid cells
    std::size_t NumberOfGridCells();

    /// @brief Get the vector of conditions struct
    /// @return Vector of conditions struct
    std::vector<micm::Conditions>& GetConditions();

    /// @brief Set the conditions struct to the state variant
    /// @param conditions Vector of conditions
    void SetConditions(const std::vector<micm::Conditions>& conditions);

    /// @brief Get the vector of concentrations
    /// @return Vector of doubles
    std::vector<double>& GetOrderedConcentrations();

    /// @brief Set the concentrations to the state variant
    /// @param concentrations Vector of concentrations
    void SetOrderedConcentrations(const std::vector<double>& concentrations);

    /// @brief Get the vector of rate constants
    /// @return Vector of doubles
    std::vector<double>& GetOrderedRateConstants();

    /// @brief Set the rate constants to the state variant
    /// @param rateConstant Vector of Rate constants
    void SetOrderedRateConstants(const std::vector<double>& rateConstant);

    /// @brief Get the pointer to the conditions struct
    /// @param state Pointer to state object
    /// @param number_of_grid_cells Pointer to num of grid cells
    std::vector<micm::Conditions>* GetConditionsToState(musica::State* state, int* number_of_grid_cells);

    /// @brief Get the point to the vector of the concentrations
    /// @param state Pointer to state object
    /// @param number_of_species Pointer to number of species
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @return Pointer to the vector
    double* GetOrderedConcentrationsToState(musica::State* state, int* number_of_species, int* number_of_grid_cells);

    /// @brief Get the point to the vector of the rates
    /// @param state Pointer to state object
    /// @param number_of_species Pointer to number of rate constants
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @return Pointer to the vector
    double* GetOrderedRateConstantsToState(musica::State* state, int* number_of_rate_constants, int* number_of_grid_cells);

    /// @brief Get the underlying strides for the concentration matrix
    /// @return Strides for the concentration matrix (grid cells, species)
    std::pair<std::size_t, std::size_t> GetConcentrationStrides();

    /// @brief Get the underlying strides for the user-defined rate parameter matrix
    /// @return Strides for the rate parameter matrix (grid cells, rate parameters)
    std::pair<std::size_t, std::size_t> GetUserDefinedRateParameterStrides();

    StateVariant state_variant_;
  };

}  // namespace musica