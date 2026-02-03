// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file implements the CUDA solver and state classes for the plugin.
#include <musica/micm/chemistry.hpp>
#include <musica/micm/solver_interface.hpp>
#include <musica/micm/state_interface.hpp>
#include <musica/util.hpp>

#include <micm/GPU.hpp>

#include <memory>
#include <system_error>

namespace musica
{
  namespace cuda
  {
    /// @brief CUDA state implementation wrapping GpuState
    class CudaState : public IState
    {
     public:
      explicit CudaState(micm::GpuState state)
          : state_(std::move(state))
      {
      }

      std::size_t NumberOfGridCells() const override
      {
        return state_.NumberOfGridCells();
      }

      std::size_t NumberOfSpecies() const override
      {
        return state_.variables_.NumColumns();
      }

      std::size_t NumberOfUserDefinedRateParameters() const override
      {
        return state_.custom_rate_parameters_.NumColumns();
      }

      std::vector<micm::Conditions>& GetConditions() override
      {
        return state_.conditions_;
      }

      const std::vector<micm::Conditions>& GetConditions() const override
      {
        return state_.conditions_;
      }

      std::vector<double>& GetOrderedConcentrations() override
      {
        return state_.variables_.AsVector();
      }

      const std::vector<double>& GetOrderedConcentrations() const override
      {
        return state_.variables_.AsVector();
      }

      std::vector<double>& GetOrderedRateParameters() override
      {
        return state_.custom_rate_parameters_.AsVector();
      }

      const std::vector<double>& GetOrderedRateParameters() const override
      {
        return state_.custom_rate_parameters_.AsVector();
      }

      std::pair<std::size_t, std::size_t> GetConcentrationsStrides() const override
      {
        return std::make_pair(state_.variables_.RowStride(), state_.variables_.ColumnStride());
      }

      std::pair<std::size_t, std::size_t> GetRateParameterStrides() const override
      {
        return std::make_pair(state_.custom_rate_parameters_.RowStride(), state_.custom_rate_parameters_.ColumnStride());
      }

      std::map<std::string, std::size_t> GetVariableMap() const override
      {
        return state_.variable_map_;
      }

      std::map<std::string, std::size_t> GetRateParameterMap() const override
      {
        return state_.custom_rate_parameter_map_;
      }

      /// @brief Get access to the underlying GPU state for solving
      micm::GpuState& GetGpuState()
      {
        return state_;
      }

     private:
      micm::GpuState state_;
    };

    /// @brief CUDA Rosenbrock solver implementation
    class CudaRosenbrockSolver : public IMicmSolver
    {
     public:
      explicit CudaRosenbrockSolver(const Chemistry& chemistry)
      {
        auto builder =
            micm::GpuRosenbrockThreeStageBuilder(micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters());
        solver_ = std::make_unique<micm::CudaRosenbrock>(
            builder.SetSystem(chemistry.system).SetReactions(chemistry.processes).SetIgnoreUnusedSpecies(true).Build());
      }

      ~CudaRosenbrockSolver() override
      {
        // Must reset solver before CUDA runtime cleanup
        solver_.reset();
      }

      micm::SolverResult Solve(IState* state, double time_step) override
      {
        auto* cuda_state = dynamic_cast<CudaState*>(state);
        if (!cuda_state)
        {
          micm::SolverResult result;
          result.state_ = micm::SolverState::NaNDetected;
          return result;
        }

        auto& gpu_state = cuda_state->GetGpuState();
        solver_->CalculateRateConstants(gpu_state);
        gpu_state.SyncInputsToDevice();
        auto result = solver_->Solve(time_step, gpu_state);
        gpu_state.SyncOutputsToHost();
        return result;
      }

      std::size_t MaximumNumberOfGridCells() const override
      {
        return solver_->MaximumNumberOfGridCells();
      }

      std::unique_ptr<IState> CreateState(std::size_t number_of_grid_cells) override
      {
        auto gpu_state = solver_->GetState(number_of_grid_cells);
        return std::make_unique<CudaState>(std::move(gpu_state));
      }

      micm::System GetSystem() const override
      {
        return solver_->GetSystem();
      }

      std::map<std::string, std::size_t> GetSpeciesOrdering() const override
      {
        auto state = solver_->GetState(1);
        return state.variable_map_;
      }

      std::map<std::string, std::size_t> GetRateParameterOrdering() const override
      {
        auto state = solver_->GetState(1);
        return state.custom_rate_parameter_map_;
      }

      std::size_t GetVectorSize() const override
      {
        return MUSICA_VECTOR_SIZE;
      }

     private:
      std::unique_ptr<micm::CudaRosenbrock> solver_;
    };

  }  // namespace cuda
}  // namespace musica
