// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "binding_common.hpp"

#include <musica/tuvx/grid.hpp>
#include <musica/tuvx/grid_map.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_tuvx_grid_map(py::module& m)
{
  py::class_<musica::GridMap>(m, "_GridMap")
      .def(py::init<>(
          []()
          {
            musica::Error error;
            auto grid_map_instance = new musica::GridMap(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error creating GridMap: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return grid_map_instance;
          }))
      .def(
          "add_grid",
          [](musica::GridMap& self, musica::Grid* grid)
          {
            musica::Error error;
            self.AddGrid(grid, &error);
            if (error.code_ != 0)
            {
              std::string message = "Error adding grid: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw std::runtime_error(message);
            }
            DeleteError(&error);
          })
      .def(
          "get_grid",
          [](musica::GridMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            musica::Grid* grid = self.GetGrid(name.c_str(), units.c_str(), &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting grid: " + message);
            }
            musica::DeleteError(&error);
            return grid;
          },
          py::return_value_policy::reference)
      .def(
          "get_grid_by_index",
          [](musica::GridMap& self, std::size_t index)
          {
            musica::Error error;
            musica::Grid* grid = self.GetGridByIndex(index, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting grid by index: " + message);
            }
            musica::DeleteError(&error);
            return grid;
          },
          py::return_value_policy::reference)
      .def(
          "remove_grid",
          [](musica::GridMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            self.RemoveGrid(name.c_str(), units.c_str(), &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error removing grid: " + message);
            }
            musica::DeleteError(&error);
          })
      .def(
          "remove_grid_by_index",
          [](musica::GridMap& self, std::size_t index)
          {
            musica::Error error;
            self.RemoveGridByIndex(index, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error removing grid by index: " + message);
            }
            musica::DeleteError(&error);
          })
      .def(
          "get_number_of_grids",
          [](musica::GridMap& self)
          {
            musica::Error error;
            std::size_t num_grids = self.GetNumberOfGrids(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error("Error getting number of grids: " + message);
            }
            musica::DeleteError(&error);
            return num_grids;
          });
}
