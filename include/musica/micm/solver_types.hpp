// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Concrete solver types used by CpuSolver.
//
// These are deduced from the MICM builders for two reasons:
//
//  1. SolverBuilder::Build() wraps its rates and constraints policies in
//     RatesBundle/ConstraintBundle, so the built type does not match the
//     micm::Rosenbrock / micm::BackwardEuler aliases in micm/CPU.hpp.
//  2. AddExternalModel() extends the builder's template parameter pack with the
//     concrete model type, so a solver carrying a MIAM model is a distinct type
//     from the chemistry-only solver and needs its own name.
#pragma once

#include <micm/CPU.hpp>
#include <micm/solver/backward_euler_solver_parameters.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>

#ifdef MUSICA_USE_MIAM
  #include <miam/miam.hpp>
#endif

#include <utility>

namespace musica::solvers
{
  /// @brief The solver type Builder produces
  template<class Builder>
  using SolverOf = decltype(std::declval<Builder>().Build());

  using RosenbrockStandardBuilder = micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>;
  using BackwardEulerStandardBuilder = micm::CpuSolverBuilder<micm::BackwardEulerSolverParameters>;

  using Rosenbrock = SolverOf<micm::RosenbrockThreeStageBuilder>;
  using RosenbrockStandard = SolverOf<RosenbrockStandardBuilder>;
  using BackwardEuler = SolverOf<micm::BackwardEulerBuilder>;
  using BackwardEulerStandard = SolverOf<BackwardEulerStandardBuilder>;

#ifdef MUSICA_USE_MIAM
  /// @brief The solver type Builder produces with a miam::Model attached
  template<class Builder>
  using MiamSolverOf = decltype(std::declval<Builder>().AddExternalModel(std::declval<miam::Model>()).Build());

  using MiamRosenbrock = MiamSolverOf<micm::RosenbrockThreeStageBuilder>;
  using MiamRosenbrockStandard = MiamSolverOf<RosenbrockStandardBuilder>;
  using MiamBackwardEuler = MiamSolverOf<micm::BackwardEulerBuilder>;
  using MiamBackwardEulerStandard = MiamSolverOf<BackwardEulerStandardBuilder>;
#endif
}  // namespace musica::solvers
