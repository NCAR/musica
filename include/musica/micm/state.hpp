// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#pragma once

#include <musica/util.hpp>

#include <micm/process/process_set.hpp>
#include <micm/solver/rosenbrock.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver.hpp>
#include <micm/solver/solver_builder.hpp>
#include <micm/util/matrix.hpp>
#include <micm/util/sparse_matrix_vector_ordering.hpp>
#include <micm/util/vector_matrix.hpp>

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

  /// @brief Defines matrix types for vector-based and standard matrices.
  using DenseMatrixVector = micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>;
  using SparseMatrixVector = micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>;
  using VectorState = micm::State<DenseMatrixVector, SparseMatrixVector>;
  using DenseMatrixStandard = micm::Matrix<double>;
  using SparseMatrixStandard = micm::SparseMatrix<double, micm::SparseMatrixStandardOrdering>;
  using StandardState = micm::State<DenseMatrixStandard, SparseMatrixStandard>;

  /// @brief Create a state object by specifying micm solver object using the solver variant
  /// @param micm Pointer to MICM object
  /// @param error Error struct to indicate success or failure
  State *CreateMicmState(musica::MICM *micm, Error *error);

  /// @brief Deletes a state object
  /// @param state Pointer to state object
  /// @param error Error struct to indicate success or failure
  void DeleteState(const State *state, Error *error);

  class State
  {
   public:
    State() = default;

    /// @brief Define the variant that holds all state types
    using StateVariant = std::variant<VectorState, StandardState>;

    /// @brief Get the vector of conditions struct
    std::vector<micm::Conditions> &GetConditions();

    /// @brief Set the conditions struct to the state variant
    /// @param conditions vector of conditions
    void SetConditions(const std::vector<micm::Conditions> &conditions);

    /// @brief Get the vector of concentrations
    std::vector<double> &GetOrderedConcentrations();

    /// @brief Set the concentrations to the state variant
    /// @param concentrations vector of concentrations
    void SetOrderedConcentrations(const std::vector<double> &concentrations);

    /// @brief Temporary method to set the concentrations to the state variant for Fortran code.
    /// @param concentrations c pointer list of concentrations
    void SetOrderedConcentrations(const double *concentrations);

    /// @brief Get the vector of rate constants
    std::vector<double> &GetOrderedRateConstants();

    /// @brief Set the rate constants to the state variant
    /// @param rateConstant vector of Rate constants
    void SetOrderedRateConstants(const std::vector<double> &rateConstant);

    /// @brief Temporary method to set the rate constants to the state variant for Fortran code.
    /// @param rateConstant c pointer list of rate constants
    void SetOrderedRateConstants(const double *rateConstant);

    StateVariant state_variant_;
  };

}  // namespace musica