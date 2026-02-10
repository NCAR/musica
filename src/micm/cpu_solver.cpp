// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the CPU solver and state classes.
#include <musica/micm/cpu_solver.hpp>
#include <musica/micm/micm.hpp>
#include <musica/util.hpp>

#include <system_error>

namespace musica
{
  CpuState::CpuState(StateVariant state)
      : state_(std::move(state))
  {
  }

  std::size_t CpuState::NumberOfGridCells() const
  {
    return std::visit([](const auto& st) -> std::size_t { return st.NumberOfGridCells(); }, state_);
  }

  std::size_t CpuState::NumberOfSpecies() const
  {
    return std::visit([](const auto& st) -> std::size_t { return st.variables_.NumColumns(); }, state_);
  }

  std::size_t CpuState::NumberOfUserDefinedRateParameters() const
  {
    return std::visit([](const auto& st) -> std::size_t { return st.custom_rate_parameters_.NumColumns(); }, state_);
  }

  std::vector<micm::Conditions>& CpuState::GetConditions()
  {
    return std::visit([](auto& st) -> std::vector<micm::Conditions>& { return st.conditions_; }, state_);
  }

  const std::vector<micm::Conditions>& CpuState::GetConditions() const
  {
    return std::visit([](const auto& st) -> const std::vector<micm::Conditions>& { return st.conditions_; }, state_);
  }

  std::vector<double>& CpuState::GetOrderedConcentrations()
  {
    return std::visit([](auto& st) -> std::vector<double>& { return st.variables_.AsVector(); }, state_);
  }

  const std::vector<double>& CpuState::GetOrderedConcentrations() const
  {
    return std::visit([](const auto& st) -> const std::vector<double>& { return st.variables_.AsVector(); }, state_);
  }

  std::vector<double>& CpuState::GetOrderedRateParameters()
  {
    return std::visit([](auto& st) -> std::vector<double>& { return st.custom_rate_parameters_.AsVector(); }, state_);
  }

  const std::vector<double>& CpuState::GetOrderedRateParameters() const
  {
    return std::visit(
        [](const auto& st) -> const std::vector<double>& { return st.custom_rate_parameters_.AsVector(); }, state_);
  }

  std::pair<std::size_t, std::size_t> CpuState::GetConcentrationsStrides() const
  {
    return std::visit(
        [](const auto& st) -> std::pair<std::size_t, std::size_t>
        { return std::make_pair(st.variables_.RowStride(), st.variables_.ColumnStride()); },
        state_);
  }

  std::pair<std::size_t, std::size_t> CpuState::GetRateParameterStrides() const
  {
    return std::visit(
        [](const auto& st) -> std::pair<std::size_t, std::size_t>
        { return std::make_pair(st.custom_rate_parameters_.RowStride(), st.custom_rate_parameters_.ColumnStride()); },
        state_);
  }

  std::unordered_map<std::string, std::size_t> CpuState::GetVariableMap() const
  {
    return std::visit(
        [](const auto& st) -> std::unordered_map<std::string, std::size_t> { return st.variable_map_; }, state_);
  }

  std::unordered_map<std::string, std::size_t> CpuState::GetRateParameterMap() const
  {
    return std::visit(
        [](const auto& st) -> std::unordered_map<std::string, std::size_t> { return st.custom_rate_parameter_map_; },
        state_);
  }

  CpuState::StateVariant& CpuState::GetStateVariant()
  {
    return state_;
  }

  const CpuState::StateVariant& CpuState::GetStateVariant() const
  {
    return state_;
  }

