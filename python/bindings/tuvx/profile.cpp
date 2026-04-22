// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the Python bindings for the TUV-x Profile class in the musica library.
#include "../common.hpp"

#include <musica/tuvx/grid.hpp>
#include <musica/tuvx/profile.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_tuvx_profile(py::module_ &profile)
{
  py::class_<musica::Profile>(profile, "_Profile")
      .def(py::init(
          [](const py::kwargs &kwargs)
          {
            if (!kwargs.contains("name"))
              throw py::value_error("Missing required argument: name");
            if (!kwargs.contains("units"))
              throw py::value_error("Missing required argument: units");
            if (!kwargs.contains("grid"))
              throw py::value_error("Missing required argument: grid");
            if (!py::isinstance<py::str>(kwargs["name"]))
              throw py::value_error("Argument 'name' must be a string");
            if (!py::isinstance<py::str>(kwargs["units"]))
              throw py::value_error("Argument 'units' must be a string");
            if (!py::isinstance<musica::Grid>(kwargs["grid"].cast<py::object>()))
              throw py::value_error("Argument 'grid' must be a Grid object");

            std::string name = kwargs["name"].cast<std::string>();
            std::string units = kwargs["units"].cast<std::string>();
            musica::Grid *grid = kwargs["grid"].cast<musica::Grid *>();

            musica::Error error;
            auto profile_instance = new musica::Profile(name.c_str(), units.c_str(), grid, &error);
            handle_error(error, "Error creating profile");
            return profile_instance;
          }))
      .def("__del__", [](musica::Profile &profile) {})
      .def_property_readonly(
          "name",
          [](musica::Profile &self)
          {
            musica::Error error;
            std::string name = self.GetName(&error);
            handle_error(error, "Error getting profile name");
            return name;
          },
          "The name of the profile")
      .def_property_readonly(
          "units",
          [](musica::Profile &self)
          {
            musica::Error error;
            std::string units = self.GetUnits(&error);
            handle_error(error, "Error getting profile units");
            return units;
          },
          "The units of the profile")
      .def_property_readonly(
          "number_of_sections",
          [](musica::Profile &self)
          {
            musica::Error error;
            size_t num_sections = self.GetNumberOfSections(&error);
            handle_error(error, "Error getting number of grid sections");
            return num_sections;
          },
          "The number of sections in the profile grid")
      .def_property(
          "edge_values",
          // Getter - converts C++ array to numpy array
          [](musica::Profile &self)
          {
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error) + 1;
            handle_error(error, "Error getting number of grid sections");

            // Get a pointer to the internal C++ array
            double *data_ptr = self.GetEdgeValuesPointer(&error);
            handle_error(error, "Error getting edge values pointer");

            // Create a numpy array that references the internal C++ array directly
            // using py::capsule to manage the lifetime correctly
            py::capsule owner = py::capsule(
                data_ptr,
                [](void *f)
                {
                  // No deletion here since the memory is managed by the Profile class
                });
            return py::array_t<double>(
                { size },            // shape
                { sizeof(double) },  // stride
                data_ptr,            // data pointer
                owner);
          },
          // Setter - converts numpy array to C++ array
          [](musica::Profile &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error) + 1;
            handle_error(error, "Error getting number of grid sections");
            if (buf.ndim != 1)
              throw py::value_error("Array must be one-dimensional");
            if (static_cast<size_t>(buf.size) != size)
              throw py::value_error("Array size must be num_sections + 1");
            double *ptr = static_cast<double *>(buf.ptr);
            self.SetEdgeValues(ptr, size, &error);
            handle_error(error, "Error setting edge values");
          },
          "Profile values at grid edges array of length num_sections + 1")
      .def_property(
          "midpoint_values",
          // Getter - converts C++ array to numpy array
          [](musica::Profile &self)
          {
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error);
            handle_error(error, "Error getting number of grid sections");

            // Get a pointer to the internal C++ array
            double *data_ptr = self.GetMidpointValuesPointer(&error);
            handle_error(error, "Error getting midpoint values pointer");

            // Create a numpy array that references the internal C++ array directly
            // using py::capsule to manage the lifetime correctly
            py::capsule owner = py::capsule(
                data_ptr,
                [](void *f)
                {
                  // No deletion here since the memory is managed by the Profile class
                });
            return py::array_t<double>(
                { size },            // shape
                { sizeof(double) },  // stride
                data_ptr,            // data pointer
                owner);
          },
          // Setter - converts numpy array to C++ array
          [](musica::Profile &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error);
            handle_error(error, "Error getting number of grid sections");
            if (buf.ndim != 1)
              throw py::value_error("Array must be one-dimensional");
            if (static_cast<size_t>(buf.size) != size)
              throw py::value_error("Array size must be num_sections");
            double *ptr = static_cast<double *>(buf.ptr);
            self.SetMidpointValues(ptr, size, &error);
            handle_error(error, "Error setting midpoint values");
          },
          "Profile values at grid midpoints array of length num_sections")
      .def_property(
          "layer_densities",
          // Getter - converts C++ array to numpy array
          [](musica::Profile &self)
          {
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error);
            handle_error(error, "Error getting number of grid sections");

            // Get a pointer to the internal C++ array
            double *data_ptr = self.GetLayerDensitiesPointer(&error);
            handle_error(error, "Error getting layer densities pointer");
            // Create a numpy array that references the internal C++ array directly
            // using py::capsule to manage the lifetime correctly
            py::capsule owner = py::capsule(
                data_ptr,
                [](void *f)
                {
                  // No deletion here since the memory is managed by the Profile class
                });
            return py::array_t<double>(
                { size },            // shape
                { sizeof(double) },  // stride
                data_ptr,            // data pointer
                owner);
          },
          // Setter - converts numpy array to C++ array
          [](musica::Profile &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error);
            handle_error(error, "Error getting number of grid sections");
            if (buf.ndim != 1)
              throw py::value_error("Array must be one-dimensional");
            if (static_cast<size_t>(buf.size) != size)
              throw py::value_error("Array size must be num_sections");
            double *ptr = static_cast<double *>(buf.ptr);
            self.SetLayerDensities(ptr, size, &error);
            handle_error(error, "Error setting layer densities");
          },
          "Layer densities array of length num_sections")
      .def_property(
          "exo_layer_density",
          // Getter
          [](musica::Profile &self)
          {
            musica::Error error;
            double density = self.GetExoLayerDensity(&error);
            handle_error(error, "Error getting exospheric layer density");
            return density;
          },
          // Setter
          [](musica::Profile &self, double density)
          {
            musica::Error error;
            self.SetExoLayerDensity(density, &error);
            handle_error(error, "Error setting exospheric layer density");
          },
          "Exospheric layer density")
      .def(
          "calculate_exo_layer_density",
          [](musica::Profile &self, double scale_height)
          {
            musica::Error error;
            self.CalculateExoLayerDensity(scale_height, &error);
            handle_error(error, "Error calculating exospheric layer density");
          },
          "Calculate the exospheric layer density using the given scale height",
          py::arg("scale_height"));
}