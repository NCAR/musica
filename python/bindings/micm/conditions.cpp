// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>

#include <span>

namespace py = pybind11;

void bind_micm_conditions(py::module_ &m)
{
  // State::GetConditions() hands back a view, because MICM stores conditions in a padded container.
  py::class_<std::span<micm::Conditions>>(m, "ConditionsView")
      .def("__len__", [](const std::span<micm::Conditions> &c) { return c.size(); })
      .def(
          "__getitem__",
          [](const std::span<micm::Conditions> &c, std::size_t i) -> micm::Conditions &
          {
            if (i >= c.size())
              throw py::index_error();
            return c[i];
          },
          py::return_value_policy::reference_internal)
      .def(
          "__iter__",
          [](const std::span<micm::Conditions> &c) { return py::make_iterator(c.begin(), c.end()); },
          py::keep_alive<0, 1>());

  py::class_<micm::Conditions>(m, "_Conditions")
      .def(py::init<>())
      .def_readwrite("temperature", &micm::Conditions::temperature_)
      .def_readwrite("pressure", &micm::Conditions::pressure_)
      .def_readwrite("air_density", &micm::Conditions::air_density_);
}