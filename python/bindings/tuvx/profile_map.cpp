// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

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
            handle_error(error, "Error creating ProfileMap");
            return profile_map_instance;
          }))
      .def(
          "add_profile",
          [](musica::ProfileMap& self, musica::Profile* profile)
          {
            musica::Error error;
            self.AddProfile(profile, &error);
            handle_error(error, "Error adding profile");
          })
      .def(
          "get_profile",
          [](musica::ProfileMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            musica::Profile* profile = self.GetProfile(name.c_str(), units.c_str(), &error);
            handle_error(error, "Error getting profile");
            return profile;
          },
          py::return_value_policy::reference)
      .def(
          "get_profile_by_index",
          [](musica::ProfileMap& self, std::size_t index)
          {
            musica::Error error;
            musica::Profile* profile = self.GetProfileByIndex(index, &error);
            handle_error(error, "Error getting profile by index");
            return profile;
          },
          py::return_value_policy::reference)
      .def(
          "remove_profile",
          [](musica::ProfileMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            self.RemoveProfile(name.c_str(), units.c_str(), &error);
            handle_error(error, "Error removing profile");
          })
      .def(
          "remove_profile_by_index",
          [](musica::ProfileMap& self, std::size_t index)
          {
            musica::Error error;
            self.RemoveProfileByIndex(index, &error);
            handle_error(error, "Error removing profile by index");
          })
      .def(
          "get_number_of_profiles",
          [](musica::ProfileMap& self)
          {
            musica::Error error;
            std::size_t num_profiles = self.GetNumberOfProfiles(&error);
            handle_error(error, "Error getting number of profiles");
            return num_profiles;
          });
}