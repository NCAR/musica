// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/tuvx/radiator.hpp>
#include <musica/tuvx/radiator_map.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_tuvx_radiator_map(py::module& m)
{
  py::class_<musica::RadiatorMap>(m, "_RadiatorMap")
      .def(py::init<>(
          []()
          {
            musica::Error error;
            auto radiator_map_instance = new musica::RadiatorMap(&error);
            handle_error(error, "Error creating RadiatorMap");
            return radiator_map_instance;
          }))
      .def(
          "add_radiator",
          [](musica::RadiatorMap& self, musica::Radiator* radiator)
          {
            musica::Error error;
            self.AddRadiator(radiator, &error);
            handle_error(error, "Error adding radiator");
          })
      .def(
          "get_radiator",
          [](musica::RadiatorMap& self, const std::string& name)
          {
            musica::Error error;
            musica::Radiator* radiator = self.GetRadiator(name.c_str(), &error);
            handle_error(error, "Error getting radiator");
            return radiator;
          },
          py::return_value_policy::reference)
      .def(
          "get_radiator_by_index",
          [](musica::RadiatorMap& self, std::size_t index)
          {
            musica::Error error;
            musica::Radiator* radiator = self.GetRadiatorByIndex(index, &error);
            handle_error(error, "Error getting radiator by index");
            return radiator;
          },
          py::return_value_policy::reference)
      .def(
          "remove_radiator",
          [](musica::RadiatorMap& self, const std::string& name)
          {
            musica::Error error;
            self.RemoveRadiator(name.c_str(), &error);
            handle_error(error, "Error removing radiator");
          })
      .def(
          "remove_radiator_by_index",
          [](musica::RadiatorMap& self, std::size_t index)
          {
            musica::Error error;
            self.RemoveRadiatorByIndex(index, &error);
            handle_error(error, "Error removing radiator by index");
          })
      .def(
          "get_number_of_radiators",
          [](musica::RadiatorMap& self)
          {
            musica::Error error;
            std::size_t num_radiators = self.GetNumberOfRadiators(&error);
            handle_error(error, "Error getting number of radiators");
            return num_radiators;
          });
}
