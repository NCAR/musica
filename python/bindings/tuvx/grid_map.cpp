// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

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
            handle_error(error, "Error creating GridMap");
            return grid_map_instance;
          }))
      .def(
          "add_grid",
          [](musica::GridMap& self, musica::Grid* grid)
          {
            musica::Error error;
            self.AddGrid(grid, &error);
            handle_error(error, "Error adding grid");
          })
      .def(
          "get_grid",
          [](musica::GridMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            musica::Grid* grid = self.GetGrid(name.c_str(), units.c_str(), &error);
            handle_error(error, "Error getting grid");
            return grid;
          },
          py::return_value_policy::reference)
      .def(
          "get_grid_by_index",
          [](musica::GridMap& self, std::size_t index)
          {
            musica::Error error;
            musica::Grid* grid = self.GetGridByIndex(index, &error);
            handle_error(error, "Error getting grid by index");
            return grid;
      },  
          py::return_value_policy::reference)
      .def(
          "remove_grid",
          [](musica::GridMap& self, const std::string& name, const std::string& units)
          {
            musica::Error error;
            self.RemoveGrid(name.c_str(), units.c_str(), &error);
            handle_error(error, "Error removing grid");
          })
      .def(
          "remove_grid_by_index",
          [](musica::GridMap& self, std::size_t index)
          {
            musica::Error error;
            self.RemoveGridByIndex(index, &error);
            handle_error(error, "Error removing grid by index");
          })
      .def(
          "get_number_of_grids",
          [](musica::GridMap& self)
          {
            musica::Error error;
            std::size_t num_grids = self.GetNumberOfGrids(&error);
            handle_error(error, "Error getting number of grids");
            return num_grids;
          });
          });
}
