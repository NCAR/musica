// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>

namespace py = pybind11;

void bind_micm_solver(py::module_ &m)
{
  py::enum_<musica::MICMSolver>(m, "_SolverType")
      .value("rosenbrock", musica::MICMSolver::Rosenbrock)
      .value("rosenbrock_standard_order", musica::MICMSolver::RosenbrockStandardOrder)
      .value("backward_euler", musica::MICMSolver::BackwardEuler)
      .value("backward_euler_standard_order", musica::MICMSolver::BackwardEulerStandardOrder)
      .value("cuda_rosenbrock", musica::MICMSolver::CudaRosenbrock);
}