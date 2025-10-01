// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "binding_common.hpp"

#include <musica/tuvx/profile.hpp>
#include <musica/tuvx/profile_map.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_tuvx_profile_map(py::module& m)
{
  py::class_<musica::ProfileMap>(m, "_ProfileMap")
      .def(py::init<>(
          []()
          {
            musica::Error error;
            auto profile_map_instance = new musica::ProfileMap(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error creating ProfileMap: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return profile_map_instance;
          }))
      .def(
          "add_profile",
          [](musica::ProfileMap& self, musica::Profile* profile)
          {
            musica::Error error;
            self.AddProfile(profile, &error);
            if (error.code_ != 0)
            {
              std::string message = "Error adding profile: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw std::runtime_error(message);
            }
            DeleteError(&error);
          })
      .def(
          "get_profile",
          [](musica::ProfileMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            musica::Profile* profile = self.GetProfile(name.c_str(), units.c_str(), &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting profile: " + message);
            }
            musica::DeleteError(&error);
            return profile;
          },
          py::return_value_policy::reference)
      .def(
          "get_profile_by_index",
          [](musica::ProfileMap& self, std::size_t index)
          {
            musica::Error error;
            musica::Profile* profile = self.GetProfileByIndex(index, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting profile by index: " + message);
            }
            musica::DeleteError(&error);
            return profile;
          },
          py::return_value_policy::reference)
      .def(
          "remove_profile",
          [](musica::ProfileMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            self.RemoveProfile(name.c_str(), units.c_str(), &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error removing profile: " + message);
            }
            musica::DeleteError(&error);
          })
      .def(
          "remove_profile_by_index",
          [](musica::ProfileMap& self, std::size_t index)
          {
            musica::Error error;
            self.RemoveProfileByIndex(index, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error removing profile by index: " + message);
            }
            musica::DeleteError(&error);
          })
      .def(
          "get_number_of_profiles",
          [](musica::ProfileMap& self)
          {
            musica::Error error;
            std::size_t num_profiles = self.GetNumberOfProfiles(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting number of profiles: " + message);
            }
            musica::DeleteError(&error);
            return num_profiles;
          });
}