// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file implements the CUDA solver and state classes for the plugin.
#include "cuda_solver.hpp"

#include <musica/util.hpp>

#include <system_error>

namespace musica
{
  namespace cuda
  {
    // CudaState implementation
    CudaState::CudaState(micm::GpuState state)
        : state_(std::move(state))
    {
    }

    std::size_t CudaState::NumberOfGridCells() const
    {
      return state_.NumberOfGridCells();
    }

    std::size_t CudaState::NumberOfSpecies() const
    {
      return state_.variables_.NumColumns();
    }

    std::size_t CudaState::NumberOfUserDefinedRateParameters() const
    {
      return state_.custom_rate_parameters_.NumColumns();
    }

    std::vector<micm::Conditions>& CudaState::GetConditions()
    {
      return state_.conditions_;
    }

    const std::vector<micm::Conditions>& CudaState::GetConditions() const
    {
      return state_.conditions_;
    }

    std::vector<double>& CudaState::GetOrderedConcentrations()
    {
      return state_.variables_.AsVector();
    }

    const std::vector<double>& CudaState::GetOrderedConcentrations() const
    {
      return state_.variables_.AsVector();
    }

    std::vector<double>& CudaState::GetOrderedRateParameters()
    {
      return state_.custom_rate_parameters_.AsVector();
    }

    const std::vector<double>& CudaState::GetOrderedRateParameters() const
    {
      return state_.custom_rate_parameters_.AsVector();
    }

    std::pair<std::size_t, std::size_t> CudaState::GetConcentrationsStrides() const
    {
      return std::make_pair(state_.variables_.RowStride(), state_.variables_.ColumnStride());
    }

    std::pair<std::size_t, std::size_t> CudaState::GetRateParameterStrides() const
    {
      return std::make_pair(state_.custom_rate_parameters_.RowStride(), state_.custom_rate_parameters_.ColumnStride());
    }

    std::unordered_map<std::string, std::size_t> CudaState::GetVariableMap() const
    {
      return state_.variable_map_;
    }

    std::unordered_map<std::string, std::size_t> CudaState::GetRateParameterMap() const
    {
      return state_.custom_rate_parameter_map_;
    }

    micm::GpuState& CudaState::GetGpuState()
    {
      return state_;
    }

    // CudaRosenbrockSolver implementation
    CudaRosenbrockSolver::CudaRosenbrockSolver(const Chemistry& chemistry)
    {
      auto builder =
          micm::GpuRosenbrockThreeStageBuilder(micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters());
      solver_ = std::make_unique<micm::CudaRosenbrock>(
          builder.SetSystem(chemistry.system).SetReactions(chemistry.processes).SetIgnoreUnusedSpecies(true).Build());
    }

    CudaRosenbrockSolver::~CudaRosenbrockSolver()
    {
      // Must reset solver before CUDA runtime cleanup
      solver_.reset();
    }

    micm::SolverResult CudaRosenbrockSolver::Solve(IState* state, double time_step)
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

    std::size_t CudaRosenbrockSolver::MaximumNumberOfGridCells() const
    {
      return solver_->MaximumNumberOfGridCells();
    }

    std::unique_ptr<IState> CudaRosenbrockSolver::CreateState(std::size_t number_of_grid_cells)
    {
      auto gpu_state = solver_->GetState(number_of_grid_cells);
      return std::make_unique<CudaState>(std::move(gpu_state));
    }

    micm::System CudaRosenbrockSolver::GetSystem() const
    {
      return solver_->GetSystem();
    }

    std::unordered_map<std::string, std::size_t> CudaRosenbrockSolver::GetSpeciesOrdering() const
    {
      auto state = solver_->GetState(1);
      return state.variable_map_;
    }

    std::unordered_map<std::string, std::size_t> CudaRosenbrockSolver::GetRateParameterOrdering() const
    {
      auto state = solver_->GetState(1);
      return state.custom_rate_parameter_map_;
    }

    std::size_t CudaRosenbrockSolver::GetVectorSize() const
    {
      return MUSICA_VECTOR_SIZE;
    }

  }  // namespace cuda
}  // namespace musica
