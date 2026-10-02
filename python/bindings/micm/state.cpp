// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state.hpp>
#include <musica/micm/state_c_interface.hpp>

namespace py = pybind11;

void bind_micm_state(py::module_ &m)
{
  py::class_<musica::State, std::unique_ptr<musica::State, std::function<void(musica::State *)>>>(m, "_State")
      .def("number_of_grid_cells", [](musica::State &state) { return state.NumberOfGridCells(); })
      .def_property_readonly(
          "conditions",
          [](py::object self)
          {
            // Each element refers to the conditions in the state, so changes to an element change the state
            py::list conditions;
            for (auto &condition : self.cast<musica::State &>().GetConditions())
            {
              conditions.append(py::cast(&condition, py::return_value_policy::reference_internal, self));
            }
            return conditions;
          },
          "list of conditions structs for each grid cell")
      .def_property(
          "concentrations",
          [](musica::State &state) -> std::vector<double> & { return state.GetOrderedConcentrations(); },
          nullptr,
          "native 1D list of concentrations, ordered by species and grid cell according to matrix type")
      .def_property(
          "user_defined_rate_parameters",
          [](musica::State &state) -> std::vector<double> & { return state.GetOrderedRateParameters(); },
          nullptr,
          "native 1D list of user-defined rate parameters, ordered by parameter and grid cell according to matrix type")
      .def("concentration_strides", [](musica::State &state) { return state.GetConcentrationsStrides(); })
      .def(
          "user_defined_rate_parameter_strides",
          [](musica::State &state) { return state.GetUserDefinedRateParametersStrides(); });
}