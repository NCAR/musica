// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file declares the CUDA solver and state classes for the plugin.
#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/solver_interface.hpp>
#include <musica/micm/state_interface.hpp>

#include <micm/GPU.hpp>

#include <memory>

namespace musica
{
  namespace cuda
  {
    /// @brief CUDA state implementation wrapping GpuState
    class CudaState : public IState
    {
     public:
      explicit CudaState(micm::GpuState state);

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
      std::map<std::string, std::size_t> GetVariableMap() const override;
      std::map<std::string, std::size_t> GetRateParameterMap() const override;

      /// @brief Get access to the underlying GPU state for solving
      micm::GpuState& GetGpuState();

     private:
      micm::GpuState state_;
    };

    /// @brief CUDA Rosenbrock solver implementation
    class CudaRosenbrockSolver : public IMicmSolver
    {
     public:
      explicit CudaRosenbrockSolver(const Chemistry& chemistry);
      ~CudaRosenbrockSolver() override;

      micm::SolverResult Solve(IState* state, double time_step) override;
      std::size_t MaximumNumberOfGridCells() const override;
      std::unique_ptr<IState> CreateState(std::size_t number_of_grid_cells) override;
      micm::System GetSystem() const override;
      std::map<std::string, std::size_t> GetSpeciesOrdering() const override;
      std::map<std::string, std::size_t> GetRateParameterOrdering() const override;
      std::size_t GetVectorSize() const override;

     private:
      std::unique_ptr<micm::CudaRosenbrock> solver_;
    };

  }  // namespace cuda
}  // namespace musica
