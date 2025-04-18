// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#pragma once

#include <musica/util.hpp>

#include <micm/CPU.hpp>

#include <any>
#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#ifndef MICM_VECTOR_MATRIX_SIZE
  #define MICM_VECTOR_MATRIX_SIZE 4
#endif

namespace musica
{

  class MICM;
  class State;

#ifdef __cplusplus
  extern "C"
  {
#endif

    /// @brief Create a state object by specifying micm solver object using the solver variant
    /// @param micm Pointer to MICM object
    /// @param error Error struct to indicate success or failure
    State* CreateMicmState(musica::MICM* micm, Error* error);

    /// @brief Deletes a state object
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    void DeleteState(const State* state, Error* error);

    /// @brief Defines matrix types for vector-based and standard matrices.
    using DenseMatrixVector = micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>;
    using SparseMatrixVector = micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>;
    using VectorState = micm::State<DenseMatrixVector, SparseMatrixVector>;
    using DenseMatrixStandard = micm::Matrix<double>;
    using SparseMatrixStandard = micm::SparseMatrix<double, micm::SparseMatrixStandardOrdering>;
    using StandardState = micm::State<DenseMatrixStandard, SparseMatrixStandard>;

    using ConditionsVector = std::vector<micm::Conditions>;

    /// @brief Get the pointer to the conditions struct
    /// @param state Pointer to state object
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    micm::Conditions* GetConditionsToStateFortran(musica::State* state, int* number_of_grid_cells, Error* error);

    /// @brief Get the point to the vector of the concentrations for Fortran interface
    /// @param state Pointer to state object
    /// @param number_of_species Pointer to number of species
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the vector
    double* GetOrderedConcentrationsToStateFortran(
        musica::State* state,
        int* number_of_species,
        int* number_of_grid_cells,
        Error* error);

    /// @brief Get the point to the vector of the rates for Fortran interface
    /// @param state Pointer to state object
    /// @param number_of_species Pointer to number of rate constants
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the vector
    double* GetOrderedRateConstantsToStateFortran(
        musica::State* state,
        int* number_of_rate_constants,
        int* number_of_grid_cells,
        Error* error);

#ifdef __cplusplus
  }
#endif

  class State
  {
   public:
    State() = default;

    /// @brief Define the variant that holds all state types
    using StateVariant = std::variant<micm::VectorState, micm::StandardState>;

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
    /// @param error Error struct to indicate success or failure
    ConditionsVector* GetConditionsToState(musica::State* state, int* number_of_grid_cells, Error* error);

    /// @brief Get the point to the vector of the concentrations
    /// @param state Pointer to state object
    /// @param number_of_species Pointer to number of species
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the vector
    double*
    GetOrderedConcentrationsToState(musica::State* state, int* number_of_species, int* number_of_grid_cells, Error* error);

    /// @brief Get the point to the vector of the rates
    /// @param state Pointer to state object
    /// @param number_of_species Pointer to number of rate constants
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the vector
    double* GetOrderedRateConstantsToState(
        musica::State* state,
        int* number_of_rate_constants,
        int* number_of_grid_cells,
        Error* error);

    StateVariant state_variant_;
  };

}  // namespace musica