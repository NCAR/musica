// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the CPU solver implementations for type-erased MICM.
#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/solver_interface.hpp>
#include <musica/micm/state_interface.hpp>

#include <micm/CPU.hpp>

#include <memory>
#include <variant>

namespace musica
{
  // Forward declarations
  class CpuState;

  /// @brief CPU state implementation wrapping VectorState or StandardState
  class CpuState : public IState
  {
   public:
    using StateVariant = std::variant<micm::VectorState, micm::StandardState>;

    explicit CpuState(StateVariant state);

    std::size_t NumberOfGridCells() const override;
    std::size_t NumberOfSpecies() const override;
    std::size_t NumberOfUserDefinedRateParameters() const override;
    std::vector<micm::Conditions>& GetConditions() override;
    const std::vector<micm::Conditions>& GetConditions() const override;
    std::vector<double>& GetOrderedConcentrations() override;
    const std::vector<double>& GetOrderedConcentrations() const override;
    std::vector<double>& GetOrderedRateParameters() override;
    const std::vector<double>& GetOrderedRateParameters() const override;
    std::pair<std::size_t, std::size_t> GetConcentrationsStrides() const override;
    std::pair<std::size_t, std::size_t> GetRateParameterStrides() const override;
    std::unordered_map<std::string, std::size_t> GetVariableMap() const override;
    std::unordered_map<std::string, std::size_t> GetRateParameterMap() const override;

    /// @brief Get access to the underlying state variant for solving
    StateVariant& GetStateVariant();
    const StateVariant& GetStateVariant() const;

   private:
    StateVariant state_;
  };

  /// @brief CPU solver implementation using internal variant
  class CpuSolver : public IMicmSolver
  {
   public:
    using SolverVariant = std::variant<
        std::unique_ptr<micm::Rosenbrock>,
        std::unique_ptr<micm::RosenbrockStandard>,
        std::unique_ptr<micm::BackwardEuler>,
        std::unique_ptr<micm::BackwardEulerStandard>>;

    /// @brief Construct a CPU solver from chemistry configuration
    /// @param chemistry The chemistry configuration
    /// @param solver_type The type of solver to create
    CpuSolver(const Chemistry& chemistry, int solver_type);

    micm::SolverResult Solve(IState* state, double time_step) override;
    std::size_t MaximumNumberOfGridCells() const override;
    std::unique_ptr<IState> CreateState(std::size_t number_of_grid_cells) override;
    micm::System GetSystem() const override;
    std::unordered_map<std::string, std::size_t> GetSpeciesOrdering() const override;
    std::unordered_map<std::string, std::size_t> GetRateParameterOrdering() const override;
    std::size_t GetVectorSize() const override;

   private:
    SolverVariant solver_;
    int solver_type_;
  };

}  // namespace musica
