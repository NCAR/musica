// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.
#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

#include <mechanism_configuration/parser.hpp>
#include <mechanism_configuration/v0/types.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

namespace musica
{
  MICM::MICM(const Chemistry& chemistry, MICMSolver solver_type, int num_grid_cells)
  {
    auto configure = [&](auto builder)
    {
      auto solver = builder.SetSystem(chemistry.system)
                        .SetReactions(chemistry.processes)
                        .SetNumberOfGridCells(num_grid_cells)
                        .SetIgnoreUnusedSpecies(true)
                        .Build();

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

      default:
        throw std::system_error(make_error_code(MusicaErrc::SolverTypeNotFound), "Solver type not found");
    }

    auto system = std::visit([](auto& solver) -> micm::System { return solver->GetSystem(); }, solver_variant_);
  }

  /// @brief Visitor struct to handle different solver and state types
  struct VariantsVisitor
  {
    double time_step;
    String* solver_state;
    SolverResultStats* solver_stats;

    template<typename SolverType, typename StateType>
    void Solve(SolverType& solver, StateType& state) const
    {
      solver->CalculateRateConstants(state);
      auto result = solver->Solve(time_step, state);

      *solver_state = CreateString(micm::SolverStateToString(result.state_).c_str());
      *solver_stats = SolverResultStats(
          result.stats_.function_calls_,
          result.stats_.jacobian_updates_,
          result.stats_.number_of_steps_,
          result.stats_.accepted_,
          result.stats_.rejected_,
          result.stats_.decompositions_,
          result.stats_.solves_,
          result.final_time_);
    }

    void operator()(std::unique_ptr<micm::Rosenbrock>& solver, micm::VectorState& state) const
    {
      Solve(solver, state);
    }

    void operator()(std::unique_ptr<micm::RosenbrockStandard>& solver, micm::StandardState& state) const
    {
      Solve(solver, state);
    }

    void operator()(std::unique_ptr<micm::BackwardEuler>& solver, micm::VectorState& state) const
    {
      Solve(solver, state);
    }

    void operator()(std::unique_ptr<micm::BackwardEulerStandard>& solver, micm::StandardState& state) const
    {
      Solve(solver, state);
    }

    // Handle unsupported combinations
    template<typename SolverT, typename StateT>
    void operator()(std::unique_ptr<SolverT>&, StateT&) const
    {
      throw std::system_error(make_error_code(MusicaErrc::UnsupportedSolverStatePair), "Unsupported solver/state combination");
    }
  };

  void MICM::Solve(MICM* micm, musica::State* state, double time_step, String* solver_state, SolverResultStats* solver_stats)
  {
    std::visit(VariantsVisitor{ time_step, solver_state, solver_stats }, micm->solver_variant_, state->state_variant_);
  }

}  // namespace musica
