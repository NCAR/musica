// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines parameter structs for configuring MICM solver behavior.
#pragma once

#include <cstddef>
#include <vector>

namespace musica
{
  /// @brief Parameters for configuring Rosenbrock solvers
  struct RosenbrockSolverParameters
  {
    double relative_tolerance{ 1e-6 };
    std::vector<double> absolute_tolerances{};  // empty = use MICM defaults
    double h_min{ 0.0 };
    double h_max{ 0.0 };
    double h_start{ 0.0 };
    std::size_t max_number_of_steps{ 1000 };
  };

  /// @brief Parameters for configuring Backward Euler solvers
  struct BackwardEulerSolverParameters
  {
    double relative_tolerance{ 1e-6 };
    std::vector<double> absolute_tolerances{};  // empty = use MICM defaults
    std::size_t max_number_of_steps{ 11 };
    std::vector<double> time_step_reductions{ 0.5, 0.5, 0.5, 0.5, 0.1 };
  };

}  // namespace musica