  CpuSolver::CpuSolver(const Chemistry& chemistry, int solver_type)
      : solver_type_(solver_type)
  {
    auto configure = [&](auto builder)
    {
      auto solver =
          builder.SetSystem(chemistry.system).SetReactions(chemistry.processes).SetIgnoreUnusedSpecies(true).Build();
      return solver;
    };

    switch (static_cast<MICMSolver>(solver_type))
    {
      case MICMSolver::Rosenbrock:
        solver_ = std::make_unique<micm::Rosenbrock>(configure(
            micm::RosenbrockThreeStageBuilder(micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));
        break;

      case MICMSolver::RosenbrockStandardOrder:
        solver_ =
            std::make_unique<micm::RosenbrockStandard>(configure(micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));
        break;

      case MICMSolver::BackwardEuler:
        solver_ = std::make_unique<micm::BackwardEuler>(
            configure(micm::BackwardEulerBuilder(micm::BackwardEulerSolverParameters())));
        break;

      case MICMSolver::BackwardEulerStandardOrder:
        solver_ = std::make_unique<micm::BackwardEulerStandard>(
            configure(micm::CpuSolverBuilder<micm::BackwardEulerSolverParameters>(micm::BackwardEulerSolverParameters())));
        break;

      default:
        throw std::system_error(
            make_error_code(MusicaErrCode::SolverTypeNotFound),
            "Solver type " + ToString(static_cast<MICMSolver>(solver_type)) + " not supported by CpuSolver");
    }
  }

  /// @brief Visitor struct to handle different solver and state type combinations
  struct CpuSolverVisitor
  {
    double time_step;

    micm::SolverResult operator()(std::unique_ptr<micm::Rosenbrock>& solver, micm::VectorState& state) const
    {
      solver->CalculateRateConstants(state);
      return solver->Solve(time_step, state);
    }

    micm::SolverResult operator()(std::unique_ptr<micm::RosenbrockStandard>& solver, micm::StandardState& state) const
    {
      solver->CalculateRateConstants(state);
      return solver->Solve(time_step, state);
    }

    micm::SolverResult operator()(std::unique_ptr<micm::BackwardEuler>& solver, micm::VectorState& state) const
    {
      solver->CalculateRateConstants(state);
      return solver->Solve(time_step, state);
    }

    micm::SolverResult operator()(std::unique_ptr<micm::BackwardEulerStandard>& solver, micm::StandardState& state) const
    {
      solver->CalculateRateConstants(state);
      return solver->Solve(time_step, state);
    }

    // Handle unsupported combinations
    template<typename SolverT, typename StateT>
    micm::SolverResult operator()(std::unique_ptr<SolverT>&, StateT&) const
    {
      throw std::system_error(
          make_error_code(MusicaErrCode::UnsupportedSolverStatePair), "Unsupported solver/state combination in CpuSolver");
    }
  };

  micm::SolverResult CpuSolver::Solve(IState* state, double time_step)
  {
    auto* cpu_state = dynamic_cast<CpuState*>(state);
    if (!cpu_state)
    {
      throw std::system_error(
          make_error_code(MusicaErrCode::UnsupportedSolverStatePair), "State type incompatible with CpuSolver");
    }

    return std::visit(CpuSolverVisitor{ time_step }, solver_, cpu_state->GetStateVariant());
  }

  std::size_t CpuSolver::MaximumNumberOfGridCells() const
  {
    return std::visit([](const auto& solver) { return solver->MaximumNumberOfGridCells(); }, solver_);
  }

  std::unique_ptr<IState> CpuSolver::CreateState(std::size_t number_of_grid_cells)
  {
    // We need to handle each solver type explicitly because GetState returns different types
    return std::visit(
        [number_of_grid_cells](auto& solver) -> std::unique_ptr<IState>
        {
          auto state = solver->GetState(number_of_grid_cells);
          return std::make_unique<CpuState>(CpuState::StateVariant(std::move(state)));
        },
        solver_);
  }

  micm::System CpuSolver::GetSystem() const
  {
    return std::visit([](const auto& solver) -> micm::System { return solver->GetSystem(); }, solver_);
  }

  std::unordered_map<std::string, std::size_t> CpuSolver::GetSpeciesOrdering() const
  {
    return std::visit(
        [](const auto& solver) -> std::unordered_map<std::string, std::size_t>
        {
          auto state = solver->GetState(1);
          return state.variable_map_;
        },
        solver_);
  }

  std::unordered_map<std::string, std::size_t> CpuSolver::GetRateParameterOrdering() const
  {
    return std::visit(
        [](const auto& solver) -> std::unordered_map<std::string, std::size_t>
        {
          auto state = solver->GetState(1);
          return state.custom_rate_parameter_map_;
        },
        solver_);
  }

  std::size_t CpuSolver::GetVectorSize() const
  {
    switch (static_cast<MICMSolver>(solver_type_))
    {
      case MICMSolver::Rosenbrock:
      case MICMSolver::BackwardEuler: return MUSICA_VECTOR_SIZE;
      case MICMSolver::RosenbrockStandardOrder:
      case MICMSolver::BackwardEulerStandardOrder: return 1;
      default: throw std::runtime_error("Invalid solver type in CpuSolver::GetVectorSize");
    }
  }

}  // namespace musica
