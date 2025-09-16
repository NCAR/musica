// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the Python bindings for the TUV-x Grid class in the musica library.
#include "binding_common.hpp"

#include <musica/tuvx/grid.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_tuvx_grid(py::module_ &grid)
{
  py::class_<musica::Grid>(grid, "_Grid")
      .def(py::init(
          [](const py::kwargs &kwargs)
          {
            if (!kwargs.contains("name"))
              throw py::value_error("Missing required argument: name");
            if (!kwargs.contains("units"))
              throw py::value_error("Missing required argument: units");
            if (!kwargs.contains("num_sections"))
              throw py::value_error("Missing required argument: num_sections");
            if (!py::isinstance<py::str>(kwargs["name"]))
              throw py::value_error("Argument 'name' must be a string");
            if (!py::isinstance<py::str>(kwargs["units"]))
              throw py::value_error("Argument 'units' must be a string");
            if (!py::isinstance<py::int_>(kwargs["num_sections"]))
              throw py::value_error("Argument 'num_sections' must be an integer");
            if (kwargs["num_sections"].cast<std::size_t>() <= 0)
              throw py::value_error("Argument 'num_sections' must be greater than 0");
            std::string name = kwargs["name"].cast<std::string>();
            std::string units = kwargs["units"].cast<std::string>();
            std::size_t num_sections = kwargs["num_sections"].cast<std::size_t>();
            musica::Error error;
            auto grid_instance = new musica::Grid(name.c_str(), units.c_str(), num_sections, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error creating grid: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return grid_instance;
          }))
      .def("__del__", [](musica::Grid &grid) {})
      .def_property_readonly(
          "name",
          [](musica::Grid &self)
          {
            musica::Error error;
            std::string name = self.GetName(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting grid name: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            musica::DeleteError(&error);
            return name;
          },
          "The name of the grid")
      .def_property_readonly(
          "units",
          [](musica::Grid &self)
          {
            musica::Error error;
            std::string units = self.GetUnits(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting grid units: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            musica::DeleteError(&error);
            return units;
          },
          "The units of the grid")
      .def_property_readonly(
          "num_sections",
          [](musica::Grid &self)
          {
            musica::Error error;
            std::size_t num_sections = self.GetNumberOfSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return num_sections;
          },
          "The number of sections in the grid")
      .def_property(
          "edges",
          // Getter - converts C++ array to numpy array
          [](musica::Grid &self)
          {
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error) + 1;
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            auto result = py::array_t<double>(size);
            py::buffer_info buf = result.request();
            double *ptr = static_cast<double *>(buf.ptr);
            self.GetEdges(ptr, size, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting grid edges: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return result;
          },
          // Setter - converts numpy array to C++ array
          [](musica::Grid &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error) + 1;
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            if (buf.ndim != 1)
            {
              throw py::value_error("Number of dimensions must be one");
            }
            if (static_cast<size_t>(buf.size) != size)
            {
              throw py::value_error("Array size must be num_sections + 1");
            }
            double *ptr = static_cast<double *>(buf.ptr);
            self.SetEdges(ptr, size, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error setting grid edges: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
          },
          "Grid edges array of length num_sections + 1")
      .def_property(
          "midpoints",
          // Getter - converts C++ array to numpy array
          [](musica::Grid &self)
          {
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            auto result = py::array_t<double>(size);
            py::buffer_info buf = result.request();
            double *ptr = static_cast<double *>(buf.ptr);
            self.GetMidpoints(ptr, size, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting grid midpoints: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return result;
          },
          // Setter - converts numpy array to C++ array
          [](musica::Grid &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t size = self.GetNumberOfSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            if (buf.ndim != 1)
            {
              throw py::value_error("Number of dimensions must be one");
            }
            if (static_cast<size_t>(buf.size) != size)
            {
              throw py::value_error("Array size must be num_sections");
            }
            double *ptr = static_cast<double *>(buf.ptr);
            self.SetMidpoints(ptr, size, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error setting grid midpoints: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
          },
          "Grid midpoints array of length num_sections");
}