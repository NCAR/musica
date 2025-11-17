// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
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
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error creating RadiatorMap: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return radiator_map_instance;
          }))
      .def(
          "add_radiator",
          [](musica::RadiatorMap& self, musica::Radiator* radiator)
          {
            musica::Error error;
            self.AddRadiator(radiator, &error);
            if (error.code_ != 0)
            {
              std::string message = "Error adding radiator: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw std::runtime_error(message);
            }
            DeleteError(&error);
          })
      .def(
          "get_radiator",
          [](musica::RadiatorMap& self, const std::string& name)
          {
            musica::Error error;
            musica::Radiator* radiator = self.GetRadiator(name.c_str(), &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting radiator: " + message);
            }
            musica::DeleteError(&error);
            return radiator;
          },
          py::return_value_policy::reference)
      .def(
          "get_radiator_by_index",
          [](musica::RadiatorMap& self, std::size_t index)
          {
            musica::Error error;
            musica::Radiator* radiator = self.GetRadiatorByIndex(index, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting radiator by index: " + message);
            }
            musica::DeleteError(&error);
            return radiator;
          },
          py::return_value_policy::reference)
      .def(
          "remove_radiator",
          [](musica::RadiatorMap& self, const std::string& name)
          {
            musica::Error error;
            self.RemoveRadiator(name.c_str(), &error);
            if (error.code_ != 0)
            {
              std::string message = "Error removing radiator: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            DeleteError(&error);
          })
      .def(
          "remove_radiator_by_index",
          [](musica::RadiatorMap& self, std::size_t index)
          {
            musica::Error error;
            self.RemoveRadiatorByIndex(index, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error removing radiator by index: " + message);
            }
            DeleteError(&error);
          })
      .def(
          "get_number_of_radiators",
          [](musica::RadiatorMap& self)
          {
            musica::Error error;
            std::size_t num_radiators = self.GetNumberOfRadiators(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting number of radiators: " + message);
            }
            musica::DeleteError(&error);
            return num_radiators;
          });
}
