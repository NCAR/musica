// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<micm::Conditions>)

void bind_micm_conditions(py::module_ &m)
{
  py::bind_vector<std::vector<micm::Conditions>>(m, "VectorConditions");

  py::class_<micm::Conditions>(m, "_Conditions")
      .def(py::init<>())
      .def_readwrite("temperature", &micm::Conditions::temperature_)
      .def_readwrite("pressure", &micm::Conditions::pressure_)
      .def_readwrite("air_density", &micm::Conditions::air_density_);
}