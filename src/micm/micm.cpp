// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.
#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/state.hpp>

#include <mechanism_configuration/parser.hpp>
#include <mechanism_configuration/v0/types.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

namespace musica
{
  std::string ToString(MICMSolver solver_type)
  {
    switch (solver_type)
    {
      case UndefinedSolver: return "UndefinedSolver";
      case Rosenbrock: return "Rosenbrock";
      case RosenbrockStandardOrder: return "RosenbrockStandardOrder";
      case BackwardEuler: return "BackwardEuler";
      case BackwardEulerStandardOrder: return "BackwardEulerStandardOrder";
      case CudaRosenbrock: return "CudaRosenbrock";
      default: throw std::system_error(make_error_code(MusicaErrCode::Unknown), "Unknown solver type");
    }
  }

  MICM::MICM(const Chemistry& chemistry, MICMSolver solver_type)
  {
    auto configure = [&](auto builder)
    {
      auto solver =
          builder.SetSystem(chemistry.system).SetReactions(chemistry.processes).SetIgnoreUnusedSpecies(true).Build();

      return solver;
    };

    switch (solver_type)
    {
      case MICMSolver::Rosenbrock:
        solver_variant_ = std::make_unique<micm::Rosenbrock>(configure(
            micm::RosenbrockThreeStageBuilder(micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));
        break;

      case MICMSolver::RosenbrockStandardOrder:
        solver_variant_ =
            std::make_unique<micm::RosenbrockStandard>(configure(micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));
        break;

      case MICMSolver::BackwardEuler:
        solver_variant_ = std::make_unique<micm::BackwardEuler>(
            configure(micm::BackwardEulerBuilder(micm::BackwardEulerSolverParameters())));
        break;

      case MICMSolver::BackwardEulerStandardOrder:
        solver_variant_ = std::make_unique<micm::BackwardEulerStandard>(
            configure(micm::CpuSolverBuilder<micm::BackwardEulerSolverParameters>(micm::BackwardEulerSolverParameters())));
        break;

#ifdef MUSICA_ENABLE_CUDA
      case MICMSolver::CudaRosenbrock:
        solver_variant_ = std::make_unique<micm::CudaRosenbrock>(configure(
            micm::GpuRosenbrockThreeStageBuilder(micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));
        break;
#endif

      default:
        std::string const msg = "Solver type " + ToString(solver_type) + " not supported in this build";
        throw std::system_error(make_error_code(MusicaErrCode::SolverTypeNotFound), msg);
    }
  }

  /// @brief Visitor struct to handle different solver and state types
  struct VariantsVisitor
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

#ifdef MUSICA_ENABLE_CUDA
    micm::SolverResult operator()(std::unique_ptr<micm::CudaRosenbrock>& solver, micm::GpuState& state) const
    {
      solver->CalculateRateConstants(state);
      state.SyncInputsToDevice();
      auto result = solver->Solve(time_step, state);
      state.SyncOutputsToHost();
      return result;
    }
#endif

    // Handle unsupported combinations
    template<typename SolverT, typename StateT>
    micm::SolverResult operator()(std::unique_ptr<SolverT>&, StateT&) const
    {
      throw std::system_error(
          make_error_code(MusicaErrCode::UnsupportedSolverStatePair), "Unsupported solver/state combination");
    }
  };

  micm::SolverResult MICM::Solve(musica::State* state, double time_step)
  {
    return std::visit(VariantsVisitor{ time_step }, this->solver_variant_, state->state_variant_);
  }

}  // namespace musica
