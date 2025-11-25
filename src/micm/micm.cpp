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
        solver_variant_ = std::make_unique<micm::RosenbrockStandard>(configure(
            micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
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

  /// @brief Concept for states that support GPU synchronization
  template<class State>
  concept GpuState = requires(State& st) {
    st.SyncInputsToDevice();
    st.SyncOutputsToHost();
  };

  /// @brief Concept for solver-state pairs that support basic solving
  template<class Solver, class State>
  concept BasicSolvable = requires(Solver& s, State& st, double dt) {
    s.CalculateRateConstants(st);
    s.Solve(dt, st);
  };

  /// @brief Visitor struct to handle different solver and state types
  struct VariantsVisitor
  {
    double dt;

    template<class Solver, class State>
      requires BasicSolvable<Solver, State>
    micm::SolverResult operator()(std::unique_ptr<Solver>& sp, State& st) const
    {
      sp->CalculateRateConstants(st);
      return sp->Solve(dt, st);
    }

    // CUDA specialization
    template<class Solver, class State>
      requires GpuState<State>
    micm::SolverResult operator()(std::unique_ptr<Solver>& sp, State& st) const
    {
      sp->CalculateRateConstants(st);
      st.SyncInputsToDevice();
      auto r = sp->Solve(dt, st);
      st.SyncOutputsToHost();
      return r;
    }

    // Fallback
    template<class S, class St>
    micm::SolverResult operator()(std::unique_ptr<S>&, St&) const
    {
      throw std::system_error(make_error_code(MusicaErrCode::UnsupportedSolverStatePair));
    }
  };

  micm::SolverResult MICM::Solve(musica::State* state, double dt)
  {
    return std::visit(VariantsVisitor{ dt }, solver_variant_, state->state_variant_);
  }

}  // namespace musica
